#ifndef UNO_EXTREME_V2_CONFIG_DEFAULTS_H
#define UNO_EXTREME_V2_CONFIG_DEFAULTS_H

#include <stdint.h>

#include "types/action_type.h"
#include "types/runtime_config.h"

namespace uno_extreme {
namespace config {

constexpr uint16_t kRuntimeConfigSchemaVersion = 1;

constexpr SoundCategoryMask kDefaultEnabledCategories =
    toSoundCategoryMask(SoundCategory::Startup) |
    toSoundCategoryMask(SoundCategory::BaseFunny) |
    toSoundCategoryMask(SoundCategory::BaseTts) |
    toSoundCategoryMask(SoundCategory::CaseEvents) |
    toSoundCategoryMask(SoundCategory::SpamReactions);

constexpr RuntimeConfig defaultRuntimeConfig() {
  return RuntimeConfig{
    kRuntimeConfigSchemaVersion,
    GameMode::Extreme,
    kDefaultEnabledCategories,
    {
      0,
      100,
      100,
      100,
      0,
      0,
      0,
      0,
      0,
      100,
      100
    },
    20,
    40,
    60,
    1500,
    {
      {2, 85},
      {3, 70},
      {4, 55},
      {5, 40}
    },
    10000UL,
    15000UL,
    150,
    100,
    {
      {0, 20},
      {85, 145}
    }
  };
}

static_assert(kGameModeCount == 3u, "Unexpected GameMode count");
static_assert(kActionTypeCount == 10u, "Unexpected ActionType count");
static_assert(kSoundCategoryCount == 11u, "Unexpected SoundCategory count");

}  // namespace config
}  // namespace uno_extreme

#endif
