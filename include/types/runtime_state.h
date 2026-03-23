#ifndef UNO_EXTREME_V2_TYPES_RUNTIME_STATE_H
#define UNO_EXTREME_V2_TYPES_RUNTIME_STATE_H

#include <stdint.h>

namespace uno_extreme {

enum class RuntimeState : uint8_t {
  IdleReady = 0,
  MaintenanceOpen = 1,
  ActionRunning = 2,
  WarningOpenTooLong = 3,
  PlaceholderMode = 4
};

}  // namespace uno_extreme

#endif
