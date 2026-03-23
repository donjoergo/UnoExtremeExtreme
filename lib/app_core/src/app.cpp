#include "app_core/app.h"

#include <Arduino.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config/defaults.h"
#include "domain_motion/motion_catalog.h"
#include "domain_sound/sound_catalog.h"
#include "platform_nano/config_store.h"
#include "platform_nano/hardware.h"
#include "platform_nano/service_transport.h"

namespace uno_extreme {
namespace app_core {
namespace {

constexpr uint8_t kLoseChancePercent = 30;
constexpr uint8_t kSafeForwardSpeed = 170;
constexpr uint16_t kSafeForwardDurationMs = 140;
constexpr uint8_t kLoseForwardSpeed = 240;
constexpr uint16_t kLoseForwardDurationMs = 220;
constexpr uint16_t kActionLeadDelayMs = 150;
constexpr uint8_t kMinimumForwardSpeed = 120;
constexpr uint16_t kMinimumForwardDurationMs = 25;
constexpr uint16_t kActionPollIntervalMs = 10;
constexpr uint16_t kLifecyclePlaybackTimeoutMs = 5000;
constexpr uint16_t kGameplayPlaybackTimeoutMs = 5000;
constexpr uint16_t kSpamPlaybackTimeoutMs = 3000;
constexpr SoundCategoryMask kAllSoundCategoriesMask =
    static_cast<SoundCategoryMask>((1u << kSoundCategoryCount) - 1u);
constexpr SoundCategoryMask kBaseGameplayCategoryMask =
    toSoundCategoryMask(SoundCategory::BaseNormal) |
    toSoundCategoryMask(SoundCategory::BaseFunny) |
    toSoundCategoryMask(SoundCategory::BaseTts);
constexpr uint8_t kMaxCategoryWeight = 100;
constexpr uint8_t kMinVolume = 0;
constexpr uint8_t kMaxVolume = 30;
constexpr uint16_t kMinDebounceMs = 1;
constexpr uint16_t kMaxDebounceMs = 1000;
constexpr uint16_t kMaxRapidPressWindowMs = 10000;
constexpr uint32_t kMinOpenWarningDelayMs = 1000UL;
constexpr uint32_t kMaxOpenWarningDelayMs = 600000UL;
constexpr uint32_t kMinOpenWarningRepeatMs = 1000UL;
constexpr uint32_t kMaxOpenWarningRepeatMs = 600000UL;
constexpr uint8_t kMinRetractionSpeed = 1;
constexpr uint16_t kMaxRetractionDurationMs = 2000;

struct AppContext {
  RuntimeConfig runtime_config;
  RuntimeStatus runtime_status;
  domain_motion::MotionPressHistory motion_press_history;
  domain_motion::MotionSelectorState motion_selector_state;
  bool pending_spam_reaction;
  bool spam_reaction_played;
  bool config_dirty;
};

AppContext g_app_context = {
  config::defaultRuntimeConfig(),
  {
    RuntimeState::MaintenanceOpen,
    false,
    false,
    0,
    0,
    0,
    ActionType::Startup
  },
  {{0}, 0},
  {0, 0},
  false,
  false,
  false
};

void handleCaseOpened(uint32_t now_ms);
void processQueuedActionEvents();
bool actionCanContinue();
domain_motion::MotionSelection fallbackMotionSelection(ActionType resolved_action, uint8_t intensity_percent);
RuntimeConfig sanitizeRuntimeConfig(const RuntimeConfig& candidate);
void resetEphemeralRuntimeState();
void applyRuntimeConfig(const RuntimeConfig& config, bool mark_dirty, bool reset_runtime_state);
void pollServiceInterface();

uint8_t clampU8(const uint8_t value, const uint8_t min_value, const uint8_t max_value) {
  if (value < min_value) {
    return min_value;
  }
  if (value > max_value) {
    return max_value;
  }
  return value;
}

uint16_t clampU16(const uint16_t value, const uint16_t min_value, const uint16_t max_value) {
  if (value < min_value) {
    return min_value;
  }
  if (value > max_value) {
    return max_value;
  }
  return value;
}

uint32_t clampU32(const uint32_t value, const uint32_t min_value, const uint32_t max_value) {
  if (value < min_value) {
    return min_value;
  }
  if (value > max_value) {
    return max_value;
  }
  return value;
}

RuntimeState readyStateForMode(const GameMode game_mode) {
  if (game_mode == GameMode::Extreme) {
    return RuntimeState::IdleReady;
  }

  return RuntimeState::PlaceholderMode;
}

void recordLifecycleAction(const ActionType action_type) {
  g_app_context.runtime_status.last_lifecycle_action = action_type;
}

uint32_t nextSoundRoll() {
  return (static_cast<uint32_t>(random(32768L)) << 16) |
      static_cast<uint32_t>(random(32768L));
}

void recordPressActivity(const uint32_t now_ms) {
  domain_motion::recordPress(g_app_context.motion_press_history, now_ms);
}

uint8_t scaleForwardSpeed(const uint8_t base_speed, const uint8_t intensity_percent) {
  if (intensity_percent >= 100u) {
    return base_speed;
  }

  uint16_t scaled_speed =
      static_cast<uint16_t>(base_speed) * static_cast<uint16_t>(intensity_percent) / 100u;
  if (scaled_speed < kMinimumForwardSpeed) {
    scaled_speed = kMinimumForwardSpeed;
  }
  if (scaled_speed > 255u) {
    scaled_speed = 255u;
  }
  return static_cast<uint8_t>(scaled_speed);
}

uint16_t scaleForwardDuration(const uint16_t base_duration_ms, const uint8_t intensity_percent) {
  if (intensity_percent >= 100u) {
    return base_duration_ms;
  }

  uint32_t scaled_duration =
      static_cast<uint32_t>(base_duration_ms) * static_cast<uint32_t>(intensity_percent) / 100u;
  if (scaled_duration < kMinimumForwardDurationMs) {
    scaled_duration = kMinimumForwardDurationMs;
  }
  return static_cast<uint16_t>(scaled_duration);
}

domain_sound::SoundSelection invalidSoundSelection() {
  domain_sound::SoundSelection selection = {
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
      domain_sound::kNoVolumeOverride,
      domain_sound::kNoMotionPatternId,
      domain_sound::kNoSequenceGroup,
      domain_sound::kNoSequenceGroup
    }
  };
  return selection;
}

domain_sound::SoundSelection chooseLifecycleSound(const ActionType action_type) {
  const uint32_t roll = nextSoundRoll();

  switch (action_type) {
    case ActionType::Startup:
      return domain_sound::chooseStartupSound(g_app_context.runtime_config, roll);
    case ActionType::CaseOpened:
    case ActionType::CaseClosed:
    case ActionType::CaseOpenTooLong:
      return domain_sound::chooseCaseEventSound(g_app_context.runtime_config, action_type, roll);
    case ActionType::SpamReaction:
      return domain_sound::chooseSpamReactionSound(g_app_context.runtime_config, roll);
    default:
      return invalidSoundSelection();
  }
}

bool startSoundPlayback(const domain_sound::SoundSelection& selection) {
  if (!selection.valid) {
    return false;
  }

  return platform_nano::playFolderSound(
      selection.item.folder_id,
      selection.item.file_index,
      selection.item.volume_override);
}

void restoreConfiguredVolume() {
  platform_nano::setMasterVolume(g_app_context.runtime_config.master_volume);
}

domain_motion::MotionSelection chooseMotionSelection(
    const domain_sound::SoundSelection& sound_selection,
    const ActionType resolved_action,
    const uint8_t intensity_percent) {
  if (sound_selection.valid && domain_sound::hasMotionPatternBinding(sound_selection.item)) {
    const domain_motion::MotionSelection bound_selection =
        domain_motion::choosePatternById(sound_selection.item.motion_pattern_id, intensity_percent);
    if (bound_selection.valid) {
      return bound_selection;
    }
  }

  domain_motion::MotionSelection motion_selection = domain_motion::choosePattern(
      resolved_action,
      nextSoundRoll(),
      g_app_context.motion_selector_state,
      intensity_percent);
  if (motion_selection.valid) {
    return motion_selection;
  }

  return fallbackMotionSelection(resolved_action, intensity_percent);
}

domain_motion::MotionSelection fallbackMotionSelection(
    const ActionType resolved_action,
    const uint8_t intensity_percent) {
  const bool is_lose = resolved_action == ActionType::Lose;
  const uint8_t speed = is_lose ? kLoseForwardSpeed : kSafeForwardSpeed;
  const uint16_t duration_ms = is_lose ? kLoseForwardDurationMs : kSafeForwardDurationMs;

  domain_motion::MotionSelection selection = {
    true,
    {
      9000,
      domain_motion::MotionPatternKind::VariableSpeed,
      toActionMask(resolved_action),
      100,
      1,
      {
        {domain_motion::MotionDirection::Forward, speed, duration_ms, 0},
        {domain_motion::MotionDirection::Stop, 0, 0, 0},
        {domain_motion::MotionDirection::Stop, 0, 0, 0},
        {domain_motion::MotionDirection::Stop, 0, 0, 0}
      }
    },
    intensity_percent
  };
  return selection;
}

RuntimeConfig sanitizeRuntimeConfig(const RuntimeConfig& candidate) {
  RuntimeConfig sanitized = candidate;
  sanitized.schema_version = config::kRuntimeConfigSchemaVersion;

  const uint8_t mode_index = static_cast<uint8_t>(sanitized.game_mode);
  if (mode_index >= kGameModeCount) {
    sanitized.game_mode = config::defaultRuntimeConfig().game_mode;
  }

  sanitized.enabled_categories &= kAllSoundCategoriesMask;
  if ((sanitized.enabled_categories & kBaseGameplayCategoryMask) == 0) {
    sanitized.enabled_categories |= toSoundCategoryMask(SoundCategory::BaseFunny);
  }

  for (uint8_t index = 0; index < kCategoryWeightCount; ++index) {
    sanitized.category_weights[index] =
        clampU8(sanitized.category_weights[index], 0, kMaxCategoryWeight);
  }

  const uint8_t base_funny_index = static_cast<uint8_t>(SoundCategory::BaseFunny);
  const uint8_t base_normal_index = static_cast<uint8_t>(SoundCategory::BaseNormal);
  const uint8_t base_tts_index = static_cast<uint8_t>(SoundCategory::BaseTts);
  if (sanitized.category_weights[base_normal_index] == 0 &&
      sanitized.category_weights[base_funny_index] == 0 &&
      sanitized.category_weights[base_tts_index] == 0) {
    sanitized.category_weights[base_funny_index] =
        config::defaultRuntimeConfig().category_weights[base_funny_index];
  }

  sanitized.master_volume = clampU8(sanitized.master_volume, kMinVolume, kMaxVolume);
  sanitized.button_debounce_ms =
      clampU16(sanitized.button_debounce_ms, kMinDebounceMs, kMaxDebounceMs);
  sanitized.case_debounce_ms =
      clampU16(sanitized.case_debounce_ms, kMinDebounceMs, kMaxDebounceMs);
  sanitized.rapid_press_window_ms =
      clampU16(sanitized.rapid_press_window_ms, 0, kMaxRapidPressWindowMs);

  for (uint8_t index = 0; index < kRapidPressPenaltyStepCount; ++index) {
    sanitized.rapid_press_penalty_steps[index].intensity_percent = clampU8(
        sanitized.rapid_press_penalty_steps[index].intensity_percent,
        0,
        kMaxCategoryWeight);
  }

  sanitized.open_warning_delay_ms =
      clampU32(sanitized.open_warning_delay_ms, kMinOpenWarningDelayMs, kMaxOpenWarningDelayMs);
  sanitized.open_warning_repeat_ms =
      clampU32(sanitized.open_warning_repeat_ms, kMinOpenWarningRepeatMs, kMaxOpenWarningRepeatMs);
  sanitized.retraction_speed =
      clampU8(sanitized.retraction_speed, kMinRetractionSpeed, 255);
  sanitized.retraction_duration_ms =
      clampU16(sanitized.retraction_duration_ms, 0, kMaxRetractionDurationMs);

  return sanitized;
}

void resetEphemeralRuntimeState() {
  domain_sound::resetSoundCatalogState();
  domain_motion::reset(g_app_context.motion_selector_state);
  g_app_context.motion_press_history.count = 0;
  for (uint8_t index = 0; index < domain_motion::kMotionPressHistorySize; ++index) {
    g_app_context.motion_press_history.timestamps[index] = 0;
  }
  g_app_context.pending_spam_reaction = false;
  g_app_context.spam_reaction_played = false;
}

void applyRuntimeConfig(
    const RuntimeConfig& config,
    const bool mark_dirty,
    const bool reset_runtime_state) {
  g_app_context.runtime_config = sanitizeRuntimeConfig(config);
  g_app_context.config_dirty = mark_dirty;
  if (reset_runtime_state) {
    resetEphemeralRuntimeState();
  }
  platform_nano::setMasterVolume(g_app_context.runtime_config.master_volume);
}

void renderState(const RuntimeState state) {
  platform_nano::applySafeIdle();

  switch (state) {
    case RuntimeState::IdleReady:
      platform_nano::applyFeedback(platform_nano::FeedbackState::Ready);
      break;
    case RuntimeState::MaintenanceOpen:
      platform_nano::applyFeedback(platform_nano::FeedbackState::Maintenance);
      break;
    case RuntimeState::ActionRunning:
      platform_nano::applyFeedback(platform_nano::FeedbackState::Active);
      break;
    case RuntimeState::WarningOpenTooLong:
      platform_nano::applyFeedback(platform_nano::FeedbackState::Warning);
      break;
    case RuntimeState::PlaceholderMode:
      platform_nano::applyFeedback(platform_nano::FeedbackState::Placeholder);
      break;
  }
}

void transitionToState(const RuntimeState state, const uint32_t now_ms) {
  (void)now_ms;
  if (g_app_context.runtime_status.state != state) {
    g_app_context.runtime_status.state = state;
  }

  renderState(state);
}

void updateReadyState(const uint32_t now_ms) {
  transitionToState(readyStateForMode(g_app_context.runtime_config.game_mode), now_ms);
}

void playLifecycleFeedback(const ActionType action_type, const RuntimeState resume_state) {
  recordLifecycleAction(action_type);

  const domain_sound::SoundSelection selection = chooseLifecycleSound(action_type);
  if (!selection.valid) {
    transitionToState(resume_state, millis());
    return;
  }

  transitionToState(RuntimeState::ActionRunning, millis());
  if (selection.item.wait_before_ms > 0) {
    (void)platform_nano::waitMilliseconds(selection.item.wait_before_ms);
  }
  if (startSoundPlayback(selection)) {
    platform_nano::waitForPlaybackFinish(kLifecyclePlaybackTimeoutMs);
    restoreConfiguredVolume();
  }
  if (selection.item.wait_after_ms > 0) {
    (void)platform_nano::waitMilliseconds(selection.item.wait_after_ms);
  }

  transitionToState(resume_state, millis());
}

void handleCaseOpened(const uint32_t now_ms) {
  g_app_context.runtime_status.case_closed = false;
  g_app_context.runtime_status.case_open_since_ms = now_ms;
  g_app_context.runtime_status.last_open_warning_ms = 0;
  playLifecycleFeedback(ActionType::CaseOpened, RuntimeState::MaintenanceOpen);
}

void handleCaseClosed(const uint32_t now_ms) {
  g_app_context.runtime_status.case_closed = true;
  g_app_context.runtime_status.case_open_since_ms = 0;
  g_app_context.runtime_status.last_open_warning_ms = 0;
  playLifecycleFeedback(ActionType::CaseClosed, readyStateForMode(g_app_context.runtime_config.game_mode));
}

void markCaseOpenedDuringAction(const uint32_t now_ms) {
  if (!g_app_context.runtime_status.case_closed) {
    return;
  }

  g_app_context.runtime_status.case_closed = false;
  g_app_context.runtime_status.case_open_since_ms = now_ms;
  g_app_context.runtime_status.last_open_warning_ms = 0;
  recordLifecycleAction(ActionType::CaseOpened);
  transitionToState(RuntimeState::MaintenanceOpen, now_ms);
}

void queueSpamReaction(const uint32_t now_ms) {
  recordPressActivity(now_ms);
  if (g_app_context.runtime_status.state != RuntimeState::ActionRunning ||
      !g_app_context.runtime_status.case_closed) {
    return;
  }

  if (g_app_context.spam_reaction_played) {
    return;
  }

  g_app_context.pending_spam_reaction = true;
}

bool actionCanContinue() {
  return g_app_context.runtime_status.state == RuntimeState::ActionRunning &&
      g_app_context.runtime_status.case_closed;
}

void processActionEvent(const platform_nano::InputEvent& event) {
  switch (event.type) {
    case platform_nano::InputEventType::None:
      break;
    case platform_nano::InputEventType::ButtonPressed:
      if (g_app_context.runtime_status.case_closed) {
        queueSpamReaction(event.timestamp_ms);
      }
      break;
    case platform_nano::InputEventType::CaseOpened:
      markCaseOpenedDuringAction(event.timestamp_ms);
      break;
    case platform_nano::InputEventType::CaseClosed:
      g_app_context.runtime_status.case_closed = true;
      g_app_context.runtime_status.case_open_since_ms = 0;
      g_app_context.runtime_status.last_open_warning_ms = 0;
      break;
  }
}

void processQueuedActionEvents() {
  uint32_t now_ms = millis();
  platform_nano::InputEvent event = platform_nano::pollInputEvent(now_ms, g_app_context.runtime_config);
  while (event.type != platform_nano::InputEventType::None) {
    processActionEvent(event);
    now_ms = millis();
    event = platform_nano::pollInputEvent(now_ms, g_app_context.runtime_config);
  }
}

bool executeMotionSegment(
    const domain_motion::MotionSegment& segment,
    const uint8_t intensity_percent) {
  switch (segment.direction) {
    case domain_motion::MotionDirection::Forward:
      return platform_nano::runMotorForward(
          scaleForwardSpeed(segment.speed, intensity_percent),
          scaleForwardDuration(segment.duration_ms, intensity_percent));
    case domain_motion::MotionDirection::Reverse:
      return platform_nano::runMotorReverse(segment.speed, segment.duration_ms);
    case domain_motion::MotionDirection::Stop:
      return platform_nano::waitMilliseconds(segment.duration_ms);
  }

  return false;
}

bool executeMotionPattern(const domain_motion::MotionSelection& motion_selection) {
  for (uint8_t index = 0; index < motion_selection.pattern.segment_count; ++index) {
    const domain_motion::MotionSegment& segment = motion_selection.pattern.segments[index];
    if (!executeMotionSegment(segment, motion_selection.intensity_percent)) {
      processQueuedActionEvents();
      return false;
    }

    processQueuedActionEvents();
    if (!actionCanContinue()) {
      return false;
    }

    transitionToState(RuntimeState::ActionRunning, millis());

    if (segment.pause_after_ms == 0) {
      continue;
    }

    if (!platform_nano::waitMilliseconds(segment.pause_after_ms)) {
      processQueuedActionEvents();
      return false;
    }

    processQueuedActionEvents();
    if (!actionCanContinue()) {
      return false;
    }

    transitionToState(RuntimeState::ActionRunning, millis());
  }

  return true;
}

bool waitForGameplayPlaybackTail(const bool sound_started) {
  if (!sound_started) {
    return true;
  }

  const uint32_t start_ms = millis();
  while ((millis() - start_ms) < kGameplayPlaybackTimeoutMs) {
    processQueuedActionEvents();
    if (!platform_nano::isPlaybackActive()) {
      restoreConfiguredVolume();
      return true;
    }
    delay(kActionPollIntervalMs);
  }

  restoreConfiguredVolume();
  return false;
}

void playPendingSpamReaction() {
  if (!g_app_context.pending_spam_reaction || g_app_context.spam_reaction_played) {
    return;
  }

  if (g_app_context.runtime_status.state != RuntimeState::ActionRunning ||
      !g_app_context.runtime_status.case_closed) {
    g_app_context.pending_spam_reaction = false;
    return;
  }

  g_app_context.pending_spam_reaction = false;
  g_app_context.spam_reaction_played = true;
  recordLifecycleAction(ActionType::SpamReaction);

  const domain_sound::SoundSelection selection =
      domain_sound::chooseSpamReactionSound(g_app_context.runtime_config, nextSoundRoll());
  if (!selection.valid) {
    return;
  }

  transitionToState(RuntimeState::ActionRunning, millis());
  if (selection.item.wait_before_ms > 0) {
    (void)platform_nano::waitMilliseconds(selection.item.wait_before_ms);
  }
  if (!startSoundPlayback(selection)) {
    return;
  }

  const uint32_t start_ms = millis();
  while ((millis() - start_ms) < kSpamPlaybackTimeoutMs) {
    processQueuedActionEvents();
    if (!platform_nano::isPlaybackActive()) {
      restoreConfiguredVolume();
      if (selection.item.wait_after_ms > 0) {
        (void)platform_nano::waitMilliseconds(selection.item.wait_after_ms);
      }
      return;
    }
    delay(kActionPollIntervalMs);
  }

  restoreConfiguredVolume();
}

bool runRetraction() {
  if (!platform_nano::waitMilliseconds(20)) {
    processQueuedActionEvents();
    return false;
  }

  const bool retraction_completed = platform_nano::runMotorReverse(
      g_app_context.runtime_config.retraction_speed,
      g_app_context.runtime_config.retraction_duration_ms);
  processQueuedActionEvents();
  return retraction_completed;
}

void executeExtremeAction(const ActionType resolved_action, const uint32_t trigger_ms) {
  g_app_context.pending_spam_reaction = false;
  g_app_context.spam_reaction_played = false;

  const uint8_t intensity_percent = domain_motion::computeIntensity(
      g_app_context.runtime_config,
      g_app_context.motion_press_history,
      trigger_ms);
  const domain_sound::SoundSelection sound_selection =
      domain_sound::chooseGameplaySound(g_app_context.runtime_config, resolved_action, nextSoundRoll());
  const domain_motion::MotionSelection motion_selection =
      chooseMotionSelection(sound_selection, resolved_action, intensity_percent);

  transitionToState(RuntimeState::ActionRunning, millis());

  const bool sound_started = startSoundPlayback(sound_selection);
  if (sound_started) {
    (void)platform_nano::waitForPlaybackStart(250);
  }

  const uint16_t wait_before_ms =
      sound_selection.valid ? sound_selection.item.wait_before_ms : kActionLeadDelayMs;
  if (platform_nano::waitMilliseconds(wait_before_ms)) {
    processQueuedActionEvents();
  } else {
    processQueuedActionEvents();
  }

  if (actionCanContinue()) {
    const bool motion_completed = executeMotionPattern(motion_selection);
    if (motion_completed && actionCanContinue()) {
      (void)runRetraction();
    }
  }

  (void)waitForGameplayPlaybackTail(sound_started);
  if (sound_selection.valid && sound_selection.item.wait_after_ms > 0) {
    (void)platform_nano::waitMilliseconds(sound_selection.item.wait_after_ms);
  }
  processQueuedActionEvents();
  playPendingSpamReaction();
  processQueuedActionEvents();

  const uint32_t end_ms = millis();
  if (g_app_context.runtime_status.case_closed) {
    updateReadyState(end_ms);
  } else {
    transitionToState(RuntimeState::MaintenanceOpen, end_ms);
  }
}

void handleButtonPressed(const uint32_t now_ms) {
  if (!g_app_context.runtime_status.case_closed) {
    return;
  }

  if (g_app_context.runtime_status.state == RuntimeState::ActionRunning) {
    queueSpamReaction(now_ms);
    return;
  }

  if (g_app_context.runtime_status.state != RuntimeState::IdleReady) {
    return;
  }

  if (g_app_context.runtime_config.game_mode != GameMode::Extreme) {
    return;
  }

  recordPressActivity(now_ms);
  g_app_context.runtime_status.accepted_button_presses++;

  const ActionType resolved_action =
      (random(100) < kLoseChancePercent) ? ActionType::Lose : ActionType::Safe;
  executeExtremeAction(resolved_action, now_ms);
}

void processInputEvent(const platform_nano::InputEvent& event) {
  switch (event.type) {
    case platform_nano::InputEventType::None:
      break;
    case platform_nano::InputEventType::ButtonPressed:
      handleButtonPressed(event.timestamp_ms);
      break;
    case platform_nano::InputEventType::CaseOpened:
      handleCaseOpened(event.timestamp_ms);
      break;
    case platform_nano::InputEventType::CaseClosed:
      handleCaseClosed(event.timestamp_ms);
      break;
  }
}

void updateOpenWarning(const uint32_t now_ms) {
  if (g_app_context.runtime_status.case_closed) {
    return;
  }

  if (g_app_context.runtime_status.case_open_since_ms == 0) {
    g_app_context.runtime_status.case_open_since_ms = now_ms;
    return;
  }

  const uint32_t case_open_duration_ms = now_ms - g_app_context.runtime_status.case_open_since_ms;
  if (g_app_context.runtime_status.state != RuntimeState::WarningOpenTooLong) {
    if (case_open_duration_ms < g_app_context.runtime_config.open_warning_delay_ms) {
      return;
    }

    transitionToState(RuntimeState::WarningOpenTooLong, now_ms);
    g_app_context.runtime_status.last_open_warning_ms = now_ms;
    playLifecycleFeedback(ActionType::CaseOpenTooLong, RuntimeState::WarningOpenTooLong);
    return;
  }

  if ((now_ms - g_app_context.runtime_status.last_open_warning_ms) < g_app_context.runtime_config.open_warning_repeat_ms) {
    return;
  }

  g_app_context.runtime_status.last_open_warning_ms = now_ms;
  playLifecycleFeedback(ActionType::CaseOpenTooLong, RuntimeState::WarningOpenTooLong);
}

const char* gameModeName(const GameMode game_mode) {
  if (game_mode == GameMode::Normal) {
    return "Normal";
  }
  if (game_mode == GameMode::Ascending) {
    return "Ascending";
  }
  if (game_mode == GameMode::Extreme) {
    return "Extreme";
  }
  return "Unknown";
}

const char* runtimeStateName(const RuntimeState state) {
  if (state == RuntimeState::IdleReady) {
    return "IdleReady";
  }
  if (state == RuntimeState::MaintenanceOpen) {
    return "MaintenanceOpen";
  }
  if (state == RuntimeState::ActionRunning) {
    return "ActionRunning";
  }
  if (state == RuntimeState::WarningOpenTooLong) {
    return "WarningOpenTooLong";
  }
  if (state == RuntimeState::PlaceholderMode) {
    return "PlaceholderMode";
  }
  return "Unknown";
}

const char* actionTypeName(const ActionType action_type) {
  if (action_type == ActionType::Startup) {
    return "Startup";
  }
  if (action_type == ActionType::Safe) {
    return "Safe";
  }
  if (action_type == ActionType::Lose) {
    return "Lose";
  }
  if (action_type == ActionType::Wait) {
    return "Wait";
  }
  if (action_type == ActionType::SpamReaction) {
    return "SpamReaction";
  }
  if (action_type == ActionType::CaseOpened) {
    return "CaseOpened";
  }
  if (action_type == ActionType::CaseClosed) {
    return "CaseClosed";
  }
  if (action_type == ActionType::CaseOpenTooLong) {
    return "CaseOpenTooLong";
  }
  if (action_type == ActionType::CreativeScript) {
    return "CreativeScript";
  }
  if (action_type == ActionType::MiniGame) {
    return "MiniGame";
  }
  return "Unknown";
}

char* trimWhitespace(char* text) {
  while (*text == ' ' || *text == '\t') {
    text++;
  }

  size_t length = strlen(text);
  while (length > 0) {
    char* tail = text + length - 1u;
    if (*tail != ' ' && *tail != '\t') {
      break;
    }
    *tail = '\0';
    length--;
  }

  return text;
}

bool parseUint32(const char* text, uint32_t& value) {
  if (text == nullptr || *text == '\0') {
    return false;
  }

  char* end_ptr = nullptr;
  const unsigned long parsed = strtoul(text, &end_ptr, 0);
  if (end_ptr == text || *trimWhitespace(end_ptr) != '\0') {
    return false;
  }

  value = static_cast<uint32_t>(parsed);
  return true;
}

bool parseCommand(char* line, char*& command, char*& args) {
  command = trimWhitespace(line);
  if (*command == '\0') {
    args = command;
    return false;
  }

  char* open_paren = strchr(command, '(');
  if (open_paren != nullptr) {
    *open_paren = '\0';
    args = trimWhitespace(open_paren + 1);
    char* close_paren = strrchr(args, ')');
    if (close_paren != nullptr) {
      *close_paren = '\0';
    }
  } else {
    args = command + strlen(command);
  }

  command = trimWhitespace(command);
  args = trimWhitespace(args);
  return true;
}

bool splitArgs(char* args, char*& first, char*& second) {
  char* comma = strchr(args, ',');
  if (comma == nullptr) {
    return false;
  }

  *comma = '\0';
  first = trimWhitespace(args);
  second = trimWhitespace(comma + 1);
  return *first != '\0' && *second != '\0';
}

bool configMutationAllowed() {
  return !g_app_context.runtime_status.case_closed &&
      g_app_context.runtime_status.state != RuntimeState::ActionRunning;
}

bool saveCurrentRuntimeConfig() {
  const bool saved = platform_nano::saveRuntimeConfig(g_app_context.runtime_config);
  if (saved) {
    g_app_context.config_dirty = false;
  }
  return saved;
}

void sendStatusReply(const platform_nano::ServiceChannel channel) {
  char buffer[160];
  snprintf(
      buffer,
      sizeof(buffer),
      "STATUS state=%s mode=%s case_closed=%u boot=%u schema=%u dirty=%u accepted_presses=%u last_action=%s",
      runtimeStateName(g_app_context.runtime_status.state),
      gameModeName(g_app_context.runtime_config.game_mode),
      g_app_context.runtime_status.case_closed ? 1u : 0u,
      g_app_context.runtime_status.boot_completed ? 1u : 0u,
      static_cast<unsigned int>(g_app_context.runtime_config.schema_version),
      g_app_context.config_dirty ? 1u : 0u,
      static_cast<unsigned int>(g_app_context.runtime_status.accepted_button_presses),
      actionTypeName(g_app_context.runtime_status.last_lifecycle_action));
  platform_nano::sendServiceReply(channel, buffer);
}

void sendConfigReply(const platform_nano::ServiceChannel channel) {
  char buffer[196];
  snprintf(
      buffer,
      sizeof(buffer),
      "CONFIG schema_version=%u game_mode=%u enabled_categories=%u master_volume=%u",
      static_cast<unsigned int>(g_app_context.runtime_config.schema_version),
      static_cast<unsigned int>(g_app_context.runtime_config.game_mode),
      static_cast<unsigned int>(g_app_context.runtime_config.enabled_categories),
      static_cast<unsigned int>(g_app_context.runtime_config.master_volume));
  platform_nano::sendServiceReply(channel, buffer);

  snprintf(
      buffer,
      sizeof(buffer),
      "WEIGHTS 0=%u 1=%u 2=%u 3=%u 4=%u 5=%u 6=%u 7=%u 8=%u 9=%u 10=%u",
      g_app_context.runtime_config.category_weights[0],
      g_app_context.runtime_config.category_weights[1],
      g_app_context.runtime_config.category_weights[2],
      g_app_context.runtime_config.category_weights[3],
      g_app_context.runtime_config.category_weights[4],
      g_app_context.runtime_config.category_weights[5],
      g_app_context.runtime_config.category_weights[6],
      g_app_context.runtime_config.category_weights[7],
      g_app_context.runtime_config.category_weights[8],
      g_app_context.runtime_config.category_weights[9],
      g_app_context.runtime_config.category_weights[10]);
  platform_nano::sendServiceReply(channel, buffer);

  snprintf(
      buffer,
      sizeof(buffer),
      "TIMING button_debounce_ms=%u case_debounce_ms=%u rapid_press_window_ms=%u open_warning_delay_ms=%lu open_warning_repeat_ms=%lu retraction_speed=%u retraction_duration_ms=%u",
      g_app_context.runtime_config.button_debounce_ms,
      g_app_context.runtime_config.case_debounce_ms,
      g_app_context.runtime_config.rapid_press_window_ms,
      static_cast<unsigned long>(g_app_context.runtime_config.open_warning_delay_ms),
      static_cast<unsigned long>(g_app_context.runtime_config.open_warning_repeat_ms),
      g_app_context.runtime_config.retraction_speed,
      g_app_context.runtime_config.retraction_duration_ms);
  platform_nano::sendServiceReply(channel, buffer);

  snprintf(
      buffer,
      sizeof(buffer),
      "PENALTIES 0=%u:%u 1=%u:%u 2=%u:%u 3=%u:%u",
      g_app_context.runtime_config.rapid_press_penalty_steps[0].press_count,
      g_app_context.runtime_config.rapid_press_penalty_steps[0].intensity_percent,
      g_app_context.runtime_config.rapid_press_penalty_steps[1].press_count,
      g_app_context.runtime_config.rapid_press_penalty_steps[1].intensity_percent,
      g_app_context.runtime_config.rapid_press_penalty_steps[2].press_count,
      g_app_context.runtime_config.rapid_press_penalty_steps[2].intensity_percent,
      g_app_context.runtime_config.rapid_press_penalty_steps[3].press_count,
      g_app_context.runtime_config.rapid_press_penalty_steps[3].intensity_percent);
  platform_nano::sendServiceReply(channel, buffer);
}

void sendSimpleReply(
    const platform_nano::ServiceChannel channel,
    const char* prefix,
    const char* message) {
  char buffer[128];
  snprintf(buffer, sizeof(buffer), "%s %s", prefix, message);
  platform_nano::sendServiceReply(channel, buffer);
}

void sendOk(const platform_nano::ServiceChannel channel, const char* message) {
  sendSimpleReply(channel, "OK", message);
}

void sendError(const platform_nano::ServiceChannel channel, const char* message) {
  sendSimpleReply(channel, "ERR", message);
}

void pollServiceInterface() {
  platform_nano::ServiceMessage message;
  while (platform_nano::pollServiceMessage(message)) {
    char* command = nullptr;
    char* args = nullptr;
    if (!parseCommand(message.command, command, args)) {
      continue;
    }

    if (strcmp(command, "status") == 0) {
      sendStatusReply(message.channel);
      continue;
    }

    if (strcmp(command, "get_config") == 0) {
      sendConfigReply(message.channel);
      continue;
    }

    if (strcmp(command, "help") == 0) {
      platform_nano::sendServiceReply(
          message.channel,
          "COMMANDS status get_config set_mode(n) set_categories(mask) set_weight(id,value) set_volume(value) set_timing(key,value) save() factory_reset(confirm)");
      continue;
    }

    if (!configMutationAllowed()) {
      sendError(message.channel, "maintenance_mode_required");
      continue;
    }

    if (strcmp(command, "set_mode") == 0) {
      uint32_t value = 0;
      if (!parseUint32(args, value) || value >= kGameModeCount) {
        sendError(message.channel, "invalid_mode");
        continue;
      }

      RuntimeConfig updated = g_app_context.runtime_config;
      updated.game_mode = static_cast<GameMode>(value);
      applyRuntimeConfig(updated, true, true);
      sendOk(message.channel, "mode_updated");
      continue;
    }

    if (strcmp(command, "set_categories") == 0) {
      uint32_t value = 0;
      if (!parseUint32(args, value)) {
        sendError(message.channel, "invalid_category_mask");
        continue;
      }

      RuntimeConfig updated = g_app_context.runtime_config;
      updated.enabled_categories = static_cast<SoundCategoryMask>(value);
      applyRuntimeConfig(updated, true, true);
      sendOk(message.channel, "categories_updated");
      continue;
    }

    if (strcmp(command, "set_weight") == 0) {
      char* category_arg = nullptr;
      char* weight_arg = nullptr;
      uint32_t category_index = 0;
      uint32_t weight_value = 0;
      if (!splitArgs(args, category_arg, weight_arg) ||
          !parseUint32(category_arg, category_index) ||
          !parseUint32(weight_arg, weight_value) ||
          category_index >= kCategoryWeightCount ||
          weight_value > kMaxCategoryWeight) {
        sendError(message.channel, "invalid_weight");
        continue;
      }

      RuntimeConfig updated = g_app_context.runtime_config;
      updated.category_weights[category_index] = static_cast<uint8_t>(weight_value);
      applyRuntimeConfig(updated, true, false);
      sendOk(message.channel, "weight_updated");
      continue;
    }

    if (strcmp(command, "set_volume") == 0) {
      uint32_t value = 0;
      if (!parseUint32(args, value) || value > kMaxVolume) {
        sendError(message.channel, "invalid_volume");
        continue;
      }

      RuntimeConfig updated = g_app_context.runtime_config;
      updated.master_volume = static_cast<uint8_t>(value);
      applyRuntimeConfig(updated, true, false);
      sendOk(message.channel, "volume_updated");
      continue;
    }

    if (strcmp(command, "set_timing") == 0) {
      char* key_arg = nullptr;
      char* value_arg = nullptr;
      uint32_t value = 0;
      if (!splitArgs(args, key_arg, value_arg) || !parseUint32(value_arg, value)) {
        sendError(message.channel, "invalid_timing");
        continue;
      }

      RuntimeConfig updated = g_app_context.runtime_config;
      bool key_handled = true;
      if (strcmp(key_arg, "button_debounce_ms") == 0) {
        updated.button_debounce_ms = static_cast<uint16_t>(value);
      } else if (strcmp(key_arg, "case_debounce_ms") == 0) {
        updated.case_debounce_ms = static_cast<uint16_t>(value);
      } else if (strcmp(key_arg, "rapid_press_window_ms") == 0) {
        updated.rapid_press_window_ms = static_cast<uint16_t>(value);
      } else if (strcmp(key_arg, "open_warning_delay_ms") == 0) {
        updated.open_warning_delay_ms = value;
      } else if (strcmp(key_arg, "open_warning_repeat_ms") == 0) {
        updated.open_warning_repeat_ms = value;
      } else if (strcmp(key_arg, "retraction_speed") == 0) {
        updated.retraction_speed = static_cast<uint8_t>(value);
      } else if (strcmp(key_arg, "retraction_duration_ms") == 0) {
        updated.retraction_duration_ms = static_cast<uint16_t>(value);
      } else {
        key_handled = false;
      }

      if (!key_handled) {
        sendError(message.channel, "unknown_timing_key");
        continue;
      }

      applyRuntimeConfig(updated, true, false);
      sendOk(message.channel, "timing_updated");
      continue;
    }

    if (strcmp(command, "save") == 0) {
      if (!saveCurrentRuntimeConfig()) {
        sendError(message.channel, "save_failed");
        continue;
      }

      sendOk(message.channel, "saved");
      continue;
    }

    if (strcmp(command, "factory_reset") == 0) {
      if (strcmp(args, "confirm") != 0) {
        sendError(message.channel, "confirm_required");
        continue;
      }

      platform_nano::clearPersistedRuntimeConfig();
      applyRuntimeConfig(config::defaultRuntimeConfig(), false, true);
      if (!saveCurrentRuntimeConfig()) {
        sendError(message.channel, "factory_reset_save_failed");
        continue;
      }

      sendOk(message.channel, "factory_reset");
      continue;
    }

    sendError(message.channel, "unknown_command");
  }
}

}  // namespace

void setup() {
  randomSeed(analogRead(0));
  RuntimeConfig initial_config = config::defaultRuntimeConfig();
  RuntimeConfig persisted_config;
  const bool loaded_persisted_config = platform_nano::loadRuntimeConfig(persisted_config);
  if (loaded_persisted_config &&
      persisted_config.schema_version == config::kRuntimeConfigSchemaVersion) {
    initial_config = sanitizeRuntimeConfig(persisted_config);
  }

  applyRuntimeConfig(initial_config, false, true);
  platform_nano::initializeHardware(g_app_context.runtime_config);

  const uint32_t now_ms = millis();
  const platform_nano::InputSnapshot inputs = platform_nano::sampleInputs();

  g_app_context.runtime_status.boot_completed = true;
  g_app_context.runtime_status.case_closed = inputs.case_closed;
  g_app_context.runtime_status.case_open_since_ms = inputs.case_closed ? 0 : now_ms;
  g_app_context.runtime_status.last_open_warning_ms = 0;
  g_app_context.runtime_status.accepted_button_presses = 0;
  g_app_context.runtime_status.last_lifecycle_action = ActionType::Startup;

  if (inputs.case_closed) {
    playLifecycleFeedback(ActionType::Startup, readyStateForMode(g_app_context.runtime_config.game_mode));
  } else {
    transitionToState(RuntimeState::MaintenanceOpen, now_ms);
  }
}

void loop() {
  if (!g_app_context.runtime_status.boot_completed) {
    return;
  }

  const uint32_t now_ms = millis();
  platform_nano::InputEvent event = platform_nano::pollInputEvent(now_ms, g_app_context.runtime_config);
  while (event.type != platform_nano::InputEventType::None) {
    processInputEvent(event);
    event = platform_nano::pollInputEvent(now_ms, g_app_context.runtime_config);
  }

  updateOpenWarning(now_ms);
  pollServiceInterface();
}

const RuntimeConfig& runtimeConfig() {
  return g_app_context.runtime_config;
}

const RuntimeStatus& runtimeStatus() {
  return g_app_context.runtime_status;
}

}  // namespace app_core
}  // namespace uno_extreme
