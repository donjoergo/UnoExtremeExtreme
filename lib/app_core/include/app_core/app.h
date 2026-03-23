#ifndef UNO_EXTREME_V2_APP_CORE_APP_H
#define UNO_EXTREME_V2_APP_CORE_APP_H

#include <stdint.h>

#include "types/action_type.h"
#include "types/runtime_config.h"
#include "types/runtime_state.h"

namespace uno_extreme {
namespace app_core {

struct RuntimeStatus {
  RuntimeState state;
  bool boot_completed;
  bool case_closed;
  uint32_t case_open_since_ms;
  uint32_t last_open_warning_ms;
  uint16_t accepted_button_presses;
  ActionType last_lifecycle_action;
};

void setup();
void loop();
const RuntimeConfig& runtimeConfig();
const RuntimeStatus& runtimeStatus();

}  // namespace app_core
}  // namespace uno_extreme

#endif
