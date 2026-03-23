# Uno Extreme Extreme — Functional Specification Document (FSD)

## 1. System Overview

Uno Extreme Extreme is a rewrite of a custom UNO Extreme / UNO Attack launcher
controller. The current hardware baseline is an Arduino Nano based device with a
card-eject motor, DFPlayer Mini audio playback, WS2812 button lighting, a front
button, a case-open switch, and optional Bluetooth configuration. This FSD
defines the target behavior and architecture for the rewrite. The current code
base is treated as a compatibility and migration reference only, not as the
architectural template for the new implementation.

The primary V2 goal is a clean, modular firmware rewrite that fully supports the
extreme game mode, introduces richer sound and motion behavior, fixes the
observed pain points, and prepares the project for a later ESP32 migration with
hotspot and captive-portal based configuration. Normal and ascending game modes
shall remain visible in the design as typed placeholders, but they are not a
full V2 delivery target.

**Users / stakeholders:**
- Players using the launcher during gameplay.
- Builder / maintainer flashing firmware and validating the hardware.
- Sound-pack curator organizing SD card content and sound metadata.
- Future operator configuring the device via Bluetooth service or web UI.

**Primary goals:**
- Rewrite the firmware into a maintainable PlatformIO project with clear module
  boundaries, naming conventions, and a testable core.
- Fully support the extreme game mode with creative sound and motion behavior.
- Prevent repetitive output by using no-repeat selection for sounds and actions.
- Reduce excessive card output when the button is pressed repeatedly.
- Support independently switchable sound categories with per-category
  probabilities and per-sound behavior metadata.
- Preserve a structured migration path from Arduino Nano to ESP32.

**Non-goals:**
- V2 does not require sensor-based physical card counting.
- V2 does not require cloud services or internet connectivity.
- V2 does not require full normal and ascending gameplay implementation on the
  Nano rewrite.
- V2 does not require preserving the current legacy file layout or naming.

**High-level system flow:**
- After a successful power-on boot, the device shall emit startup-ready
  feedback once if the case is already closed.
- Case closed and device ready: a debounced button press triggers exactly one
  gameplay resolution in the active game mode.
- Extreme mode resolves a press into a state-machine driven sequence of audio,
  LEDs, timing, and motion segments.
- Case open: gameplay motion is blocked and the device enters a maintenance /
  configuration context; case-open sounds and warnings may play.
- Repeated quick presses influence the next resolved action by reducing motion
  intensity and optionally triggering annoyed feedback.
- V3: the same configuration model is exposed through an ESP32 hotspot and
  captive portal.

## 2. System Architecture

### 2.1 Logical Architecture

The rewritten system shall be divided into the following logical subsystems:

- **Input subsystem:** debounced front-button handling, case-open switch
  handling, press burst tracking, and service input handling.
- **Game engine:** game-mode orchestration, press resolution, placeholder
  handling for non-implemented modes, action selection, and session state.
- **Sound engine:** sound category eligibility, weighted selection,
  shuffle-bag/no-repeat logic, per-sound metadata, and DFPlayer playback
  requests.
- **Motion engine:** ejection, retraction, rapid-press attenuation, and
  multi-segment creative motion patterns.
- **Feedback engine:** LED state rendering, forbidden-color handling, warning
  signals, and synchronization with active actions.
- **Configuration and persistence subsystem:** typed compile-time defaults,
  runtime configuration, schema versioning, persistence, and migration helpers.
- **Platform adapter layer:** Nano hardware drivers in V2 and ESP32 adapters in
  V3 behind stable internal interfaces.

**Primary runtime interaction pattern:**
- Input subsystem produces debounced events such as `ButtonPressed`,
  `CaseOpened`, `CaseClosed`, `CaseOpenTimeout`, and `ServiceCommandReceived`.
- Game engine converts input events into action intents such as `SafeAction`,
  `LoseAction`, `WaitAction`, `SpamReaction`, or `CreativeScript`.
- Sound engine and motion engine resolve the selected action using configuration,
  category weights, no-repeat pools, and per-sound metadata.
- Feedback engine updates LEDs according to the active state and forbids idle
  hue transitions through the configured red and green ranges.
- Persistence subsystem saves configuration changes and exposes a versioned
  schema for future ESP32 migration.

### 2.2 Hardware / Platform Architecture

**V2 baseline platform:**
- Arduino Nano / ATmega328P.
- Existing card launcher motor and mechanics.
- L293D or equivalent motor driver.
- DFPlayer Mini with microSD sound storage.
- WS2812B LEDs in the front button.
- Front gameplay button.
- Case-open switch.
- Optional HC-05 Bluetooth module for local service access.

**V2 platform expectations:**
- The Nano firmware shall remain compatible with the current mechanical launcher.
- The rewrite shall preserve current wiring as far as possible unless migration
  tasks explicitly redefine pin mapping.
- Audio assets remain SD-card based and deterministic in folder layout.
- Persistence on Nano shall use on-device non-volatile storage, preferably
  EEPROM.

**V3 target platform:**
- ESP32 variant with WiFi AP support and sufficient GPIO / UART resources
  (exact module TBD, assumed).
- Hotspot mode and captive portal for browser-based configuration.
- Shared game, sound, and config domain logic reused from the V2 rewrite.
- Optional local storage for web assets and richer metadata handling.

### 2.3 Software Architecture

The rewrite shall use a PlatformIO-oriented architecture with clear separation
between domain logic, hardware adapters, and configuration data.

**Target project structure:**

```text
include/
  config/
    build_config.h
    defaults.h
    pins_nano.h
    pins_esp32.h
  types/
    game_mode.h
    sound_category.h
    action_type.h
    config_schema.h
lib/
  app_core/
    include/
    src/
  domain_game/
    include/
    src/
  domain_sound/
    include/
    src/
  domain_motion/
    include/
    src/
  domain_feedback/
    include/
    src/
  platform_nano/
    include/
    src/
  platform_esp32/
    include/
    src/
src/
  main.cpp
test/
  native/
  embedded/
data/
  web/
docs/
  FSD.md
  specs.md
```

**Naming and design rules:**
- Files and directories shall use lowercase `snake_case`.
- Types, structs, enums, and classes shall use `PascalCase`.
- Functions shall use one consistent project-wide style; the preferred rewrite
  style is `camelCase`.
- Build-time constants shall prefer `constexpr`, `enum class`, and typed config
  structures over preprocessor macros.
- The legacy monolithic `defines.h` shall be split into focused configuration
  headers during migration.
- Domain logic shall not directly include `Arduino.h` unless it is part of a
  platform adapter.

**Boot sequence:**
1. Load compile-time defaults and persisted runtime configuration.
2. Initialize platform adapters for button, switch, LEDs, motor, storage, and
   audio.
3. Validate sound catalog / configuration schema compatibility.
4. Start in safe idle state with maintenance blocked until initialization
   completes.
5. Enter the event loop and process debounced input, timers, sound state, and
   motion state through a cooperative state machine.

**Persistence model:**
- Runtime configuration shall be stored in a versioned config structure.
- Persistent values shall include selected game mode, enabled sound categories,
  category probabilities, volume, timing thresholds, and migration schema
  version.
- No-repeat pool state does not need to persist across reboots unless later
  required.

**Update model:**
- V2: firmware flashing through PlatformIO.
- V3: PlatformIO-based firmware flashing plus ESP32 web asset deployment if
  applicable.
- Sound assets remain deployable through SD card content updates.

## 3. Implementation Phases

### 3.1 Phase 1 — Nano Rewrite Foundation

**Scope:**
- Replace the legacy flat firmware with a new PlatformIO structure.
- Implement a typed config model with `GameMode` placeholders for normal,
  ascending, and extreme.
- Fully implement extreme mode only.
- Add debounced button and case-switch handling.
- Add startup-ready, case-open, case-close, and case-open-too-long sound hooks.
- Add independently switchable sound categories and per-category probabilities.
- Preserve Nano hardware compatibility.

**Deliverables:**
- Buildable Nano firmware with new module structure.
- Typed config headers and versioned runtime config schema.
- Event-driven extreme-mode runtime.
- Service access for local configuration on Nano.
- Initial sound catalog and SD card mapping for the defined categories.

**Exit criteria:**
- `pio run -e nanoatmega328` succeeds.
- The device starts, loads config, and enters safe idle without motor motion.
- If the device boots with the case closed, startup-ready feedback is emitted
  exactly once.
- Closed-case button presses trigger exactly one resolved extreme-mode action.
- Case-open state blocks gameplay motion and exposes maintenance behavior.
- Sound category enable/disable and category weights affect runtime selection.

**Dependencies:**
- Current Nano hardware and pinout documentation.
- Defined SD card sound-pack manifest.
- Decision on Nano service transport using Bluetooth and/or wired serial.

### 3.2 Phase 2 — Creative Runtime And Hardening

**Scope:**
- Implement shuffle-bag/no-repeat logic for sounds and action patterns.
- Implement rapid-press attenuation to reduce excessive card output.
- Add richer motion patterns: split eject with blessing-compatible audio, variable
  speed, stutter, and fake fault.
- Add spam-reaction sounds during active playback or action.
- Add per-sound metadata for wait-before, wait-after, volume override, motion
  pattern binding, and predecessor/follow-up pools.
- Improve retraction tuning and LED feedback rules.

**Deliverables:**
- Weighted no-repeat selector for sound and motion pools.
- Press-history model with configurable attenuation rules.
- Motion pattern engine with multi-segment execution.
- Sound metadata manifest and content validation tooling.
- Stable persistence and settings migration between config schema revisions.

**Exit criteria:**
- No sound or creative action repeats before its eligible pool is exhausted.
- Repeated quick presses measurably reduce average ejected card output.
- Case-open-too-long warning repeats according to configuration.
- Idle LED cycling avoids the forbidden red and green bands.
- The firmware remains stable under burst input and long audio playback.

**Dependencies:**
- Completed Phase 1 architecture.
- Curated sound assets with metadata for creative modes.
- Hardware tuning sessions for retraction and motor behavior.

### 3.3 Phase 3 — ESP32 Migration And Web Configuration

**Scope:**
- Port the platform layer from Nano to ESP32.
- Add hotspot and captive portal settings UI.
- Keep the same domain model and config schema where possible.
- Add full normal and ascending game modes.
- Support migration of saved settings from Nano schema to ESP32 schema.

**Deliverables:**
- ESP32 PlatformIO environment and platform adapter layer.
- Captive portal web UI with versioned config endpoints.
- Full game mode support for normal, ascending, and extreme.
- Migration utilities and documentation.

**Exit criteria:**
- `pio run` succeeds for the selected ESP32 environment.
- A phone can connect to the device hotspot and change settings in the browser.
- Migrated settings are applied correctly on the ESP32 build.
- Normal and ascending modes move from placeholder to functional runtime states.

**Dependencies:**
- ESP32 hardware selection and updated pinout.
- Web UI assets and local storage strategy.
- Stable V2 domain logic and migration schema.

## 4. Functional Requirements

### 4.1 Functional Requirements (FR)

#### Gameplay And Session Control

- **FR-1.1** [Must]: The system shall accept a front-button press as a gameplay
  trigger only after debounce validation and only when the case is closed.
- **FR-1.2** [Must]: The system shall define a typed `GameMode` enumeration with
  at least `Normal`, `Ascending`, and `Extreme`.
- **FR-1.3** [Must]: The V2 Nano rewrite shall fully implement `Extreme` mode and
  shall keep `Normal` and `Ascending` as non-destructive placeholders until
  Phase 3.
- **FR-1.4** [Must]: When the case is open, the system shall block gameplay
  motor output and switch into a maintenance / configuration context.
- **FR-1.5** [Must]: The system shall provide dedicated sound event hooks for
  `Startup`, `CaseOpened`, `CaseClosed`, and `CaseOpenTooLong`.
- **FR-1.6** [Must]: Each accepted gameplay trigger shall resolve through a
  state machine that combines action selection, sound selection, motion
  selection, and LED feedback.
- **FR-1.7** [Should]: The system shall track rapid repeated button presses and
  shall reduce card output intensity for consecutive presses within a
  configurable time window.
- **FR-1.8** [Should]: When the button is pressed again while a sound or action
  is still active, the system shall be able to emit an annoyed / spam-reaction
  feedback instead of starting a full new gameplay action.
- **FR-1.9** [Should]: Extreme mode shall support intentional pause or fake-wait
  sequences before the final outcome is executed.
- **FR-1.10** [May]: The system shall provide a one-button mini-game extension
  framework for future modes such as simplified Simon Says.
- **FR-1.11** [Must]: After a successful boot, if the case is already closed,
  the system shall emit startup-ready feedback exactly once before entering the
  normal ready state.

#### Sound And Content System

- **FR-2.1** [Must]: The system shall support individually enabled and disabled
  sound categories.
- **FR-2.2** [Must]: The base sound foundation shall include at least
  `BaseNormal`, `BaseFunny`, and `BaseTts`.
- **FR-2.3** [Must]: Additional sound categories shall include `Startup`,
  `CantinaShort`, `CantinaLong`, `Fetish`, `Drinking`, `Bdsm`, `CaseEvents`,
  and `SpamReactions`.
- **FR-2.4** [Must]: The SD card layout shall be reorganized into deterministic,
  DFPlayer-compatible folders defined by a versioned sound asset manifest.
- **FR-2.5** [Must]: Each sound category shall have an independent configurable
  probability or weight.
- **FR-2.6** [Must]: The base sound foundation shall be considered during every
  regular gameplay resolution before optional overlay categories are selected.
- **FR-2.7** [Must]: The system shall use random-without-replacement behavior for
  eligible sound items and eligible creative action items so that no item
  repeats until the relevant pool has been exhausted.
- **FR-2.8** [Should]: A sound item shall be able to declare validity for
  multiple action types.
- **FR-2.9** [Must]: A sound item shall support metadata fields for
  `wait_before_ms`, `wait_after_ms`, `volume_override`, and `motion_pattern_id`.
- **FR-2.10** [Should]: A sound item shall be able to declare predecessor or
  follow-up pool constraints for scripted sequences, for example a wait sound
  followed by a specific win or lose pool.
- **FR-2.11** [Must]: The long Cantina mix shall require an explicit long-play
  command or reboot-gated activation and shall not trigger as a normal random
  gameplay sound.
- **FR-2.12** [Must]: Each gameplay-relevant sound item shall declare whether it
  is valid for `Safe`, `Lose`, or both gameplay outcomes.
- **FR-2.13** [Must]: Gameplay sound selection shall first filter the eligible
  sound pool by gameplay outcome (`Safe` or `Lose`) before applying category
  enablement, weighting, metadata constraints, and no-repeat logic.

#### Motion, Mechanical Control, And LED Feedback

- **FR-3.1** [Must]: The system shall eject cards by driving the motor forward
  and shall then perform a separately configurable retraction phase.
- **FR-3.2** [Must]: Retraction timing and retraction speed shall be tunable
  independently from forward motion to improve the last-card pullback behavior.
- **FR-3.3** [Must]: The motion engine shall support multi-segment motion
  patterns with per-segment duration, speed, and optional pause.
- **FR-3.4** [Should]: Creative motion patterns shall include at least split
  eject with blessing-compatible audio, variable-speed eject, stutter eject,
  and fake fault behavior.
- **FR-3.5** [Must]: The rapid-press penalty model shall reduce motion intensity
  for consecutive presses according to configurable thresholds.
- **FR-3.6** [Must]: The motion engine shall refuse gameplay motion while the
  device is in maintenance mode or while the case is open.
- **FR-3.7** [Must]: The idle LED color cycle shall exclude the configured red
  and green hue bands.
- **FR-3.8** [Must]: LED feedback shall distinguish at least ready, active
  action, maintenance, warning, and fault / blocked states.
- **FR-3.9** [Should]: When the case remains open for too long, the system shall
  repeat warning feedback at a configurable interval until the case is closed.

#### Configuration, Persistence, And Platform Migration

- **FR-4.1** [Must]: Compile-time defaults shall be expressed through typed
  configuration headers, including the `GameMode` enum and a sound-category
  bitmask.
- **FR-4.2** [Must]: Runtime settings shall persist across power cycles.
- **FR-4.3** [Should]: The Nano rewrite shall provide local service access for
  configuration over Bluetooth serial or a wired serial fallback.
- **FR-4.4** [Should]: The local service interface shall support changing at
  least game mode, sound-category mask, category probabilities, volume, and
  timing thresholds.
- **FR-4.5** [Must]: The ESP32 revision shall expose a hotspot and captive-portal
  based settings webpage.
- **FR-4.6** [Must]: The runtime configuration schema shall be versioned and
  shared across Nano and ESP32 builds.
- **FR-4.7** [Should]: The project shall provide a settings migration path from
  Nano schema revisions to ESP32 schema revisions.
- **FR-4.8** [Must]: The rewrite shall use a PlatformIO best-practice file
  structure with separated domain modules, platform adapters, and tests.

### 4.2 Non-Functional Requirements (NFR)

- **NFR-1.1** [Must]: The firmware shall build under PlatformIO with at least one
  Nano environment in V2 and one ESP32 environment in V3.
- **NFR-1.2** [Must]: The game, sound, motion, and selection logic shall be
  platform-agnostic and isolated from direct hardware access.
- **NFR-1.3** [Must]: Debounce thresholds for the front button and case-open
  switch shall be configurable and shall prevent duplicate event acceptance
  inside the configured debounce window.
- **NFR-1.4** [Should]: For non-scripted gameplay actions, visible or audible
  primary feedback should begin within 200 ms after a valid button press.
- **NFR-1.5** [Should]: The idle LED implementation should never enter the
  forbidden red or green hue bands, including wrap-around transitions.
- **NFR-1.6** [Must]: Sound folder mapping, sound metadata, and category IDs
  shall be deterministic and versioned.
- **NFR-1.7** [Should]: The persisted settings format should include a schema
  version to support future migrations and default repair.
- **NFR-1.8** [Must]: The Nano implementation shall fit within the available MCU
  flash and SRAM budget with engineering headroom at release time (assumed).
- **NFR-1.9** [Should]: Core selection, press-history, and config-migration logic
  should be covered by host-side automated tests.
- **NFR-1.10** [Must]: The system shall fail safe by keeping the motor disabled
  during initialization errors, invalid maintenance state, and open-case state.
- **NFR-1.11** [Should]: Long waits, warning sounds, and scripted sequences should
  be orchestrated without turning the entire application into a single blocking
  delay chain.

### 4.3 Constraints

- The V2 baseline target is the current Arduino Nano class hardware.
- The current launcher mechanics are retained and no physical card-count sensor
  is assumed.
- Audio output relies on DFPlayer Mini and a deterministic microSD folder layout.
- Nano resources are limited; large metadata structures may need flash /
  `PROGMEM` storage or code generation.
- The V3 platform migration target is ESP32 with local WiFi AP capability.
- The rewrite must preserve the recognizable behavior of the project while
  allowing architectural changes.

## 5. Risks, Assumptions & Dependencies

| Risk | Likelihood | Impact | Mitigation |
|------|------------|--------|------------|
| Nano flash and SRAM may be insufficient for rich sound metadata and creative motion tables | Medium | High | Keep domain model compact, use generated tables in flash, move heavy metadata handling to ESP32 where needed |
| Rapid-press attenuation may still not correlate perfectly with actual card count because there is no sensor feedback | High | Medium | Calibrate against real hardware, expose tunable thresholds, document mechanical variance |
| DFPlayer playback latency may make tightly synchronized motion feel inconsistent | Medium | Medium | Treat sound and motion as loosely synchronized scripts with bounded timing assumptions |
| Long Cantina playback can interfere with normal gameplay if not isolated | Medium | Medium | Require explicit activation and define blocking behavior clearly |
| SD card content may diverge from the expected folder manifest | Medium | High | Version the manifest and add a validation / provisioning checklist |
| ESP32 migration may drift from the Nano config schema | Medium | High | Version the config schema and implement migration tests before web UI rollout |
| Rewriting the project may accidentally preserve legacy coupling through copy-paste instead of modular extraction | Medium | High | Treat current code as behavioral reference only and enforce the new structure from the first commit |

**Assumptions:**
- The case-open switch remains available on future hardware because it is part of
  the intended user interaction model `(assumed)`.
- Bluetooth serial remains acceptable as a local Nano service interface until the
  ESP32 web UI is available `(assumed)`.
- At least one base sound category must always remain enabled to avoid silent
  standard gameplay `(assumed)`.
- The long Cantina mix is a deliberate special mode and not part of the standard
  random gameplay pool `(assumed)`.

**External dependencies:**
- Current launcher mechanics and motor assembly.
- DFPlayer Mini compatible audio module and correctly prepared SD card.
- PlatformIO toolchain and supported libraries.
- Optional Bluetooth module on Nano builds.
- Future ESP32 hardware selection and WiFi support.

## 6. Interface Specifications

### 6.1 External Interfaces

| Interface | Direction | Medium | Purpose |
|-----------|-----------|--------|---------|
| Front button | Input to device | Digital GPIO | Main gameplay trigger |
| Case-open switch | Input to device | Digital GPIO | Maintenance gating and case-event detection |
| Motor driver | Output from device | GPIO + PWM | Card ejection and retraction |
| DFPlayer Mini | Bidirectional control | UART | Trigger sound playback and volume control |
| DFPlayer microSD | Physical media | FAT-formatted SD | Stores deterministic sound folders |
| WS2812 LEDs | Output from device | One-wire digital | Visual state and warning feedback |
| Nano local service | Bidirectional | Bluetooth serial and/or wired serial | Local settings and service commands |
| ESP32 captive portal | Bidirectional | WiFi AP + HTTP | Browser-based settings UI in V3 |

### 6.2 Internal Interfaces

| Producer | Consumer | Interface | Purpose |
|----------|----------|-----------|---------|
| Input subsystem | Game engine | Debounced event queue | Transfers validated button and case events |
| Game engine | Sound engine | `ActionRequest` | Requests sound resolution for an action |
| Game engine | Motion engine | `ActionRequest` | Requests motion resolution for an action |
| Sound engine | Feedback engine | Playback state events | LED synchronization and busy indication |
| Config subsystem | All subsystems | `RuntimeConfig` snapshot | Supplies typed settings and thresholds |
| Persistence subsystem | Config subsystem | Versioned config blob | Stores and restores settings |
| Platform adapters | Domain services | Hardware abstraction interfaces | Isolates Arduino / ESP32 specifics |

### 6.3 Data Models / Schemas

**Core enums:**

| Type | Values | Notes |
|------|--------|-------|
| `GameMode` | `Normal`, `Ascending`, `Extreme` | Only `Extreme` is fully implemented in V2 |
| `ActionType` | `Startup`, `Safe`, `Lose`, `Wait`, `SpamReaction`, `CaseOpened`, `CaseClosed`, `CaseOpenTooLong`, `CreativeScript`, `MiniGame` | Used by sound and motion eligibility |
| `SoundCategory` | `BaseNormal`, `BaseFunny`, `BaseTts`, `Startup`, `CantinaShort`, `CantinaLong`, `Fetish`, `Drinking`, `Bdsm`, `CaseEvents`, `SpamReactions` | Represented internally as a bitmask |

**Runtime configuration schema:**

| Field | Type | Description |
|-------|------|-------------|
| `schema_version` | `uint16_t` | Version of persisted config format |
| `game_mode` | `GameMode` | Active mode selection |
| `enabled_categories` | bitmask | Enabled sound categories |
| `category_weights[]` | integer array | Per-category weights / probabilities |
| `master_volume` | `uint8_t` | Global playback volume |
| `button_debounce_ms` | `uint16_t` | Front-button debounce threshold |
| `case_debounce_ms` | `uint16_t` | Case-switch debounce threshold |
| `rapid_press_window_ms` | `uint16_t` | Time window for press burst detection |
| `rapid_press_penalty_steps[]` | config array | Penalty curve for repeated presses |
| `open_warning_delay_ms` | `uint32_t` | Delay until first open-too-long warning |
| `open_warning_repeat_ms` | `uint32_t` | Repeat interval for open-too-long warnings |
| `retraction_speed` | `uint8_t` | Reverse motion speed |
| `retraction_duration_ms` | `uint16_t` | Reverse motion duration |
| `forbidden_hue_ranges[]` | config array | LED hue bands to skip in idle mode |

**Sound item metadata schema:**

| Field | Type | Description |
|-------|------|-------------|
| `sound_id` | integer | Logical ID inside the manifest |
| `folder_id` | integer | SD folder / sound set mapping |
| `file_index` | integer | File index inside the folder |
| `category` | `SoundCategory` | Category membership |
| `valid_actions` | bitmask | Eligible `ActionType` values; gameplay-relevant sounds must include `Safe`, `Lose`, or both |
| `weight` | integer | Selection weight inside the eligible pool |
| `wait_before_ms` | integer | Wait before playback or motion |
| `wait_after_ms` | integer | Wait after playback or motion |
| `volume_override` | integer / sentinel | Optional per-sound volume |
| `motion_pattern_id` | integer / sentinel | Optional bound motion pattern |
| `predecessor_group` | integer / sentinel | Optional required predecessor group |
| `follow_up_group` | integer / sentinel | Optional follow-up pool selector |

**Motion pattern schema:**

| Field | Type | Description |
|-------|------|-------------|
| `pattern_id` | integer | Unique pattern identifier |
| `segment_count` | integer | Number of motion segments |
| `segments[]` | list | Ordered segments |

**Motion segment schema:**

| Field | Type | Description |
|-------|------|-------------|
| `direction` | enum | `Forward`, `Reverse`, `Stop` |
| `speed` | integer | PWM or abstract speed value |
| `duration_ms` | integer | Segment duration |
| `pause_after_ms` | integer | Optional pause after segment |

### 6.4 Commands / Opcodes

The exact Nano on-wire syntax may change during implementation, but the rewrite
shall support the following service operations as the canonical contract.

| Operation | Nano service intent | ESP32 web equivalent | Purpose |
|-----------|---------------------|----------------------|---------|
| Status query | `status()` | `GET /api/v1/status` | Read health, active mode, schema version |
| Read config | `get_config()` | `GET /api/v1/config` | Read current settings |
| Set mode | `set_mode(mode)` | `PUT /api/v1/config/mode` | Change `GameMode` |
| Set sound mask | `set_categories(mask)` | `PUT /api/v1/config/categories` | Enable / disable categories |
| Set category weight | `set_weight(id,value)` | `PUT /api/v1/config/categories/{id}` | Adjust category probability |
| Set volume | `set_volume(value)` | `PUT /api/v1/config/audio` | Adjust master volume |
| Set timing | `set_timing(key,value)` | `PUT /api/v1/config/timing` | Adjust debounce, warning, and penalty timing |
| Save config | `save()` | `POST /api/v1/config/save` | Persist current settings |
| Factory reset | `factory_reset(confirm)` | `POST /api/v1/config/factory-reset` | Restore defaults |
| Long Cantina activation | `play_long_mix(confirm)` | `POST /api/v1/actions/play-long-mix` | Explicitly start protected long playback |
| Reboot | `reboot()` | `POST /api/v1/actions/reboot` | Restart the device if required |

## 7. Operational Procedures

### 7.1 Build And Flash Procedure

1. Select the intended PlatformIO environment.
2. Build the firmware with `pio run`.
3. Flash the firmware with `pio run -t upload`.
4. Verify boot logs or service readiness on serial output if enabled.
5. Confirm that the motor stays disabled until initialization completes.

### 7.2 Sound-Pack Provisioning Procedure

1. Prepare the SD card with the documented folder IDs and numbered files.
2. Validate that every enabled category in the config has matching folder
   content.
3. Validate that special sound metadata references only existing folder/file
   pairs.
4. Insert the SD card and power-cycle the device.
5. Run a content validation check or provisioning checklist before gameplay.

### 7.3 Nano Configuration Procedure

1. Open the case or otherwise enter maintenance mode.
2. Connect through the Nano local service interface.
3. Read the current configuration.
4. Adjust category mask, weights, volume, and timing parameters.
5. Save the configuration.
6. Close the case and verify case-close feedback.

### 7.4 Normal Gameplay Procedure

1. Ensure the case is closed and the device indicates ready state.
2. Press the front button once.
3. Wait for the device to resolve the action sequence.
4. If the button is spammed, expect attenuated motion or spam feedback instead
   of repeated full-strength ejection.

### 7.5 Maintenance And Recovery Procedure

1. If the device behaves unexpectedly, open the case and enter maintenance mode.
2. Query current config and schema version.
3. If content mismatch is suspected, re-validate the SD card manifest.
4. If config corruption is suspected, execute a factory reset and re-provision
   settings.
5. If the firmware no longer behaves correctly after an update, reflash the last
   known-good build.

### 7.6 ESP32 Migration Procedure

1. Freeze the Nano config schema revision used for migration.
2. Port hardware-specific adapters to the ESP32 platform layer.
3. Deploy the same domain logic and config schema on the ESP32 build.
4. Add the hotspot and captive portal UI.
5. Import or translate Nano settings into the ESP32 schema.
6. Run migration acceptance tests before enabling non-placeholder game modes.

## 8. Verification & Validation

### 8.1 Phase 1 Verification

| Test ID | Feature | Procedure | Success Criteria |
|---------|---------|-----------|-----------------|
| TC-1.1 | PlatformIO project skeleton | Build the Nano environment from the rewritten file structure | Build succeeds and module boundaries compile cleanly |
| TC-1.2 | Typed config and placeholders | Validate `GameMode`, category bitmask, and versioned config loading | Config loads defaults correctly and placeholders do not move the motor |
| TC-1.3 | Debounced input handling | Trigger repeated fast button and case-switch transitions | Exactly one accepted event is emitted per valid debounce window |
| TC-1.4 | Maintenance gating | Open the case and press the button | Motor motion is blocked and maintenance behavior becomes active |
| TC-1.5 | Extreme mode base flow | Trigger repeated normal gameplay presses with case closed | Extreme mode resolves actions with sound, LED, and motion feedback |
| TC-1.6 | Category enable/disable and weights | Toggle categories and vary their weights | Only enabled categories are eligible and weighting changes selection frequency |
| TC-1.7 | No-repeat selection | Exhaust a defined test pool of sounds and creative actions | No eligible item repeats before its pool is exhausted |
| TC-1.8 | LED forbidden hue handling | Run idle LED cycling for multiple full cycles | Idle LEDs never traverse forbidden red or green bands |
| TC-1.9 | Persistence | Save modified settings, power-cycle, and reload | Settings survive reboot and schema version is preserved |
| TC-1.10 | Startup and case-close feedback | Boot with the case closed, then open and close the case during runtime | Startup-ready feedback plays exactly once after boot and case-close feedback plays after maintenance return |

### 8.2 Phase 2 Verification

| Test ID | Feature | Procedure | Success Criteria |
|---------|---------|-----------|-----------------|
| TC-2.1 | Rapid-press attenuation | Press the button repeatedly inside the rapid-press window | Forward motion intensity is reduced according to the configured penalty curve |
| TC-2.2 | Per-sound metadata | Execute sounds with wait, volume override, and motion binding | Metadata is applied consistently to the resulting action sequence |
| TC-2.3 | Creative motion patterns | Run split eject with blessing-compatible audio, variable speed, stutter, and fake fault patterns | Each pattern executes the expected segment sequence |
| TC-2.4 | Spam reaction behavior | Press the button during active playback or motion | The configured annoyed feedback is emitted without starting an unsafe extra ejection |
| TC-2.5 | Long Cantina protection | Attempt normal play and explicit long-mix activation | Long mix never triggers randomly and only starts after explicit protected activation |
| TC-2.6 | Case-open-too-long warning | Keep the case open beyond the warning delay | Warning feedback starts after the delay and repeats at the configured interval |
| TC-2.7 | Placeholder mode safety | Select `Normal` and `Ascending` in V2 Nano firmware | The system stays safe, non-destructive, and clearly indicates placeholder status |
| TC-2.8 | Naming and structure compliance | Review rewritten project paths, modules, and include usage | Project follows the defined PlatformIO structure and naming rules |
| TC-2.9 | Host-side logic tests | Run automated tests for selection, press history, and migration logic | Host-side tests pass and cover the core platform-agnostic logic |
| TC-2.10 | Outcome-based sound pool filtering | Trigger both `Safe` and `Lose` gameplay results across multiple categories | Only sounds valid for the current gameplay outcome are eligible before weighting and no-repeat are applied |

### 8.3 Acceptance Tests

| Test ID | Scenario | Procedure | Success Criteria |
|---------|----------|-----------|-----------------|
| AT-1 | Full Nano gameplay session | Run a representative party session with enabled base, drinking, and BDSM categories | Device remains stable, varied, and free of immediate repetition |
| AT-2 | Persistence and content integrity | Change settings, power-cycle, and validate SD manifest against config | Settings persist and invalid content references are detected |
| AT-3 | Nano-to-ESP32 config migration | Export Nano config, import into ESP32 schema, and compare values | Shared settings map correctly and unsupported fields are handled deterministically |
| AT-4 | ESP32 captive portal | Connect a phone to the hotspot and change runtime settings in the browser | Settings update successfully and are applied on-device |
| AT-5 | Non-placeholder mode completion | Validate normal and ascending mode on the Phase 3 build | Both modes operate beyond placeholder state and remain compatible with shared config |

### 8.4 Traceability Matrix

| Requirement | Priority | Test Case(s) | Status |
|-------------|----------|--------------|--------|
| FR-1.1 | Must | TC-1.3, TC-1.5 | Covered |
| FR-1.2 | Must | TC-1.2 | Covered |
| FR-1.3 | Must | TC-1.2, TC-2.7, AT-5 | Covered |
| FR-1.4 | Must | TC-1.4 | Covered |
| FR-1.5 | Must | TC-1.10, TC-2.6 | Covered |
| FR-1.6 | Must | TC-1.5, TC-2.2 | Covered |
| FR-1.7 | Should | TC-2.1 | Covered |
| FR-1.8 | Should | TC-2.4 | Covered |
| FR-1.9 | Should | TC-2.2, TC-2.3 | Covered |
| FR-1.10 | May | --- | Planned |
| FR-1.11 | Must | TC-1.10 | Covered |
| FR-2.1 | Must | TC-1.6 | Covered |
| FR-2.2 | Must | TC-1.6, AT-1 | Covered |
| FR-2.3 | Must | TC-1.6, AT-1 | Covered |
| FR-2.4 | Must | TC-1.6, AT-2 | Covered |
| FR-2.5 | Must | TC-1.6 | Covered |
| FR-2.6 | Must | TC-1.5, AT-1 | Covered |
| FR-2.7 | Must | TC-1.7, AT-1 | Covered |
| FR-2.8 | Should | TC-2.2 | Covered |
| FR-2.9 | Must | TC-2.2 | Covered |
| FR-2.10 | Should | TC-2.2 | Covered |
| FR-2.11 | Must | TC-2.5 | Covered |
| FR-2.12 | Must | TC-2.10 | Covered |
| FR-2.13 | Must | TC-2.10 | Covered |
| FR-3.1 | Must | TC-1.5, TC-2.3 | Covered |
| FR-3.2 | Must | TC-2.3 | Covered |
| FR-3.3 | Must | TC-2.3 | Covered |
| FR-3.4 | Should | TC-2.3 | Covered |
| FR-3.5 | Must | TC-2.1 | Covered |
| FR-3.6 | Must | TC-1.4 | Covered |
| FR-3.7 | Must | TC-1.8 | Covered |
| FR-3.8 | Must | TC-1.4, TC-1.5, TC-2.6 | Covered |
| FR-3.9 | Should | TC-2.6 | Covered |
| FR-4.1 | Must | TC-1.2 | Covered |
| FR-4.2 | Must | TC-1.9, AT-2 | Covered |
| FR-4.3 | Should | TC-1.4 | Covered |
| FR-4.4 | Should | TC-1.9, TC-2.8 | Covered |
| FR-4.5 | Must | AT-4 | Covered |
| FR-4.6 | Must | TC-1.2, AT-3 | Covered |
| FR-4.7 | Should | AT-3 | Covered |
| FR-4.8 | Must | TC-1.1, TC-2.8 | Covered |
| NFR-1.1 | Must | TC-1.1, AT-4 | Covered |
| NFR-1.2 | Must | TC-2.9, TC-2.8 | Covered |
| NFR-1.3 | Must | TC-1.3 | Covered |
| NFR-1.4 | Should | TC-1.5 | Covered |
| NFR-1.5 | Should | TC-1.8 | Covered |
| NFR-1.6 | Must | AT-2 | Covered |
| NFR-1.7 | Should | TC-1.9, AT-3 | Covered |
| NFR-1.8 | Must | TC-1.1 | Covered |
| NFR-1.9 | Should | TC-2.9 | Covered |
| NFR-1.10 | Must | TC-1.4, TC-1.5 | Covered |
| NFR-1.11 | Should | TC-2.2, TC-2.6 | Covered |

## 9. Troubleshooting Guide

| Symptom | Likely Cause | Diagnostic Steps | Corrective Action |
|---------|--------------|-----------------|-------------------|
| Cards eject too aggressively after repeated presses | Rapid-press penalty disabled or mis-tuned | Inspect timing config and press history thresholds | Lower penalty thresholds and retune motion profiles |
| The same sound repeats too often | Shuffle-bag pool too small or metadata invalid | Validate category size and pool reset logic | Add more assets or fix pool eligibility |
| A safe sound plays on a lose result or vice versa | Outcome classification in the manifest is wrong or ignored | Inspect `valid_actions` and result filtering for the affected item | Correct the manifest classification and verify pool filtering order |
| No sound plays for a category | SD folder missing or category disabled | Validate manifest, folder IDs, and config mask | Re-provision SD card or re-enable the category |
| Idle LED passes through red or green | Forbidden hue ranges missing or wrap-around handling incorrect | Inspect LED config and idle cycle implementation | Correct forbidden hue logic and retest |
| Device still ejects while the case is open | Case-switch debounce or state gating broken | Inspect raw case-switch state and maintenance state transitions | Fix switch handling and verify safe-state gating |
| Settings disappear after reboot | EEPROM write failed or schema mismatch | Query schema version and verify save path | Repair persistence logic or reset to defaults |
| Long Cantina starts unexpectedly | Wrong pool tagging or protected activation bypassed | Review long-mix eligibility and service commands | Remove the item from random pools and enforce protection |
| ESP32 import misreads Nano settings | Migration rules incomplete | Compare source and target schema versions | Extend migration map and rerun migration tests |

## 10. Appendix

### 10.1 Proposed SD Card Layout And Pool Mapping

The SD card layout shall be rebuilt alongside the firmware rewrite. Physical
folders should remain simple and DFPlayer-compatible, while the manifest carries
the higher-level semantics such as category, valid outcome, metadata, and
follow-up rules. For V2, gameplay pools should be split physically by outcome
where that improves clarity and reduces ambiguity.

| Folder ID | Pool / Folder Name | Intended content |
|-----------|--------------------|------------------|
| 01 | `Startup` | Startup-ready clips played once after a successful boot with the case closed |
| 02 | `CaseEvents` | Open, close, and open-too-long clips |
| 03 | `Wait` | Wait and fake-delay clips |
| 04 | `Safe_BaseNormal` | Classic gameplay sounds for safe outcomes |
| 05 | `Lose_BaseNormal` | Classic gameplay sounds for lose outcomes |
| 06 | `Safe_BaseFunny` | Funny gameplay sounds for safe outcomes |
| 07 | `Lose_BaseFunny` | Funny gameplay sounds for lose outcomes |
| 08 | `Safe_BaseTts` | Spoken / robot gameplay sounds for safe outcomes |
| 09 | `Lose_BaseTts` | Spoken / robot gameplay sounds for lose outcomes |
| 10 | `Safe_Drinking` | Drinking prompts for safe outcomes |
| 11 | `Lose_Drinking` | Drinking prompts for lose outcomes |
| 12 | `Safe_Bdsm` | Dominant / rude prompts for safe outcomes |
| 13 | `Lose_Bdsm` | Dominant / rude prompts for lose outcomes |
| 14 | `Safe_Fetish` | Fetish prompts for safe outcomes |
| 15 | `Lose_Fetish` | Fetish prompts for lose outcomes |
| 16 | `SpamReactions` | Annoyed responses to repeated presses |
| 17 | `CantinaShort` | Short Cantina / blessing style clips, typically in the short-clip or ~1-minute range |
| 18 | `CantinaLong` | Long protected Cantina mix, for example a 30-minute session track |

### 10.2 Rewrite Naming Conventions

| Item | Convention |
|------|------------|
| Directories | lowercase `snake_case` |
| Source / header files | lowercase `snake_case` |
| Enums / structs / classes | `PascalCase` |
| Functions | `camelCase` |
| Compile-time constants | `constexpr kCamelCase` or scoped enum values |
| Macros | Header guards only where possible |
| Config files | grouped under `include/config/` |
| Platform code | isolated under `lib/platform_*` |

### 10.3 Migration Principles

- Reuse the current code base only to extract behavior, constants, and hardware
  assumptions.
- Do not preserve the current flat function-oriented module layout.
- Introduce stable interfaces before porting feature logic.
- Migrate one behavior area at a time: input, config, sound selection, motion,
  LEDs, then platform services.
- Keep placeholder modes explicit instead of partially reusing legacy logic.
- Validate every migrated behavior on hardware before moving to the next layer.

### 10.4 Recommended Nano Rewrite Defaults

| Setting | Recommended initial value |
|---------|---------------------------|
| `game_mode` | `Extreme` |
| `enabled_categories` | `Startup`, `BaseFunny`, `BaseTts`, `CaseEvents`, `SpamReactions` |
| `rapid_press_window_ms` | 1500 |
| `button_debounce_ms` | 40 |
| `case_debounce_ms` | 60 |
| `open_warning_delay_ms` | 10000 |
| `open_warning_repeat_ms` | 15000 |
| `retraction_duration_ms` | To be calibrated on hardware |
| `forbidden_hue_ranges` | Red and green bands defined during LED tuning |

## 11. Implementation Task Backlog

| Task ID | Phase | Task | Output |
|---------|-------|------|--------|
| T-01 | 1 | Freeze the legacy behavior that must remain recognizable and explicitly drop behaviors that are out of scope for the rewrite | Migration baseline note |
| T-02 | 1 | Create the new PlatformIO directory structure with `src`, `lib`, `include/config`, and `test` layout | New project skeleton |
| T-03 | 1 | Split legacy `defines.h` into typed config headers and introduce `GameMode`, `ActionType`, and `SoundCategory` enums | Config and type layer |
| T-04 | 1 | Define the versioned `RuntimeConfig` schema and EEPROM storage format | Persisted config model |
| T-05 | 1 | Implement debounced button and case-switch input services with event emission | Input subsystem |
| T-06 | 1 | Implement the maintenance / configuration state and safe motor gating | State-machine foundation |
| T-07 | 1 | Implement the extreme-mode action resolver as platform-agnostic domain logic | Game engine core |
| T-08 | 1 | Define the new SD card asset layout, the legacy-to-new folder migration map, and the manifest schema for categories, outcomes, and metadata | Content baseline |
| T-09 | 1 | Implement category enable/disable, category weights, and base-foundation selection rules | Sound selection core |
| T-10 | 1 | Implement startup-ready, case-open, case-close, and case-open-too-long sound hooks | Lifecycle and case event behavior |
| T-11 | 1 | Implement LED ready / maintenance / active / warning state rendering with forbidden hue handling | Feedback engine |
| T-12 | 1 | Implement Nano local service commands for reading, changing, and saving config | Nano service layer |
| T-13 | 1 | Add host-side tests for config loading, input debounce, placeholder mode safety, and establish a `pio check` static-analysis baseline | Automated Phase 1 tests and analysis |
| T-14 | 2 | Implement shuffle-bag / no-repeat pool logic for sounds and creative actions | Anti-boredom selector |
| T-15 | 2 | Implement rapid-press history tracking and motion attenuation | Card-count mitigation |
| T-16 | 2 | Implement motion pattern engine with split eject plus blessing-compatible audio, variable speed, stutter, and fake fault scripts | Creative motion runtime |
| T-17 | 2 | Introduce sound metadata for waits, volume overrides, motion pattern bindings, outcome classification, and follow-up pools | Rich sound manifest |
| T-18 | 2 | Implement spam-reaction sounds when input occurs during active playback / action | Spam feedback |
| T-19 | 2 | Calibrate improved retraction behavior on real hardware and store tuned defaults | Mechanical tuning |
| T-20 | 2 | Add content validation tooling or checklist to detect missing folders and invalid metadata links | Content QA |
| T-21 | 2 | Expand automated tests for no-repeat logic, rapid-press penalty, and migration helpers | Automated Phase 2 tests |
| T-22 | 3 | Add an ESP32 PlatformIO environment and port the platform adapter layer | ESP32 baseline |
| T-23 | 3 | Implement hotspot provisioning, captive portal, and config API endpoints | Web configuration layer |
| T-24 | 3 | Add config migration from Nano schema to ESP32 schema with import / export tests | Migration path |
| T-25 | 3 | Replace placeholder normal and ascending modes with functional implementations | Full game mode support |
| T-26 | 3 | Run end-to-end hardware acceptance tests on Nano and ESP32 builds | Release validation |
