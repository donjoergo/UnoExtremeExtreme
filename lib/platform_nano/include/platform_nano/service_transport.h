#ifndef UNO_EXTREME_V2_PLATFORM_NANO_SERVICE_TRANSPORT_H
#define UNO_EXTREME_V2_PLATFORM_NANO_SERVICE_TRANSPORT_H

#include <stdint.h>

namespace uno_extreme {
namespace platform_nano {

constexpr uint8_t kServiceCommandMaxLength = 42;

enum class ServiceChannel : uint8_t {
  None = 0,
  UsbSerial = 1,
  Bluetooth = 2
};

struct ServiceMessage {
  ServiceChannel channel;
  char command[kServiceCommandMaxLength];
};

void initializeServiceTransport();
bool pollServiceMessage(ServiceMessage& message);
void sendServiceReply(ServiceChannel channel, const char* line);

}  // namespace platform_nano
}  // namespace uno_extreme

#endif
