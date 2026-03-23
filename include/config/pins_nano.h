#ifndef UNO_EXTREME_V2_CONFIG_PINS_NANO_H
#define UNO_EXTREME_V2_CONFIG_PINS_NANO_H

#include <stdint.h>

namespace uno_extreme {
namespace config {

constexpr uint8_t kButtonPin = 2;
constexpr uint8_t kCaseSwitchPin = 3;
constexpr uint8_t kLedDataPin = 4;
constexpr uint8_t kDfPlayerRxPin = 5;
constexpr uint8_t kDfPlayerTxPin = 6;
constexpr uint8_t kDfPlayerBusyPin = 7;
constexpr uint8_t kBluetoothTxPin = 8;
constexpr uint8_t kMotorIn2Pin = 9;
constexpr uint8_t kBluetoothRxPin = 10;
constexpr uint8_t kMotorEnablePin = 11;
constexpr uint8_t kMotorIn1Pin = 12;

}  // namespace config
}  // namespace uno_extreme

#endif
