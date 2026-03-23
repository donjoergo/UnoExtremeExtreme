#ifndef UNO_EXTREME_V2_DOMAIN_MOTION_MOTION_CATALOG_H
#define UNO_EXTREME_V2_DOMAIN_MOTION_MOTION_CATALOG_H

#include <stdint.h>

#include "types/action_type.h"
#include "types/runtime_config.h"

namespace uno_extreme {
namespace domain_motion {

enum class MotionDirection : uint8_t {
  Forward = 0,
  Reverse = 1,
  Stop = 2
};

enum class MotionPatternKind : uint8_t {
  SplitEject = 0,
  VariableSpeed = 1,
  Stutter = 2,
  FakeFault = 3
};

constexpr uint8_t kMotionPatternMaxSegments = 4;
constexpr uint8_t kMotionPressHistorySize = 8;
constexpr uint8_t kMotionPatternCount = 4;

struct MotionSegment {
  MotionDirection direction;
  uint8_t speed;
  uint16_t duration_ms;
  uint16_t pause_after_ms;
};

struct MotionPattern {
  uint16_t pattern_id;
  MotionPatternKind kind;
  ActionMask valid_actions;
  uint8_t weight;
  uint8_t segment_count;
  MotionSegment segments[kMotionPatternMaxSegments];
};

struct MotionSelection {
  bool valid;
  MotionPattern pattern;
  uint8_t intensity_percent;
};

struct MotionPressHistory {
  uint16_t timestamps[kMotionPressHistorySize];
  uint8_t count;
};

struct MotionSelectorState {
  uint8_t remaining_safe_mask;
  uint8_t remaining_lose_mask;
};

void reset(MotionSelectorState& state);
void recordPress(MotionPressHistory& history, uint32_t timestamp_ms);
uint8_t computeIntensity(const RuntimeConfig& config, const MotionPressHistory& history, uint32_t now_ms);
MotionSelection choosePatternById(uint16_t pattern_id, uint8_t intensity_percent);
MotionSelection choosePattern(
    ActionType result_action,
    uint32_t roll,
    MotionSelectorState& state,
    uint8_t intensity_percent);

}  // namespace domain_motion
}  // namespace uno_extreme

#endif
