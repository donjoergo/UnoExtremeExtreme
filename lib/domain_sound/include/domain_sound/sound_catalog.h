#ifndef UNO_EXTREME_V2_DOMAIN_SOUND_SOUND_CATALOG_H
#define UNO_EXTREME_V2_DOMAIN_SOUND_SOUND_CATALOG_H

#include <stdint.h>

#include "types/action_type.h"
#include "types/runtime_config.h"
#include "types/sound_category.h"

namespace uno_extreme {
namespace domain_sound {

constexpr uint8_t kNoVolumeOverride = 0xFFu;
constexpr uint16_t kNoMotionPatternId = 0xFFFFu;
constexpr uint8_t kNoSequenceGroup = 0xFFu;

struct SoundItem {
  uint16_t sound_id;
  uint8_t folder_id;
  uint8_t file_index;
  SoundCategory category;
  ActionMask valid_actions;
  uint8_t weight;
  uint16_t wait_before_ms;
  uint16_t wait_after_ms;
  uint8_t volume_override;
  uint16_t motion_pattern_id;
  uint8_t predecessor_group;
  uint8_t follow_up_group;
};

struct SoundSelection {
  bool valid;
  SoundItem item;
};

constexpr bool hasVolumeOverride(const SoundItem& item) {
  return item.volume_override != kNoVolumeOverride;
}

constexpr bool hasMotionPatternBinding(const SoundItem& item) {
  return item.motion_pattern_id != kNoMotionPatternId;
}

void resetSoundCatalogState();
bool hasEnabledBaseGameplayCategory(const RuntimeConfig& config);

SoundSelection chooseStartupSound(const RuntimeConfig& config, uint32_t roll);
SoundSelection chooseCaseEventSound(const RuntimeConfig& config, ActionType action_type, uint32_t roll);
SoundSelection chooseSpamReactionSound(const RuntimeConfig& config, uint32_t roll);
SoundSelection chooseGameplaySound(const RuntimeConfig& config, ActionType result_action, uint32_t roll);

}  // namespace domain_sound
}  // namespace uno_extreme

#endif
