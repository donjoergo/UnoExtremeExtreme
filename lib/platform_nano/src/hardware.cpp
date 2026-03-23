#include "platform_nano/hardware.h"

#include <Arduino.h>

#include "config/pins_nano.h"

namespace uno_extreme {
namespace platform_nano {

void applySafeIdle();
void setLedOff();
void applyFeedback(FeedbackState state);

namespace {

constexpr uint8_t kEventQueueSize = 8;

struct DebounceChannel {
  bool raw_state;
  bool debounced_state;
  uint32_t last_raw_change_ms;
  uint16_t debounce_window_ms;
};

struct EventQueue {
  InputEvent events[kEventQueueSize];
  uint8_t head;
  uint8_t tail;
  uint8_t count;
};

RuntimeConfig g_runtime_config;
bool g_has_runtime_config = false;
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

void pushEvent(InputEventType type, uint32_t timestamp_ms, bool case_closed) {
  if (g_event_queue.count >= kEventQueueSize) {
    return;
  }

  g_event_queue.events[g_event_queue.tail].type = type;
  g_event_queue.events[g_event_queue.tail].timestamp_ms = timestamp_ms;
  g_event_queue.events[g_event_queue.tail].case_closed = case_closed;
  g_event_queue.tail = static_cast<uint8_t>((g_event_queue.tail + 1u) % kEventQueueSize);
  g_event_queue.count++;
}

void configureDebounceChannel(DebounceChannel& channel, bool raw_state, uint16_t debounce_window_ms, uint32_t now_ms) {
  channel.raw_state = raw_state;
  channel.debounced_state = raw_state;
  channel.last_raw_change_ms = now_ms;
  channel.debounce_window_ms = debounce_window_ms;
}

void syncDebounceWindows(const RuntimeConfig& config) {
  g_button_channel.debounce_window_ms = config.button_debounce_ms;
  g_case_channel.debounce_window_ms = config.case_debounce_ms;
}

void updateButtonDebounce(bool raw_state, uint32_t now_ms, bool case_closed) {
  if (raw_state != g_button_channel.raw_state) {
    g_button_channel.raw_state = raw_state;
    g_button_channel.last_raw_change_ms = now_ms;
  }

  if (g_button_channel.debounced_state == g_button_channel.raw_state) {
    return;
  }

  if (static_cast<uint32_t>(now_ms - g_button_channel.last_raw_change_ms) < g_button_channel.debounce_window_ms) {
    return;
  }

  g_button_channel.debounced_state = g_button_channel.raw_state;
  if (g_button_channel.debounced_state) {
    pushEvent(InputEventType::ButtonPressed, now_ms, case_closed);
  }
}

void updateCaseDebounce(bool raw_state, uint32_t now_ms) {
  if (raw_state != g_case_channel.raw_state) {
    g_case_channel.raw_state = raw_state;
    g_case_channel.last_raw_change_ms = now_ms;
  }

  if (g_case_channel.debounced_state == g_case_channel.raw_state) {
    return;
  }

  if (static_cast<uint32_t>(now_ms - g_case_channel.last_raw_change_ms) < g_case_channel.debounce_window_ms) {
    return;
  }

  g_case_channel.debounced_state = g_case_channel.raw_state;
  pushEvent(g_case_channel.debounced_state ? InputEventType::CaseClosed : InputEventType::CaseOpened, now_ms, g_case_channel.debounced_state);
}

void initializeDebounceState() {
  const uint32_t now_ms = millis();
  syncDebounceWindows(g_runtime_config);
  configureDebounceChannel(g_button_channel, readButtonRaw(), g_runtime_config.button_debounce_ms, now_ms);
  configureDebounceChannel(g_case_channel, readCaseClosedRaw(), g_runtime_config.case_debounce_ms, now_ms);
  g_event_queue.head = 0;
  g_event_queue.tail = 0;
  g_event_queue.count = 0;
  g_last_tick_ms = now_ms;
}

}  // namespace

void initializeButton() {
  pinMode(config::kButtonPin, INPUT_PULLUP);
}

void initializeCaseSwitch() {
  pinMode(config::kCaseSwitchPin, INPUT_PULLUP);
}

void initializeLeds() {
  pinMode(config::kLedDataPin, OUTPUT);
  digitalWrite(config::kLedDataPin, LOW);
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
  pinMode(config::kDfPlayerTxPin, INPUT);
  pinMode(config::kBluetoothRxPin, INPUT);
  pinMode(config::kBluetoothTxPin, INPUT);
}

void initializeStorage() {
}

void initializeHardware(const RuntimeConfig& config) {
  g_runtime_config = config;
  g_has_runtime_config = true;

  initializeButton();
  initializeCaseSwitch();
  initializeLeds();
  initializeMotor();
  initializeAudio();
  initializeStorage();
  initializeDebounceState();
}

void applySafeIdle() {
  digitalWrite(config::kMotorIn1Pin, LOW);
  digitalWrite(config::kMotorIn2Pin, LOW);
  analogWrite(config::kMotorEnablePin, 0);
  setLedOff();
}

void setLedOff() {
  digitalWrite(config::kLedDataPin, LOW);
}

void setLedRgb(uint8_t red, uint8_t green, uint8_t blue) {
  (void)red;
  (void)green;
  (void)blue;
}

void applyFeedback(FeedbackState state) {
  switch (state) {
    case FeedbackState::Ready:
    case FeedbackState::Placeholder:
      setLedOff();
      break;
    case FeedbackState::Maintenance:
    case FeedbackState::Warning:
      setLedOff();
      break;
  }
}

bool isCaseClosed() {
  return g_case_channel.debounced_state;
}

bool isButtonPressed() {
  return g_button_channel.debounced_state;
}

InputEvent pollInputEvent(uint32_t now_ms, const RuntimeConfig& config) {
  g_runtime_config = config;
  g_has_runtime_config = true;
  syncDebounceWindows(config);

  const bool button_raw = readButtonRaw();
  const bool case_closed_raw = readCaseClosedRaw();
  updateButtonDebounce(button_raw, now_ms, g_case_channel.debounced_state);
  updateCaseDebounce(case_closed_raw, now_ms);

  InputEvent event = {InputEventType::None, now_ms, g_case_channel.debounced_state};
  if (g_event_queue.count == 0) {
    return event;
  }

  event = g_event_queue.events[g_event_queue.head];
  g_event_queue.head = static_cast<uint8_t>((g_event_queue.head + 1u) % kEventQueueSize);
  g_event_queue.count--;
  return event;
}

InputSnapshot sampleInputs() {
  InputSnapshot snapshot = {isButtonPressed(), isCaseClosed()};
  return snapshot;
}

void tick() {
  if (!g_has_runtime_config) {
    return;
  }

  const uint32_t now_ms = millis();
  if (now_ms == g_last_tick_ms) {
    return;
  }

  g_last_tick_ms = now_ms;

  (void)pollInputEvent(now_ms, g_runtime_config);
}

}  // namespace platform_nano
}  // namespace uno_extreme
