#include "app_core/app.h"

#include <Arduino.h>

#include "config/defaults.h"
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
constexpr uint16_t kLifecyclePlaybackTimeoutMs = 5000;
constexpr uint16_t kGameplayPlaybackTimeoutMs = 5000;

struct AppContext {
  RuntimeConfig runtime_config;
  RuntimeStatus runtime_status;
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
  }
};

void handleCaseOpened(uint32_t now_ms);

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

void refreshStateFromHardware(const uint32_t now_ms) {
  if (platform_nano::isCaseClosed()) {
    g_app_context.runtime_status.case_closed = true;
    return;
  }

  if (g_app_context.runtime_status.case_closed) {
    handleCaseOpened(now_ms);
  }
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

void runRetraction() {
  delay(20);
  const bool retraction_completed = platform_nano::runMotorReverse(
      g_app_context.runtime_config.retraction_speed,
      g_app_context.runtime_config.retraction_duration_ms);
  if (!retraction_completed) {
    refreshStateFromHardware(millis());
  }
}

void executeExtremeAction(const ActionType resolved_action) {
  const uint32_t now_ms = millis();
  transitionToState(RuntimeState::ActionRunning, now_ms);

  const bool is_lose = resolved_action == ActionType::Lose;
  const uint8_t speed = is_lose ? kLoseForwardSpeed : kSafeForwardSpeed;
  const uint16_t duration_ms = is_lose ? kLoseForwardDurationMs : kSafeForwardDurationMs;

  const domain_sound::SoundSelection sound_selection =
      domain_sound::chooseGameplaySound(g_app_context.runtime_config, resolved_action, nextSoundRoll());
  const bool sound_started = startSoundPlayback(sound_selection);
  if (sound_started) {
    (void)platform_nano::waitForPlaybackStart(250);
  }

  delay(kActionLeadDelayMs);

  const bool forward_completed = platform_nano::runMotorForward(speed, duration_ms);
  if (forward_completed) {
    runRetraction();
  } else {
    refreshStateFromHardware(millis());
  }

  if (sound_started) {
    platform_nano::waitForPlaybackFinish(kGameplayPlaybackTimeoutMs);
  }

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

  if (g_app_context.runtime_status.state != RuntimeState::IdleReady) {
    return;
  }

  if (g_app_context.runtime_config.game_mode != GameMode::Extreme) {
    return;
  }

  g_app_context.runtime_status.accepted_button_presses++;
  g_app_context.runtime_status.last_button_press_ms = now_ms;

  const ActionType resolved_action =
      (random(100) < kLoseChancePercent) ? ActionType::Lose : ActionType::Safe;
  executeExtremeAction(resolved_action);
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
