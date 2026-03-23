#ifndef UNO_EXTREME_V2_TYPES_ACTION_TYPE_H
#define UNO_EXTREME_V2_TYPES_ACTION_TYPE_H

#include <stdint.h>

namespace uno_extreme {

using ActionMask = uint16_t;

enum class ActionType : uint8_t {
  Startup = 0,
  Safe = 1,
  Lose = 2,
  Wait = 3,
  SpamReaction = 4,
  CaseOpened = 5,
  CaseClosed = 6,
  CaseOpenTooLong = 7,
  CreativeScript = 8,
  MiniGame = 9
};

constexpr uint8_t kActionTypeCount = 10;

constexpr ActionMask toActionMask(ActionType action_type) {
  return static_cast<ActionMask>(1u << static_cast<uint8_t>(action_type));
}

}  // namespace uno_extreme

#endif
