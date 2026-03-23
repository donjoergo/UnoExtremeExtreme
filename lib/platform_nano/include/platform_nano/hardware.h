#ifndef UNO_EXTREME_V2_PLATFORM_NANO_HARDWARE_H
#define UNO_EXTREME_V2_PLATFORM_NANO_HARDWARE_H

#include <stdint.h>

#include "types/runtime_config.h"

namespace uno_extreme {
namespace platform_nano {

enum class InputEventType : uint8_t {
  None = 0,
  ButtonPressed = 1,
  CaseOpened = 2,
  CaseClosed = 3
};

enum class FeedbackState : uint8_t {
  Ready = 0,
  Maintenance = 1,
  Warning = 2,
  Placeholder = 3,
  Active = 4
};

struct InputSnapshot {
  bool button_pressed;
  bool case_closed;
};

struct InputEvent {
  InputEventType type;
  uint32_t timestamp_ms;
  bool case_closed;
};

void initializeButton();
void initializeCaseSwitch();
void initializeLeds();
void initializeMotor();
void initializeAudio();
void initializeStorage();
void initializeHardware(const RuntimeConfig& config);
void applySafeIdle();
void setLedOff();
void setLedRgb(uint8_t red, uint8_t green, uint8_t blue);
void applyFeedback(FeedbackState state);
void setMasterVolume(uint8_t volume);
bool playFolderSound(uint8_t folder, uint8_t file_index);
bool waitForPlaybackStart(uint32_t timeout_ms);
void waitForPlaybackFinish(uint32_t timeout_ms);
bool isPlaybackActive();
bool waitMilliseconds(uint16_t duration_ms);
bool runMotorForward(uint8_t speed, uint16_t duration_ms);
bool runMotorReverse(uint8_t speed, uint16_t duration_ms);
bool isCaseClosed();
bool isButtonPressed();
InputEvent pollInputEvent(uint32_t now_ms, const RuntimeConfig& config);
void tick();
InputSnapshot sampleInputs();

}  // namespace platform_nano
}  // namespace uno_extreme

#endif
