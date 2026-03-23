#include "domain_sound/sound_catalog.h"

#include <stddef.h>

namespace uno_extreme {
namespace domain_sound {
namespace {

struct SoundPool {
  uint16_t sound_id;
  uint8_t folder_id;
  uint8_t file_count;
  SoundCategory category;
  ActionMask valid_actions;
  uint8_t weight;
};

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
constexpr SoundPool kStartupPools[] = {
  {1001, 1, 14, SoundCategory::Startup, kStartupActions, 100},
};

constexpr SoundPool kCaseEventPools[] = {
  {2001, 2, 36, SoundCategory::CaseEvents, kCaseEventActions, 100},
};

constexpr SoundPool kSpamReactionPools[] = {
  {3001, 6, 5, SoundCategory::SpamReactions, kSpamReactionActions, 100},
};

constexpr SoundPool kGameplayPools[] = {
  {4001, 3, 128, SoundCategory::BaseFunny, kSafeActions, 100},
  {4002, 4, 85, SoundCategory::BaseFunny, kLoseActions, 100},
  {5001, 8, 24, SoundCategory::BaseTts, kSafeActions, 100},
  {5002, 9, 13, SoundCategory::BaseTts, kLoseActions, 100},
};

static_assert(kSoundCategoryCount == 11u, "Unexpected SoundCategory count");
static_assert(kCategoryWeightCount == kSoundCategoryCount, "Category weights must track categories");

bool isGameplayCategory(const SoundCategory category) {
  return category == SoundCategory::BaseNormal ||
      category == SoundCategory::BaseFunny ||
      category == SoundCategory::BaseTts;
}

bool hasBaseCategoryInMask(const SoundCategoryMask mask) {
  return (mask & kGameplayCategoryMask) != 0;
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

SoundSelection makeSelection(const SoundPool& pool, const uint32_t roll) {
  SoundSelection selection = {true, {0, 0, 0, SoundCategory::BaseNormal, 0, 0}};
  selection.item.sound_id = pool.sound_id;
  selection.item.folder_id = pool.folder_id;
  selection.item.file_index = resolveFileIndex(pool.file_count, roll);
  selection.item.category = pool.category;
  selection.item.valid_actions = pool.valid_actions;
  selection.item.weight = pool.weight;
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
    const SoundPool& pool = pools[index];
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
    SoundSelection invalid = {false, {0, 0, 0, SoundCategory::BaseNormal, 0, 0}};
    return invalid;
  }

  const uint32_t pick = roll % total_weight;
  uint32_t accumulated = 0;

  for (size_t index = 0; index < PoolCount; ++index) {
    const SoundPool& pool = pools[index];
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

  SoundSelection invalid = {false, {0, 0, 0, SoundCategory::BaseNormal, 0, 0}};
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
    SoundSelection invalid = {false, {0, 0, 0, SoundCategory::BaseNormal, 0, 0}};
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

}  // namespace domain_sound
}  // namespace uno_extreme
