#include "platform_nano/hardware.h"

#include <Arduino.h>
#include <avr/io.h>

#include "config/pins_nano.h"
#include "platform_nano/service_transport.h"

namespace uno_extreme {
namespace platform_nano {

void applySafeIdle();
void setLedOff();
void applyFeedback(FeedbackState state);

namespace {

constexpr uint8_t kEventQueueSize = 6;
constexpr uint8_t kLedCount = 2;
constexpr uint8_t kMotorBoostThreshold = 150;
constexpr uint8_t kMotorBoostSpeed = 170;
constexpr uint16_t kMotorBoostTimeMs = 50;
constexpr uint16_t kMotorCheckIntervalMs = 5;
constexpr uint16_t kDfPlayerBitDurationUs = 104;
constexpr uint8_t kDfPlayerCmdVolume = 0x06;
constexpr uint8_t kDfPlayerCmdPlayFolder = 0x0Fu;
constexpr uint8_t kLedPortMask = _BV(PD4);
constexpr uint8_t kLedPortLowMask = static_cast<uint8_t>(~kLedPortMask);

struct DebounceChannel {
  bool raw_state;
  bool debounced_state;
  uint16_t last_raw_change_tick;
  uint16_t debounce_window_ms;
};

struct QueuedEvent {
  InputEventType type;
  bool case_closed;
};

struct EventQueue {
  QueuedEvent events[kEventQueueSize];
  uint8_t head;
  uint8_t tail;
  uint8_t count;
};

bool g_audio_available = false;
DebounceChannel g_button_channel = {true, true, 0, 40};
DebounceChannel g_case_channel = {true, true, 0, 60};
EventQueue g_event_queue = {{}, 0, 0, 0};
uint32_t g_last_tick_ms = 0;

bool readButtonRaw() {
  return digitalRead(config::kButtonPin) == LOW;
}

bool readCaseClosedRaw() {
  return digitalRead(config::kCaseSwitchPin) == LOW;
}

void pushEvent(InputEventType type, bool case_closed);
void updateButtonDebounce(bool raw_state, uint32_t now_ms, bool case_closed);
void updateCaseDebounce(bool raw_state, uint32_t now_ms);
void writeDfPlayerByte(uint8_t value);
void sendDfPlayerCommand(uint8_t command, uint16_t parameter);
void writeLedByte(uint8_t value);
void showLedColor(uint8_t red, uint8_t green, uint8_t blue);
void setLedPinHigh();
void setLedPinLow();

void setCaseOpenImmediately(uint32_t now_ms) {
  g_case_channel.raw_state = false;
  g_case_channel.debounced_state = false;
  g_case_channel.last_raw_change_tick = static_cast<uint16_t>(now_ms);
  pushEvent(InputEventType::CaseOpened, false);
}

void pushEvent(InputEventType type, bool case_closed) {
  if (g_event_queue.count >= kEventQueueSize) {
    return;
  }

  g_event_queue.events[g_event_queue.tail].type = type;
  g_event_queue.events[g_event_queue.tail].case_closed = case_closed;
  g_event_queue.tail = static_cast<uint8_t>((g_event_queue.tail + 1u) % kEventQueueSize);
  g_event_queue.count++;
}

void configureDebounceChannel(DebounceChannel& channel, bool raw_state, uint16_t debounce_window_ms, uint32_t now_ms) {
  channel.raw_state = raw_state;
  channel.debounced_state = raw_state;
  channel.last_raw_change_tick = static_cast<uint16_t>(now_ms);
  channel.debounce_window_ms = debounce_window_ms;
}

void sampleAndQueueInputs(const uint32_t now_ms) {
  const bool button_raw = readButtonRaw();
  const bool case_closed_raw = readCaseClosedRaw();
  updateButtonDebounce(button_raw, now_ms, g_case_channel.debounced_state);
  updateCaseDebounce(case_closed_raw, now_ms);
}

void syncDebounceWindows(const RuntimeConfig& config) {
  g_button_channel.debounce_window_ms = config.button_debounce_ms;
  g_case_channel.debounce_window_ms = config.case_debounce_ms;
}

void updateButtonDebounce(bool raw_state, uint32_t now_ms, bool case_closed) {
  if (raw_state != g_button_channel.raw_state) {
    g_button_channel.raw_state = raw_state;
    g_button_channel.last_raw_change_tick = static_cast<uint16_t>(now_ms);
  }

  if (g_button_channel.debounced_state == g_button_channel.raw_state) {
    return;
  }

  if (static_cast<uint16_t>(
          static_cast<uint16_t>(now_ms) - g_button_channel.last_raw_change_tick) <
      g_button_channel.debounce_window_ms) {
    return;
  }

  g_button_channel.debounced_state = g_button_channel.raw_state;
  if (g_button_channel.debounced_state) {
    pushEvent(InputEventType::ButtonPressed, case_closed);
  }
}

void updateCaseDebounce(bool raw_state, uint32_t now_ms) {
  if (raw_state != g_case_channel.raw_state) {
    g_case_channel.raw_state = raw_state;
    g_case_channel.last_raw_change_tick = static_cast<uint16_t>(now_ms);
  }

  if (g_case_channel.debounced_state == g_case_channel.raw_state) {
    return;
  }

  if (static_cast<uint16_t>(
          static_cast<uint16_t>(now_ms) - g_case_channel.last_raw_change_tick) <
      g_case_channel.debounce_window_ms) {
    return;
  }

  g_case_channel.debounced_state = g_case_channel.raw_state;
  pushEvent(
      g_case_channel.debounced_state ? InputEventType::CaseClosed : InputEventType::CaseOpened,
      g_case_channel.debounced_state);
}

void initializeDebounceState() {
  const uint32_t now_ms = millis();
  configureDebounceChannel(g_button_channel, readButtonRaw(), g_button_channel.debounce_window_ms, now_ms);
  configureDebounceChannel(g_case_channel, readCaseClosedRaw(), g_case_channel.debounce_window_ms, now_ms);
  g_event_queue.head = 0;
  g_event_queue.tail = 0;
  g_event_queue.count = 0;
  g_last_tick_ms = now_ms;
}

void writeDfPlayerByte(const uint8_t value) {
  noInterrupts();

  digitalWrite(config::kDfPlayerTxPin, LOW);
  delayMicroseconds(kDfPlayerBitDurationUs);

  for (uint8_t bit_index = 0; bit_index < 8u; ++bit_index) {
    const bool bit_high = ((value >> bit_index) & 0x01u) != 0;
    digitalWrite(config::kDfPlayerTxPin, bit_high ? HIGH : LOW);
    delayMicroseconds(kDfPlayerBitDurationUs);
  }

  digitalWrite(config::kDfPlayerTxPin, HIGH);
  delayMicroseconds(kDfPlayerBitDurationUs);

  interrupts();
}

void sendDfPlayerCommand(const uint8_t command, const uint16_t parameter) {
  const uint8_t parameter_high = static_cast<uint8_t>(parameter >> 8);
  const uint8_t parameter_low = static_cast<uint8_t>(parameter & 0xFFu);
  const uint16_t checksum = static_cast<uint16_t>(
      0u - (0xFFu + 0x06u + command + 0x00u + parameter_high + parameter_low));

  writeDfPlayerByte(0x7Eu);
  writeDfPlayerByte(0xFFu);
  writeDfPlayerByte(0x06u);
  writeDfPlayerByte(command);
  writeDfPlayerByte(0x00u);
  writeDfPlayerByte(parameter_high);
  writeDfPlayerByte(parameter_low);
  writeDfPlayerByte(static_cast<uint8_t>(checksum >> 8));
  writeDfPlayerByte(static_cast<uint8_t>(checksum & 0xFFu));
  writeDfPlayerByte(0xEFu);
}

void writeLedByte(uint8_t value) {
  for (uint8_t mask = 0x80u; mask != 0u; mask >>= 1u) {
    setLedPinHigh();
    if ((value & mask) != 0u) {
      asm volatile(
          "nop\nnop\nnop\nnop\nnop\nnop\nnop\n");
      setLedPinLow();
      asm volatile("nop\nnop\n");
    } else {
      asm volatile("nop\nnop\n");
      setLedPinLow();
      asm volatile(
          "nop\nnop\nnop\nnop\nnop\nnop\n");
    }
  }
}

void showLedColor(const uint8_t red, const uint8_t green, const uint8_t blue) {
  noInterrupts();
  for (uint8_t index = 0; index < kLedCount; ++index) {
    writeLedByte(green);
    writeLedByte(red);
    writeLedByte(blue);
  }
  interrupts();
  delayMicroseconds(80);
}

void setLedPinHigh() {
  PORTD |= kLedPortMask;
}

void setLedPinLow() {
  PORTD = static_cast<uint8_t>(PORTD & kLedPortLowMask);
}

}  // namespace

bool isPlaybackActive();

void initializeButton() {
  pinMode(config::kButtonPin, INPUT_PULLUP);
}

void initializeCaseSwitch() {
  pinMode(config::kCaseSwitchPin, INPUT_PULLUP);
}

void initializeLeds() {
  pinMode(config::kLedDataPin, OUTPUT);
  digitalWrite(config::kLedDataPin, LOW);
  setLedOff();
}

void initializeMotor() {
  pinMode(config::kMotorEnablePin, OUTPUT);
  pinMode(config::kMotorIn1Pin, OUTPUT);
  pinMode(config::kMotorIn2Pin, OUTPUT);
  applySafeIdle();
}

void initializeAudio() {
  pinMode(config::kDfPlayerBusyPin, INPUT);
  pinMode(config::kDfPlayerRxPin, INPUT);
  pinMode(config::kDfPlayerTxPin, OUTPUT);
  digitalWrite(config::kDfPlayerTxPin, HIGH);
  g_audio_available = true;
}

void initializeStorage() {
}

void initializeHardware(const RuntimeConfig& config) {
  initializeButton();
  initializeCaseSwitch();
  initializeLeds();
  initializeMotor();
  syncDebounceWindows(config);
  initializeAudio();
  initializeStorage();
  initializeServiceTransport();
  initializeDebounceState();
  setMasterVolume(config.master_volume);
}

void applySafeIdle() {
  digitalWrite(config::kMotorIn1Pin, LOW);
  digitalWrite(config::kMotorIn2Pin, LOW);
  analogWrite(config::kMotorEnablePin, 0);
  setLedOff();
}

void setLedOff() {
  setLedRgb(0, 0, 0);
}

void setLedRgb(uint8_t red, uint8_t green, uint8_t blue) {
  showLedColor(red, green, blue);
}

void applyFeedback(FeedbackState state) {
  switch (state) {
    case FeedbackState::Ready:
      setLedRgb(0, 48, 0);
      break;
    case FeedbackState::Maintenance:
      setLedRgb(0, 0, 48);
      break;
    case FeedbackState::Warning:
      setLedRgb(64, 24, 0);
      break;
    case FeedbackState::Placeholder:
      setLedRgb(48, 0, 48);
      break;
    case FeedbackState::Active:
      setLedRgb(48, 48, 48);
      break;
  }
}

void setMasterVolume(uint8_t volume) {
  if (!g_audio_available) {
    return;
  }

  if (volume > 30u) {
    volume = 30u;
  }
  sendDfPlayerCommand(kDfPlayerCmdVolume, volume);
}

bool playFolderSound(uint8_t folder, uint8_t file_index, uint8_t volume_override) {
  if (!g_audio_available) {
    return false;
  }

  if (volume_override <= 30u) {
    setMasterVolume(volume_override);
  }
  const uint16_t folder_file = static_cast<uint16_t>(
      (static_cast<uint16_t>(folder) << 8) | static_cast<uint16_t>(file_index));
  sendDfPlayerCommand(kDfPlayerCmdPlayFolder, folder_file);
  return true;
}

bool waitForPlaybackStart(uint32_t timeout_ms) {
  if (!g_audio_available) {
    return false;
  }

  const uint32_t start_ms = millis();
  while ((millis() - start_ms) < timeout_ms) {
    if (isPlaybackActive()) {
      return true;
    }
    delay(10);
  }

  return false;
}

void waitForPlaybackFinish(uint32_t timeout_ms) {
  if (!g_audio_available) {
    return;
  }

  const bool playback_started = waitForPlaybackStart(250);
  if (!playback_started) {
    return;
  }

  const uint32_t start_ms = millis();
  while ((millis() - start_ms) < timeout_ms) {
    if (!isPlaybackActive()) {
      return;
    }
    delay(10);
  }
}

bool isPlaybackActive() {
  if (!g_audio_available) {
    return false;
  }

  return digitalRead(config::kDfPlayerBusyPin) == LOW;
}

bool waitMilliseconds(const uint16_t duration_ms) {
  const uint32_t start_ms = millis();
  while ((millis() - start_ms) < duration_ms) {
    tick();
    if (!readCaseClosedRaw()) {
      setCaseOpenImmediately(millis());
      applySafeIdle();
      return false;
    }
    delay(kMotorCheckIntervalMs);
  }

  return true;
}

bool runMotorSegment(bool forward, uint8_t speed, uint16_t duration_ms) {
  if (!readCaseClosedRaw()) {
    setCaseOpenImmediately(millis());
    applySafeIdle();
    return false;
  }

  if (forward && speed < kMotorBoostThreshold) {
    digitalWrite(config::kMotorIn1Pin, LOW);
    digitalWrite(config::kMotorIn2Pin, HIGH);
    analogWrite(config::kMotorEnablePin, kMotorBoostSpeed);
    delay(kMotorBoostTimeMs);
  }

  digitalWrite(config::kMotorIn1Pin, forward ? LOW : HIGH);
  digitalWrite(config::kMotorIn2Pin, forward ? HIGH : LOW);
  analogWrite(config::kMotorEnablePin, speed);

  const uint32_t start_ms = millis();
  while ((millis() - start_ms) < duration_ms) {
    tick();
    if (!readCaseClosedRaw()) {
      setCaseOpenImmediately(millis());
      applySafeIdle();
      return false;
    }
    delay(kMotorCheckIntervalMs);
  }

  applySafeIdle();
  return true;
}

bool runMotorForward(uint8_t speed, uint16_t duration_ms) {
  return runMotorSegment(true, speed, duration_ms);
}

bool runMotorReverse(uint8_t speed, uint16_t duration_ms) {
  return runMotorSegment(false, speed, duration_ms);
}

bool isCaseClosed() {
  return g_case_channel.debounced_state;
}

bool isButtonPressed() {
  return g_button_channel.debounced_state;
}

InputEvent pollInputEvent(uint32_t now_ms, const RuntimeConfig& config) {
  syncDebounceWindows(config);
  sampleAndQueueInputs(now_ms);

  InputEvent event = {InputEventType::None, now_ms, g_case_channel.debounced_state};
  if (g_event_queue.count == 0) {
    return event;
  }

  event.type = g_event_queue.events[g_event_queue.head].type;
  event.timestamp_ms = now_ms;
  event.case_closed = g_event_queue.events[g_event_queue.head].case_closed;
  g_event_queue.head = static_cast<uint8_t>((g_event_queue.head + 1u) % kEventQueueSize);
  g_event_queue.count--;
  return event;
}

InputSnapshot sampleInputs() {
  InputSnapshot snapshot = {isButtonPressed(), isCaseClosed()};
  return snapshot;
}

void tick() {
  const uint32_t now_ms = millis();
  if (now_ms == g_last_tick_ms) {
    return;
  }

  g_last_tick_ms = now_ms;
  sampleAndQueueInputs(now_ms);
}

}  // namespace platform_nano
}  // namespace uno_extreme
