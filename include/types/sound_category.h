#ifndef UNO_EXTREME_V2_TYPES_SOUND_CATEGORY_H
#define UNO_EXTREME_V2_TYPES_SOUND_CATEGORY_H

#include <stdint.h>

namespace uno_extreme {

using SoundCategoryMask = uint16_t;

enum class SoundCategory : uint8_t {
  BaseNormal = 0,
  BaseFunny = 1,
  BaseTts = 2,
  Startup = 3,
  CantinaShort = 4,
  CantinaLong = 5,
  Fetish = 6,
  Drinking = 7,
  Bdsm = 8,
  CaseEvents = 9,
  SpamReactions = 10
};

constexpr uint8_t kSoundCategoryCount = 11;

constexpr SoundCategoryMask toSoundCategoryMask(SoundCategory category) {
  return static_cast<SoundCategoryMask>(1u << static_cast<uint8_t>(category));
}

constexpr bool hasSoundCategory(SoundCategoryMask mask, SoundCategory category) {
  return (mask & toSoundCategoryMask(category)) != 0;
}

}  // namespace uno_extreme

#endif
