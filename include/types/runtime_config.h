#ifndef UNO_EXTREME_V2_TYPES_RUNTIME_CONFIG_H
#define UNO_EXTREME_V2_TYPES_RUNTIME_CONFIG_H

#include <stdint.h>

#include "types/game_mode.h"
#include "types/sound_category.h"

namespace uno_extreme {

constexpr uint8_t kCategoryWeightCount = kSoundCategoryCount;
constexpr uint8_t kRapidPressPenaltyStepCount = 4;
constexpr uint8_t kForbiddenHueRangeCount = 2;

struct RapidPressPenaltyStep {
  uint8_t press_count;
  uint8_t intensity_percent;
};

struct HueRange {
  uint8_t start;
  uint8_t end;
};

struct RuntimeConfig {
  uint16_t schema_version;
  GameMode game_mode;
  SoundCategoryMask enabled_categories;
  uint8_t category_weights[kCategoryWeightCount];
  uint8_t master_volume;
  uint16_t button_debounce_ms;
  uint16_t case_debounce_ms;
  uint16_t rapid_press_window_ms;
  RapidPressPenaltyStep rapid_press_penalty_steps[kRapidPressPenaltyStepCount];
  uint32_t open_warning_delay_ms;
  uint32_t open_warning_repeat_ms;
  uint8_t retraction_speed;
  uint16_t retraction_duration_ms;
  HueRange forbidden_hue_ranges[kForbiddenHueRangeCount];
};

}  // namespace uno_extreme

#endif
