#include "platform_nano/config_store.h"

#include <avr/eeprom.h>
#include <stdint.h>

namespace uno_extreme {
namespace platform_nano {
namespace {

constexpr uint16_t kConfigMagic = 0x5845;
constexpr uint16_t kNanoEepromCapacityBytes = 1024;

struct PersistedConfigRecord {
  uint16_t magic;
  uint16_t checksum;
  RuntimeConfig config;
};

PersistedConfigRecord EEMEM g_persisted_config;

uint16_t computeChecksum(const RuntimeConfig& config) {
  const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&config);
  uint16_t checksum = 0;
  for (uint16_t index = 0; index < sizeof(RuntimeConfig); ++index) {
    checksum = static_cast<uint16_t>((checksum << 1) ^ bytes[index]);
  }
  return checksum;
}

static_assert(sizeof(PersistedConfigRecord) <= kNanoEepromCapacityBytes,
    "Persisted runtime config exceeds EEPROM capacity");

}  // namespace

bool loadRuntimeConfig(RuntimeConfig& config) {
  PersistedConfigRecord record;
  eeprom_read_block(&record, &g_persisted_config, sizeof(record));
  if (record.magic != kConfigMagic) {
    return false;
  }

  if (record.checksum != computeChecksum(record.config)) {
    return false;
  }

  config = record.config;
  return true;
}

bool saveRuntimeConfig(const RuntimeConfig& config) {
  PersistedConfigRecord record = {
    kConfigMagic,
    computeChecksum(config),
    config
  };
  eeprom_update_block(&record, &g_persisted_config, sizeof(record));

  PersistedConfigRecord verify_record;
  eeprom_read_block(&verify_record, &g_persisted_config, sizeof(verify_record));
  return verify_record.magic == record.magic &&
      verify_record.checksum == record.checksum;
}

void clearPersistedRuntimeConfig() {
  const uint16_t cleared_magic = 0;
  eeprom_update_word(&g_persisted_config.magic, cleared_magic);
}

}  // namespace platform_nano
}  // namespace uno_extreme
