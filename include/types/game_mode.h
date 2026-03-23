#ifndef UNO_EXTREME_V2_TYPES_GAME_MODE_H
#define UNO_EXTREME_V2_TYPES_GAME_MODE_H

#include <stdint.h>

namespace uno_extreme {

enum class GameMode : uint8_t {
  Normal = 0,
  Ascending = 1,
  Extreme = 2
};

constexpr uint8_t kGameModeCount = 3;

}  // namespace uno_extreme

#endif
