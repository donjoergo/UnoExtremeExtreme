#ifndef UNO_EXTREME_V2_PLATFORM_NANO_CONFIG_STORE_H
#define UNO_EXTREME_V2_PLATFORM_NANO_CONFIG_STORE_H

#include "types/runtime_config.h"

namespace uno_extreme {
namespace platform_nano {

bool loadRuntimeConfig(RuntimeConfig& config);
bool saveRuntimeConfig(const RuntimeConfig& config);
void clearPersistedRuntimeConfig();

}  // namespace platform_nano
}  // namespace uno_extreme

#endif
