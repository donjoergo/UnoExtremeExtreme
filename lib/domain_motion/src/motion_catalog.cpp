#include "domain_motion/motion_catalog.h"

#include <avr/pgmspace.h>

namespace uno_extreme {
namespace domain_motion {
namespace {

constexpr ActionMask kSafeMask = toActionMask(ActionType::Safe);
constexpr ActionMask kLoseMask = toActionMask(ActionType::Lose);
constexpr ActionMask kSafeOrLoseMask = kSafeMask | kLoseMask;

const MotionPattern kMotionPatterns[kMotionPatternCount] PROGMEM = {
  {
    1001,
    MotionPatternKind::SplitEject,
    kSafeOrLoseMask,
    40,
    3,
    {
      {MotionDirection::Forward, 220, 120, 30},
      {MotionDirection::Stop, 0, 60, 20},
      {MotionDirection::Forward, 180, 80, 0},
      {MotionDirection::Stop, 0, 0, 0}
    }
  },
  {
    1002,
    MotionPatternKind::VariableSpeed,
    kSafeOrLoseMask,
    30,
    3,
    {
      {MotionDirection::Forward, 140, 80, 10},
      {MotionDirection::Forward, 180, 80, 10},
      {MotionDirection::Forward, 220, 90, 0},
      {MotionDirection::Stop, 0, 0, 0}
    }
  },
  {
    1003,
    MotionPatternKind::Stutter,
    kLoseMask,
    20,
    4,
    {
      {MotionDirection::Forward, 160, 45, 15},
      {MotionDirection::Stop, 0, 35, 20},
      {MotionDirection::Forward, 165, 45, 15},
      {MotionDirection::Forward, 210, 70, 0}
    }
  },
  {
    1004,
    MotionPatternKind::FakeFault,
    kSafeMask,
    10,
    4,
    {
      {MotionDirection::Forward, 200, 70, 10},
      {MotionDirection::Stop, 0, 140, 30},
      {MotionDirection::Forward, 120, 40, 20},
      {MotionDirection::Forward, 220, 80, 0}
    }
  }
};

static_assert(kMotionPatternCount == 4u, "Unexpected motion pattern count");
static_assert(kMotionPatternMaxSegments == 4u, "Unexpected motion segment capacity");

MotionPattern readMotionPattern(const uint8_t index) {
  MotionPattern pattern;
  memcpy_P(&pattern, &kMotionPatterns[index], sizeof(pattern));
  return pattern;
}

uint8_t eligibleMaskForAction(const ActionType result_action) {
  uint8_t mask = 0;

  for (uint8_t index = 0; index < kMotionPatternCount; ++index) {
    const MotionPattern pattern = readMotionPattern(index);
    if ((pattern.valid_actions & toActionMask(result_action)) != 0) {
      mask |= static_cast<uint8_t>(1u << index);
    }
  }

  return mask;
}

uint8_t* remainingMaskForAction(MotionSelectorState& state, const ActionType result_action) {
  if (result_action == ActionType::Safe) {
    return &state.remaining_safe_mask;
  }

  return &state.remaining_lose_mask;
}

void ensureMaskInitialized(MotionSelectorState& state, const ActionType result_action) {
  uint8_t* const remaining_mask = remainingMaskForAction(state, result_action);
  const uint8_t eligible_mask = eligibleMaskForAction(result_action);

  if (eligible_mask == 0) {
    *remaining_mask = 0;
    return;
  }

  if ((*remaining_mask & eligible_mask) == 0) {
    *remaining_mask = eligible_mask;
  } else {
    *remaining_mask &= eligible_mask;
  }
}

bool isActionEligible(const MotionPattern& pattern, const ActionType result_action) {
  return (pattern.valid_actions & toActionMask(result_action)) != 0;
}

uint8_t countPressesWithinWindow(
    const RuntimeConfig& config,
    const MotionPressHistory& history,
    const uint32_t now_ms) {
  if (history.count == 0) {
    return 0;
  }

  if (config.rapid_press_window_ms == 0) {
    return history.count;
  }

  uint8_t count = 0;
  const uint16_t now_tick = static_cast<uint16_t>(now_ms);
  for (uint8_t index = 0; index < history.count; ++index) {
    const uint16_t press_age_ms = static_cast<uint16_t>(now_tick - history.timestamps[index]);
    if (press_age_ms <= config.rapid_press_window_ms) {
      count++;
    }
  }

  return count;
}

MotionSelection makeSelection(const MotionPattern& pattern, const uint8_t intensity_percent) {
  MotionSelection selection = {true, {}, intensity_percent};
  selection.pattern = pattern;
  return selection;
}

MotionSelection invalidSelection(const uint8_t intensity_percent) {
  MotionSelection selection = {
    false,
    {
      0,
      MotionPatternKind::SplitEject,
      0,
      0,
      0,
      {
        {MotionDirection::Stop, 0, 0, 0},
        {MotionDirection::Stop, 0, 0, 0},
        {MotionDirection::Stop, 0, 0, 0},
        {MotionDirection::Stop, 0, 0, 0}
      }
    },
    intensity_percent
  };
  return selection;
}

}  // namespace

void reset(MotionSelectorState& state) {
  state.remaining_safe_mask = 0;
  state.remaining_lose_mask = 0;
}

void recordPress(MotionPressHistory& history, const uint32_t timestamp_ms) {
  const uint16_t stored_timestamp = static_cast<uint16_t>(timestamp_ms);
  if (history.count < kMotionPressHistorySize) {
    history.timestamps[history.count] = stored_timestamp;
    history.count++;
    return;
  }

  for (uint8_t index = 1; index < kMotionPressHistorySize; ++index) {
    history.timestamps[index - 1] = history.timestamps[index];
  }
  history.timestamps[kMotionPressHistorySize - 1] = stored_timestamp;
}

uint8_t computeIntensity(
    const RuntimeConfig& config,
    const MotionPressHistory& history,
    const uint32_t now_ms) {
  if (config.rapid_press_window_ms == 0) {
    return 100;
  }

  const uint8_t recent_press_count = countPressesWithinWindow(config, history, now_ms);
  if (recent_press_count <= 1) {
    return 100;
  }

  uint8_t intensity_percent = 100;
  for (uint8_t index = 0; index < kRapidPressPenaltyStepCount; ++index) {
    const RapidPressPenaltyStep& step = config.rapid_press_penalty_steps[index];
    if (step.press_count == 0) {
      continue;
    }

    if (recent_press_count >= step.press_count) {
      intensity_percent = step.intensity_percent;
    }
  }

  return intensity_percent;
}

MotionSelection choosePattern(
    const ActionType result_action,
    const uint32_t roll,
    MotionSelectorState& state,
    const uint8_t intensity_percent) {
  if (result_action != ActionType::Safe && result_action != ActionType::Lose) {
    return invalidSelection(intensity_percent);
  }

  ensureMaskInitialized(state, result_action);
  uint8_t* const remaining_mask = remainingMaskForAction(state, result_action);
  const uint8_t eligible_mask = eligibleMaskForAction(result_action);
  if (*remaining_mask == 0 || eligible_mask == 0) {
    return invalidSelection(intensity_percent);
  }

  uint32_t total_weight = 0;
  for (uint8_t index = 0; index < kMotionPatternCount; ++index) {
    const uint8_t bit = static_cast<uint8_t>(1u << index);
    if ((*remaining_mask & bit) == 0) {
      continue;
    }

    const MotionPattern pattern = readMotionPattern(index);
    if (!isActionEligible(pattern, result_action)) {
      continue;
    }

    total_weight += pattern.weight;
  }

  if (total_weight == 0) {
    *remaining_mask = 0;
    return invalidSelection(intensity_percent);
  }

  const uint32_t pick = roll % total_weight;
  uint32_t accumulated = 0;

  for (uint8_t index = 0; index < kMotionPatternCount; ++index) {
    const uint8_t bit = static_cast<uint8_t>(1u << index);
    if ((*remaining_mask & bit) == 0) {
      continue;
    }

    const MotionPattern pattern = readMotionPattern(index);
    if (!isActionEligible(pattern, result_action)) {
      continue;
    }

    accumulated += pattern.weight;
    if (pick < accumulated) {
      *remaining_mask = static_cast<uint8_t>(*remaining_mask & static_cast<uint8_t>(~bit));
      return makeSelection(pattern, intensity_percent);
    }
  }

  *remaining_mask = 0;
  return invalidSelection(intensity_percent);
}

MotionSelection choosePatternById(const uint16_t pattern_id, const uint8_t intensity_percent) {
  for (uint8_t index = 0; index < kMotionPatternCount; ++index) {
    const MotionPattern pattern = readMotionPattern(index);
    if (pattern.pattern_id == pattern_id) {
      return makeSelection(pattern, intensity_percent);
    }
  }

  return invalidSelection(intensity_percent);
}

}  // namespace domain_motion
}  // namespace uno_extreme
