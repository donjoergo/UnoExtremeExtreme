#include "app_core/app.h"

#include <Arduino.h>

#include "config/defaults.h"
#include "domain_motion/motion_catalog.h"
#include "domain_sound/sound_catalog.h"
#include "platform_nano/hardware.h"

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

struct AppContext {
  RuntimeConfig runtime_config;
  RuntimeStatus runtime_status;
  domain_motion::MotionPressHistory motion_press_history;
  domain_motion::MotionSelectorState motion_selector_state;
  bool pending_spam_reaction;
  bool spam_reaction_played;
};

AppContext g_app_context = {
  config::kDefaultRuntimeConfig,
  {
    RuntimeState::MaintenanceOpen,
    false,
    false,
    0,
    0,
    0,
    0,
    0,
    0,
    ActionType::Startup
  },
  {{0}, 0},
  {0, 0},
  false,
  false
};

void handleCaseOpened(uint32_t now_ms);
void processQueuedActionEvents();
bool actionCanContinue();

RuntimeState readyStateForMode(const GameMode game_mode) {
  if (game_mode == GameMode::Extreme) {
    return RuntimeState::IdleReady;
  }

  return RuntimeState::PlaceholderMode;
}

void recordLifecycleAction(const ActionType action_type) {
  g_app_context.runtime_status.last_lifecycle_action = action_type;
  g_app_context.runtime_status.lifecycle_action_generation++;
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
    {0, 0, 0, SoundCategory::BaseNormal, 0, 0}
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
      selection.item.file_index);
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
  if (g_app_context.runtime_status.state != state) {
    g_app_context.runtime_status.state = state;
    g_app_context.runtime_status.last_transition_ms = now_ms;
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
  if (startSoundPlayback(selection)) {
    platform_nano::waitForPlaybackFinish(kLifecyclePlaybackTimeoutMs);
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
      return true;
    }
    delay(kActionPollIntervalMs);
  }

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
  if (!startSoundPlayback(selection)) {
    return;
  }

  const uint32_t start_ms = millis();
  while ((millis() - start_ms) < kSpamPlaybackTimeoutMs) {
    processQueuedActionEvents();
    if (!platform_nano::isPlaybackActive()) {
      return;
    }
    delay(kActionPollIntervalMs);
  }
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
  domain_motion::MotionSelection motion_selection = domain_motion::choosePattern(
      resolved_action,
      nextSoundRoll(),
      g_app_context.motion_selector_state,
      intensity_percent);
  if (!motion_selection.valid) {
    motion_selection = fallbackMotionSelection(resolved_action, intensity_percent);
  }

  transitionToState(RuntimeState::ActionRunning, millis());

  const domain_sound::SoundSelection sound_selection =
      domain_sound::chooseGameplaySound(g_app_context.runtime_config, resolved_action, nextSoundRoll());
  const bool sound_started = startSoundPlayback(sound_selection);
  if (sound_started) {
    (void)platform_nano::waitForPlaybackStart(250);
  }

  if (platform_nano::waitMilliseconds(kActionLeadDelayMs)) {
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
  g_app_context.runtime_status.last_button_press_ms = now_ms;

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

}  // namespace

void setup() {
  randomSeed(analogRead(0));
  g_app_context.runtime_config = config::kDefaultRuntimeConfig;
  domain_sound::resetSoundCatalogState();
  domain_motion::reset(g_app_context.motion_selector_state);
  platform_nano::initializeHardware(g_app_context.runtime_config);

  const uint32_t now_ms = millis();
  const platform_nano::InputSnapshot inputs = platform_nano::sampleInputs();

  g_app_context.runtime_status.boot_completed = true;
  g_app_context.runtime_status.case_closed = inputs.case_closed;
  g_app_context.runtime_status.last_transition_ms = now_ms;
  g_app_context.runtime_status.case_open_since_ms = inputs.case_closed ? 0 : now_ms;
  g_app_context.runtime_status.last_open_warning_ms = 0;
  g_app_context.runtime_status.accepted_button_presses = 0;
  g_app_context.runtime_status.last_button_press_ms = 0;
  g_app_context.runtime_status.lifecycle_action_generation = 0;
  g_app_context.runtime_status.last_lifecycle_action = ActionType::Startup;
  g_app_context.motion_press_history.count = 0;
  for (uint8_t index = 0; index < domain_motion::kMotionPressHistorySize; ++index) {
    g_app_context.motion_press_history.timestamps[index] = 0;
  }
  g_app_context.pending_spam_reaction = false;
  g_app_context.spam_reaction_played = false;

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
}

const RuntimeConfig& runtimeConfig() {
  return g_app_context.runtime_config;
}

const RuntimeStatus& runtimeStatus() {
  return g_app_context.runtime_status;
}

}  // namespace app_core
}  // namespace uno_extreme
