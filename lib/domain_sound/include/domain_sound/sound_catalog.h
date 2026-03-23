#ifndef UNO_EXTREME_V2_DOMAIN_SOUND_SOUND_CATALOG_H
#define UNO_EXTREME_V2_DOMAIN_SOUND_SOUND_CATALOG_H

#include <stdint.h>

#include "types/action_type.h"
#include "types/runtime_config.h"
#include "types/sound_category.h"

namespace uno_extreme {
namespace domain_sound {

struct SoundItem {
  uint16_t sound_id;
  uint8_t folder_id;
  uint8_t file_index;
  SoundCategory category;
  ActionMask valid_actions;
  uint8_t weight;
};

struct SoundSelection {
  bool valid;
  SoundItem item;
};

bool hasEnabledBaseGameplayCategory(const RuntimeConfig& config);

SoundSelection chooseStartupSound(const RuntimeConfig& config, uint32_t roll);
SoundSelection chooseCaseEventSound(const RuntimeConfig& config, ActionType action_type, uint32_t roll);
SoundSelection chooseSpamReactionSound(const RuntimeConfig& config, uint32_t roll);
SoundSelection chooseGameplaySound(const RuntimeConfig& config, ActionType result_action, uint32_t roll);

}  // namespace domain_sound
}  // namespace uno_extreme

#endif
