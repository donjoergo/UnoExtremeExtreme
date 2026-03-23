#include "platform_nano/service_transport.h"

#include <Arduino.h>
#include <SoftwareSerial.h>

#include "config/pins_nano.h"

namespace uno_extreme {
namespace platform_nano {
namespace {

struct ServiceBuffer {
  char data[kServiceCommandMaxLength];
  uint8_t length;
  ServiceChannel channel;
};

SoftwareSerial g_bluetooth_serial(config::kBluetoothRxPin, config::kBluetoothTxPin);
ServiceBuffer g_service_buffer = {{0}, 0, ServiceChannel::None};

bool readLineFromStream(
    Stream& stream,
    const ServiceChannel channel,
    ServiceMessage& message) {
  if (g_service_buffer.channel != channel) {
    g_service_buffer.length = 0;
    g_service_buffer.channel = channel;
  }

  while (stream.available() > 0) {
    const char character = static_cast<char>(stream.read());
    if (character == '\r') {
      continue;
    }

    if (character == '\n') {
      if (g_service_buffer.length == 0) {
        continue;
      }

      g_service_buffer.data[g_service_buffer.length] = '\0';
      message.channel = channel;
      for (uint8_t index = 0; index <= g_service_buffer.length; ++index) {
        message.command[index] = g_service_buffer.data[index];
      }
      g_service_buffer.length = 0;
      return true;
    }

    if (g_service_buffer.length >= (kServiceCommandMaxLength - 1u)) {
      g_service_buffer.length = 0;
      continue;
    }

    g_service_buffer.data[g_service_buffer.length] = character;
    g_service_buffer.length++;
  }

  return false;
}

}  // namespace

void initializeServiceTransport() {
  Serial.begin(9600);
  g_bluetooth_serial.begin(9600);
  g_bluetooth_serial.listen();
}

bool pollServiceMessage(ServiceMessage& message) {
  message.channel = ServiceChannel::None;
  message.command[0] = '\0';

  g_bluetooth_serial.listen();
  if (readLineFromStream(g_bluetooth_serial, ServiceChannel::Bluetooth, message)) {
    return true;
  }

  if (readLineFromStream(Serial, ServiceChannel::UsbSerial, message)) {
    return true;
  }

  return false;
}

void sendServiceReply(const ServiceChannel channel, const char* line) {
  switch (channel) {
    case ServiceChannel::Bluetooth:
      g_bluetooth_serial.println(line);
      break;
    case ServiceChannel::UsbSerial:
      Serial.println(line);
      break;
    case ServiceChannel::None:
      Serial.println(line);
      break;
  }
}

}  // namespace platform_nano
}  // namespace uno_extreme
