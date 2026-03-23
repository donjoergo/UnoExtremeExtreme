#include "domain_sound/sound_catalog.h"

#include <avr/pgmspace.h>
#include <stddef.h>

namespace uno_extreme {
namespace domain_sound {
namespace {

enum class PoolStateId : uint8_t {
  Startup = 0,
  CaseEvents = 1,
  SpamReactions = 2,
  GameplayBaseFunnySafe = 3,
  GameplayBaseFunnyLose = 4,
  GameplayBaseTtsSafe = 5,
  GameplayBaseTtsLose = 6,
  Count = 7
};

constexpr uint8_t kPoolStateCount = static_cast<uint8_t>(PoolStateId::Count);
constexpr uint8_t kStartupPoolBitsetBytes = 2;
constexpr uint8_t kCaseEventPoolBitsetBytes = 5;
constexpr uint8_t kSpamReactionPoolBitsetBytes = 1;
constexpr uint8_t kGameplayBaseFunnySafePoolBitsetBytes = 16;
constexpr uint8_t kGameplayBaseFunnyLosePoolBitsetBytes = 11;
constexpr uint8_t kGameplayBaseTtsSafePoolBitsetBytes = 3;
constexpr uint8_t kGameplayBaseTtsLosePoolBitsetBytes = 2;

struct SoundPool {
  PoolStateId state_id;
  uint16_t sound_id;
  uint8_t folder_id;
  uint8_t file_count;
  SoundCategory category;
  ActionMask valid_actions;
  uint8_t weight;
  uint16_t wait_before_ms;
  uint16_t wait_after_ms;
  uint8_t volume_override;
  uint16_t motion_pattern_id;
  uint8_t predecessor_group;
  uint8_t follow_up_group;
};

struct PoolMeta {
  uint8_t used_count;
  uint8_t last_file_index;
};

static_assert(kPoolStateCount == 7u, "Unexpected sound pool state count");

constexpr SoundCategoryMask kGameplayCategoryMask =
    toSoundCategoryMask(SoundCategory::BaseNormal) |
    toSoundCategoryMask(SoundCategory::BaseFunny) |
    toSoundCategoryMask(SoundCategory::BaseTts);

constexpr SoundCategoryMask kFallbackGameplayCategoryMask =
    toSoundCategoryMask(SoundCategory::BaseFunny) |
    toSoundCategoryMask(SoundCategory::BaseTts);

constexpr ActionMask kStartupActions = toActionMask(ActionType::Startup);
constexpr ActionMask kCaseEventActions =
    toActionMask(ActionType::CaseOpened) |
    toActionMask(ActionType::CaseClosed) |
    toActionMask(ActionType::CaseOpenTooLong);
constexpr ActionMask kSpamReactionActions = toActionMask(ActionType::SpamReaction);
constexpr ActionMask kSafeActions = toActionMask(ActionType::Safe);
constexpr ActionMask kLoseActions = toActionMask(ActionType::Lose);

// Transitional mapping: these folders point at the legacy Nano asset pools
// until the V2 SD card layout is physically provisioned.
const SoundPool kStartupPools[] PROGMEM = {
  {PoolStateId::Startup, 1001, 1, 14, SoundCategory::Startup, kStartupActions, 100,
      0, 120, kNoVolumeOverride, kNoMotionPatternId, kNoSequenceGroup, kNoSequenceGroup},
};

const SoundPool kCaseEventPools[] PROGMEM = {
  {PoolStateId::CaseEvents, 2001, 2, 36, SoundCategory::CaseEvents, kCaseEventActions, 100,
      0, 80, 18, kNoMotionPatternId, kNoSequenceGroup, kNoSequenceGroup},
};

const SoundPool kSpamReactionPools[] PROGMEM = {
  {PoolStateId::SpamReactions, 3001, 6, 5, SoundCategory::SpamReactions, kSpamReactionActions, 100,
      0, 40, 24, kNoMotionPatternId, kNoSequenceGroup, kNoSequenceGroup},
};

const SoundPool kGameplayPools[] PROGMEM = {
  {PoolStateId::GameplayBaseFunnySafe, 4001, 3, 128, SoundCategory::BaseFunny, kSafeActions, 100,
      140, 20, kNoVolumeOverride, kNoMotionPatternId, kNoSequenceGroup, kNoSequenceGroup},
  {PoolStateId::GameplayBaseFunnyLose, 4002, 4, 85, SoundCategory::BaseFunny, kLoseActions, 100,
      80, 60, 22, kNoMotionPatternId, kNoSequenceGroup, kNoSequenceGroup},
  {PoolStateId::GameplayBaseTtsSafe, 5001, 8, 24, SoundCategory::BaseTts, kSafeActions, 100,
      220, 0, 18, 1002, kNoSequenceGroup, kNoSequenceGroup},
  {PoolStateId::GameplayBaseTtsLose, 5002, 9, 13, SoundCategory::BaseTts, kLoseActions, 100,
      100, 120, 24, 1003, kNoSequenceGroup, kNoSequenceGroup},
};

static_assert(kSoundCategoryCount == 11u, "Unexpected SoundCategory count");
static_assert(kCategoryWeightCount == kSoundCategoryCount, "Category weights must track categories");

PoolMeta g_pool_meta[kPoolStateCount];
uint8_t g_startup_pool_bits[kStartupPoolBitsetBytes];
uint8_t g_case_event_pool_bits[kCaseEventPoolBitsetBytes];
uint8_t g_spam_reaction_pool_bits[kSpamReactionPoolBitsetBytes];
uint8_t g_gameplay_base_funny_safe_pool_bits[kGameplayBaseFunnySafePoolBitsetBytes];
uint8_t g_gameplay_base_funny_lose_pool_bits[kGameplayBaseFunnyLosePoolBitsetBytes];
uint8_t g_gameplay_base_tts_safe_pool_bits[kGameplayBaseTtsSafePoolBitsetBytes];
uint8_t g_gameplay_base_tts_lose_pool_bits[kGameplayBaseTtsLosePoolBitsetBytes];

bool isGameplayCategory(const SoundCategory category) {
  return category == SoundCategory::BaseNormal ||
      category == SoundCategory::BaseFunny ||
      category == SoundCategory::BaseTts;
}

bool hasBaseCategoryInMask(const SoundCategoryMask mask) {
  return (mask & kGameplayCategoryMask) != 0;
}

PoolMeta& poolMeta(const PoolStateId state_id) {
  return g_pool_meta[static_cast<uint8_t>(state_id)];
}

uint8_t* poolBits(const PoolStateId state_id) {
  if (state_id == PoolStateId::Startup) {
    return g_startup_pool_bits;
  }
  if (state_id == PoolStateId::CaseEvents) {
    return g_case_event_pool_bits;
  }
  if (state_id == PoolStateId::SpamReactions) {
    return g_spam_reaction_pool_bits;
  }
  if (state_id == PoolStateId::GameplayBaseFunnySafe) {
    return g_gameplay_base_funny_safe_pool_bits;
  }
  if (state_id == PoolStateId::GameplayBaseFunnyLose) {
    return g_gameplay_base_funny_lose_pool_bits;
  }
  if (state_id == PoolStateId::GameplayBaseTtsSafe) {
    return g_gameplay_base_tts_safe_pool_bits;
  }
  if (state_id == PoolStateId::GameplayBaseTtsLose) {
    return g_gameplay_base_tts_lose_pool_bits;
  }
  return nullptr;
}

uint8_t poolBitsetBytes(const PoolStateId state_id) {
  if (state_id == PoolStateId::Startup) {
    return kStartupPoolBitsetBytes;
  }
  if (state_id == PoolStateId::CaseEvents) {
    return kCaseEventPoolBitsetBytes;
  }
  if (state_id == PoolStateId::SpamReactions) {
    return kSpamReactionPoolBitsetBytes;
  }
  if (state_id == PoolStateId::GameplayBaseFunnySafe) {
    return kGameplayBaseFunnySafePoolBitsetBytes;
  }
  if (state_id == PoolStateId::GameplayBaseFunnyLose) {
    return kGameplayBaseFunnyLosePoolBitsetBytes;
  }
  if (state_id == PoolStateId::GameplayBaseTtsSafe) {
    return kGameplayBaseTtsSafePoolBitsetBytes;
  }
  if (state_id == PoolStateId::GameplayBaseTtsLose) {
    return kGameplayBaseTtsLosePoolBitsetBytes;
  }
  return 0;
}

SoundPool readPool(const SoundPool* pools, const size_t index) {
  SoundPool pool;
  memcpy_P(&pool, &pools[index], sizeof(pool));
  return pool;
}

void clearPoolState(const PoolStateId state_id) {
  PoolMeta& meta = poolMeta(state_id);
  uint8_t* const bits = poolBits(state_id);
  const uint8_t bitset_bytes = poolBitsetBytes(state_id);
  for (uint8_t index = 0; index < bitset_bytes; ++index) {
    bits[index] = 0;
  }
  meta.used_count = 0;
}

bool isFileUsed(const PoolStateId state_id, const uint8_t file_index, const uint8_t file_count) {
  if (file_index == 0 || file_index > file_count) {
    return false;
  }

  const uint8_t* const bits = poolBits(state_id);
  const uint8_t bit_index = static_cast<uint8_t>(file_index - 1u);
  const uint8_t byte_index = static_cast<uint8_t>(bit_index / 8u);
  const uint8_t bit_mask = static_cast<uint8_t>(1u << (bit_index % 8u));
  return (bits[byte_index] & bit_mask) != 0;
}

void markFileUsed(const PoolStateId state_id, const uint8_t file_index, const uint8_t file_count) {
  if (file_index == 0 || file_index > file_count) {
    return;
  }

  PoolMeta& meta = poolMeta(state_id);
  uint8_t* const bits = poolBits(state_id);
  const uint8_t bit_index = static_cast<uint8_t>(file_index - 1u);
  const uint8_t byte_index = static_cast<uint8_t>(bit_index / 8u);
  const uint8_t bit_mask = static_cast<uint8_t>(1u << (bit_index % 8u));
  if ((bits[byte_index] & bit_mask) == 0) {
    bits[byte_index] |= bit_mask;
    if (meta.used_count < 255u) {
      meta.used_count++;
    }
  }
}

bool isFreshPool(const PoolStateId state_id) {
  return poolMeta(state_id).used_count == 0;
}

bool isEnabledCategory(const RuntimeConfig& config, const SoundCategory category, const bool allow_fallback) {
  if (hasSoundCategory(config.enabled_categories, category)) {
    return true;
  }

  if (!allow_fallback) {
    return false;
  }

  return isGameplayCategory(category);
}

uint8_t categoryWeight(const RuntimeConfig& config, const SoundCategory category, const bool allow_fallback) {
  const uint8_t index = static_cast<uint8_t>(category);
  if (index >= kCategoryWeightCount) {
    return 0;
  }

  const uint8_t configured_weight = config.category_weights[index];
  if (configured_weight > 0) {
    return configured_weight;
  }

  if (!allow_fallback) {
    return 0;
  }

  if (category == SoundCategory::BaseFunny || category == SoundCategory::BaseTts || category == SoundCategory::BaseNormal) {
    return 100;
  }

  return 0;
}

uint8_t resolveFileIndex(const uint8_t file_count, const uint32_t roll) {
  if (file_count == 0) {
    return 0;
  }

  return static_cast<uint8_t>((roll % file_count) + 1u);
}

uint8_t chooseFileIndex(const SoundPool& pool, const uint32_t roll) {
  if (pool.file_count == 0) {
    return 0;
  }

  PoolMeta& meta = poolMeta(pool.state_id);
  if (meta.used_count >= pool.file_count) {
    clearPoolState(pool.state_id);
  }

  uint8_t eligible_count = 0;
  const bool avoid_last =
      isFreshPool(pool.state_id) && pool.file_count > 1u && meta.last_file_index > 0u;
  for (uint8_t file_index = 1; file_index <= pool.file_count; ++file_index) {
    if (isFileUsed(pool.state_id, file_index, pool.file_count)) {
      continue;
    }

    if (avoid_last && file_index == meta.last_file_index) {
      continue;
    }

    eligible_count++;
  }

  if (eligible_count == 0) {
    clearPoolState(pool.state_id);
    for (uint8_t file_index = 1; file_index <= pool.file_count; ++file_index) {
      if (avoid_last && pool.file_count > 1u && file_index == meta.last_file_index) {
        continue;
      }
      eligible_count++;
    }
  }

  if (eligible_count == 0) {
    return resolveFileIndex(pool.file_count, roll);
  }

  const uint8_t pick = static_cast<uint8_t>(roll % eligible_count);
  uint8_t seen = 0;
  for (uint8_t file_index = 1; file_index <= pool.file_count; ++file_index) {
    if (isFileUsed(pool.state_id, file_index, pool.file_count)) {
      continue;
    }

    if (avoid_last && file_index == meta.last_file_index) {
      continue;
    }

    if (seen == pick) {
      markFileUsed(pool.state_id, file_index, pool.file_count);
      meta.last_file_index = file_index;
      return file_index;
    }
    seen++;
  }

  for (uint8_t file_index = 1; file_index <= pool.file_count; ++file_index) {
    if (!isFileUsed(pool.state_id, file_index, pool.file_count)) {
      markFileUsed(pool.state_id, file_index, pool.file_count);
      meta.last_file_index = file_index;
      return file_index;
    }
  }

  return resolveFileIndex(pool.file_count, roll);
}

SoundSelection makeSelection(const SoundPool& pool, const uint32_t roll) {
  SoundSelection selection = {
    true,
    {
      0,
      0,
      0,
      SoundCategory::BaseNormal,
      0,
      0,
      0,
      0,
      kNoVolumeOverride,
      kNoMotionPatternId,
      kNoSequenceGroup,
      kNoSequenceGroup
    }
  };
  selection.item.sound_id = pool.sound_id;
  selection.item.folder_id = pool.folder_id;
  selection.item.file_index = chooseFileIndex(pool, roll);
  selection.item.category = pool.category;
  selection.item.valid_actions = pool.valid_actions;
  selection.item.weight = pool.weight;
  selection.item.wait_before_ms = pool.wait_before_ms;
  selection.item.wait_after_ms = pool.wait_after_ms;
  selection.item.volume_override = pool.volume_override;
  selection.item.motion_pattern_id = pool.motion_pattern_id;
  selection.item.predecessor_group = pool.predecessor_group;
  selection.item.follow_up_group = pool.follow_up_group;
  return selection;
}

bool matchesAction(const SoundPool& pool, const ActionType action_type) {
  return (pool.valid_actions & toActionMask(action_type)) != 0;
}

bool matchesCategoryMask(const SoundPool& pool, const SoundCategoryMask category_mask) {
  return (category_mask & toSoundCategoryMask(pool.category)) != 0;
}

template <size_t PoolCount>
SoundSelection chooseFromPools(
    const SoundPool (&pools)[PoolCount],
    const RuntimeConfig& config,
    const ActionType action_type,
    const SoundCategoryMask category_mask,
    const bool allow_fallback,
    const uint32_t roll) {
  uint32_t total_weight = 0;

  for (size_t index = 0; index < PoolCount; ++index) {
    const SoundPool pool = readPool(pools, index);
    if (!matchesAction(pool, action_type)) {
      continue;
    }

    if (!matchesCategoryMask(pool, category_mask)) {
      continue;
    }

    if (!isEnabledCategory(config, pool.category, allow_fallback)) {
      continue;
    }

    const uint8_t cat_weight = categoryWeight(config, pool.category, allow_fallback);
    if (cat_weight == 0) {
      continue;
    }

    total_weight += static_cast<uint32_t>(pool.weight) * static_cast<uint32_t>(cat_weight);
  }

  if (total_weight == 0) {
    SoundSelection invalid = {
      false,
      {
        0,
        0,
        0,
        SoundCategory::BaseNormal,
        0,
        0,
        0,
        0,
        kNoVolumeOverride,
        kNoMotionPatternId,
        kNoSequenceGroup,
        kNoSequenceGroup
      }
    };
    return invalid;
  }

  const uint32_t pick = roll % total_weight;
  uint32_t accumulated = 0;

  for (size_t index = 0; index < PoolCount; ++index) {
    const SoundPool pool = readPool(pools, index);
    if (!matchesAction(pool, action_type)) {
      continue;
    }

    if (!matchesCategoryMask(pool, category_mask)) {
      continue;
    }

    if (!isEnabledCategory(config, pool.category, allow_fallback)) {
      continue;
    }

    const uint8_t cat_weight = categoryWeight(config, pool.category, allow_fallback);
    if (cat_weight == 0) {
      continue;
    }

    const uint32_t effective_weight = static_cast<uint32_t>(pool.weight) * static_cast<uint32_t>(cat_weight);
    accumulated += effective_weight;
    if (pick < accumulated) {
      return makeSelection(pool, roll);
    }
  }

  SoundSelection invalid = {
    false,
    {
      0,
      0,
      0,
      SoundCategory::BaseNormal,
      0,
      0,
      0,
      0,
      kNoVolumeOverride,
      kNoMotionPatternId,
      kNoSequenceGroup,
      kNoSequenceGroup
    }
  };
  return invalid;
}

}  // namespace

bool hasEnabledBaseGameplayCategory(const RuntimeConfig& config) {
  return hasBaseCategoryInMask(config.enabled_categories);
}

SoundSelection chooseStartupSound(const RuntimeConfig& config, const uint32_t roll) {
  return chooseFromPools(kStartupPools, config, ActionType::Startup, toSoundCategoryMask(SoundCategory::Startup), false, roll);
}

SoundSelection chooseCaseEventSound(const RuntimeConfig& config, const ActionType action_type, const uint32_t roll) {
  return chooseFromPools(kCaseEventPools, config, action_type, toSoundCategoryMask(SoundCategory::CaseEvents), false, roll);
}

SoundSelection chooseSpamReactionSound(const RuntimeConfig& config, const uint32_t roll) {
  return chooseFromPools(kSpamReactionPools, config, ActionType::SpamReaction, toSoundCategoryMask(SoundCategory::SpamReactions), false, roll);
}

SoundSelection chooseGameplaySound(const RuntimeConfig& config, const ActionType result_action, const uint32_t roll) {
  if (result_action != ActionType::Safe && result_action != ActionType::Lose) {
    SoundSelection invalid = {
      false,
      {
        0,
        0,
        0,
        SoundCategory::BaseNormal,
        0,
        0,
        0,
        0,
        kNoVolumeOverride,
        kNoMotionPatternId,
        kNoSequenceGroup,
        kNoSequenceGroup
      }
    };
    return invalid;
  }

  SoundSelection selection = chooseFromPools(
      kGameplayPools,
      config,
      result_action,
      kGameplayCategoryMask,
      false,
      roll);
  if (selection.valid) {
    return selection;
  }

  return chooseFromPools(
      kGameplayPools,
      config,
      result_action,
      kFallbackGameplayCategoryMask,
      true,
      roll);
}

void resetSoundCatalogState() {
  for (uint8_t index = 0; index < kPoolStateCount; ++index) {
    const PoolStateId state_id = static_cast<PoolStateId>(index);
    clearPoolState(state_id);
    poolMeta(state_id).last_file_index = 0;
  }
}

}  // namespace domain_sound
}  // namespace uno_extreme
