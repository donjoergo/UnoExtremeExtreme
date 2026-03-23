# Uno Extreme Extreme — Implementation Plan

## 1. Goal

This document describes the recommended approach for implementing the new
architecture defined in [FSD.md](./FSD.md). The current code base is treated
only as a behavioral and hardware reference. The implementation shall be a
deliberate rewrite with a clean PlatformIO structure, clear module boundaries,
and a stable migration foundation for ESP32.

## 2. Guiding Principles

- The legacy code will not be expanded incrementally. It will be analyzed for
  behavior and then replaced with new modules.
- The current Nano hardware remains the reference platform for V2.
- Work shall be done in small, testable vertical slices.
- Domain logic shall be separated from Arduino-specific code.
- `Extreme` shall be implemented first; `Normal` and `Ascending` shall initially
  remain safe placeholders.
- Safety takes priority over feature richness: open case or unclear state means
  no motor movement.
- Buildability, structure, and testability come before special features.

## 3. Target Rewrite Structure

```text
include/
  config/
  types/
lib/
  app_core/
  domain_game/
  domain_sound/
  domain_motion/
  domain_feedback/
  platform_nano/
  platform_esp32/
src/
  main.cpp
test/
  native/
  embedded/
```

## 4. Implementation Order

### Phase 0 — Freeze Legacy Behavior And Extract The Reference

**Goal:**
Stop adding to the existing code and extract the facts required for the rewrite
in a controlled way.

**Tasks:**
- Document pin mapping, default values, audio folders, and hardware assumptions
  from the current code.
- Inventory the current SD card folder structure and define which legacy pools
  need to be migrated, merged, split, or dropped.
- Decide which legacy behaviors must remain from a functional perspective.
- Decide which legacy structures must explicitly not be carried over.
- Document hardware risks: motor, retraction, busy pin, and case switch.

**Outcome:**
- A technical reference baseline for the rewrite.
- No further architecture work inside the old `functions_*` layout.

**Exit Criteria:**
- It is clearly documented what will be retained from the old code and what
  will be discarded.

### Phase 1 — Build The Project Skeleton And Typed Configuration

**Goal:**
Create a clean, buildable PlatformIO foundation.

**Tasks:**
- Create the new directory structure.
- Introduce `src/main.cpp` as the new entry point.
- Define `GameMode`, `ActionType`, and `SoundCategory` as typed enums.
- Create `RuntimeConfig` with a schema version.
- Introduce `defaults.h` and `pins_nano.h` as focused configuration headers.
- Define `Startup`, `CaseOpened`, `CaseClosed`, and `CaseOpenTooLong` as
  explicit event/action concepts in the type system.
- Stabilize the build for `nanoatmega328`.
- Integrate `pio check` as the static analysis baseline.

**Outcome:**
- The new project skeleton exists.
- The typed configuration model starts replacing the legacy flat structure.

**Exit Criteria:**
- `pio run` passes.
- `pio check` passes, or known warnings are documented.
- The new types and config structures are available centrally in the project.

### Phase 2 — Nano Platform Adapters And Safe Runtime Foundation

**Goal:**
Provide a safe hardware abstraction and a small event/state machine.

**Tasks:**
- Create adapters for button, case switch, LEDs, motor, DFPlayer, and
  persistence.
- Implement debounce for button and case switch.
- Define safe runtime states:
  - `IdleReady`
  - `MaintenanceOpen`
  - `ActionRunning`
  - `WarningOpenTooLong`
  - `PlaceholderMode`
- Ensure the motor can only be driven from approved states.
- Implement the boot sequence with safe idle behavior.

**Outcome:**
- Hardware is integrated behind defined interfaces.
- The system can boot safely and change states predictably.

**Exit Criteria:**
- Open case reliably blocks motor movement.
- Invalid states do not trigger uncontrolled actions.
- Button and case switch do not emit duplicate events due to bounce.

### Phase 3 — Minimal Extreme-Mode Vertical Slice

**Goal:**
Create the first complete but still simple gameplay path.

**Tasks:**
- Evaluate a valid button press.
- Check whether the case is closed.
- Select a basic extreme action.
- Emit startup-ready feedback once after boot if the case is already closed.
- Emit case-close feedback after returning from maintenance mode.
- Play a sound.
- Move the motor forward.
- Execute retraction.
- Reset the LED status correctly.

**Outcome:**
- A small end-to-end path runs on real hardware.

**Exit Criteria:**
- One button press triggers exactly one action.
- The motor only runs in a valid gameplay state.
- Sound, motion, and LEDs work together.

### Phase 4 — Sound System With Categories And Manifest

**Goal:**
Translate the new category model from the FSD into firmware.

**Tasks:**
- Introduce the category system as a bitmask/config model.
- Implement the first categories:
  - `Startup`
  - `BaseFunny`
  - `BaseTts`
  - `CaseEvents`
  - `SpamReactions`
- Define the rebuilt SD folder mapping for event pools and gameplay result
  pools.
- Prepare a manifest for sound sets, gameplay outcome classification, and sound
  metadata.
- Split gameplay-relevant sound pools so that `Safe` and `Lose` can be resolved
  explicitly before weighting and no-repeat logic are applied.
- Support weights/probabilities per category.
- Ensure at least one base category remains active.

**Outcome:**
- Sounds are no longer selected only through hardcoded legacy logic.
- The sound system becomes data-driven.

**Exit Criteria:**
- Categories can be enabled and disabled.
- Weighting affects selection.
- Sound folders, outcome pools, and firmware configuration remain consistent.

### Phase 5 — Anti-Boredom Logic And Creative Runtime

**Goal:**
Implement the entertainment and variation features defined in `specs.md` in a
stable way.

**Tasks:**
- Implement shuffle-bag / no-repeat for sounds.
- Implement shuffle-bag / no-repeat for action and motion pools.
- Implement rapid-press detection with an attenuation model.
- Introduce a motion pattern engine:
  - Split eject
  - Variable speed
  - Stutter
  - Fake fault
- Implement spam reactions when the button is pressed again during an active
  action.
- Calibrate improved retraction behavior on real hardware.

**Outcome:**
- The device becomes more varied and more robust.
- The biggest pain points from the specification are addressed.

**Exit Criteria:**
- No immediate repetitions occur within a pool.
- Repeated fast button presses measurably reduce output intensity.
- Creative motion patterns run in a controlled and reproducible way.

### Phase 6 — Per-Sound Metadata, Persistence, And Nano Service

**Goal:**
Make sound and behavior logic fully configurable.

**Tasks:**
- Introduce per-sound metadata:
  - `valid_actions` with explicit `Safe` / `Lose` eligibility for gameplay
    sounds
  - `wait_before_ms`
  - `wait_after_ms`
  - `volume_override`
  - `motion_pattern_id`
  - optional predecessor/follow-up relations
- Implement EEPROM-based persistence.
- Version the configuration schema.
- Define and implement the Nano service interface:
  - read status
  - read config
  - set mode
  - set category mask
  - set weights
  - set timing
  - save
  - factory reset
- Keep Bluetooth as the primary Nano service interface and keep a serial
  fallback option open.

**Outcome:**
- Configuration can be stored permanently.
- Sound and motion behavior become data-driven and configurable.

**Exit Criteria:**
- Configuration survives reboot.
- Sound metadata measurably affects runtime behavior.
- The service interface can change settings safely.

### Phase 7 — Test Expansion And Release Hardening

**Goal:**
Make the rewrite stable before adding new platforms.

**Tasks:**
- Build host-side tests for:
  - config loading
  - debounce logic
  - shuffle bag
  - rapid-press history
  - migration logic
- Define embedded smoke tests.
- Add content validation for the sound manifest and SD structure.
- Check performance and memory limits on Nano.

**Outcome:**
- The project is not only functional, but robust.

**Exit Criteria:**
- Core logic is covered by automated tests.
- Hardware smoke tests are documented and reproducible.

### Phase 8 — ESP32 Migration And Captive Portal

**Goal:**
Move the domain logic to ESP32 and introduce web-based configuration.

**Tasks:**
- Introduce `platform_esp32`.
- Add a PlatformIO environment for ESP32.
- Implement WiFi AP / hotspot mode.
- Build the captive portal and configuration API.
- Implement config migration from Nano to ESP32.
- Replace `Normal` and `Ascending` placeholders with real modes.

**Outcome:**
- A modern configuration platform built on the same domain logic.

**Exit Criteria:**
- A smartphone can connect and change configuration in the browser.
- Shared settings migrate correctly.
- All three game modes are functional.

## 5. Concrete Next Steps

If implementation starts immediately, the first steps should happen in exactly
this order:

1. Create the new project skeleton without deleting the legacy code yet.
2. Introduce typed config and type definitions.
3. Define the new SD asset layout and the legacy-to-new folder migration map.
4. Create `main.cpp` with a minimal boot sequence and safe idle.
5. Add Nano platform adapters for input, LEDs, motor, and audio.
6. Implement debounce and the maintenance state.
7. Get the first minimal extreme-mode vertical slice, startup-ready sound, and
   case-close feedback running on real hardware.
8. Only then add the category model, safe/lose pool filtering, sound manifest,
   and no-repeat logic.

## 6. Recommended PR Breakdown

### PR 1 — Project Skeleton And Types

**Content:**
- New structure under `src/`, `lib/`, `include/config/`, `include/types/`
- `main.cpp`
- `GameMode`, `ActionType`, `SoundCategory`
- `RuntimeConfig`
- Event typing for startup and case lifecycle hooks

**Goal:**
- Create a buildable foundation.

### PR 2 — Nano Platform And Safe States

**Content:**
- Platform adapters
- Debounce
- Maintenance state
- Safe idle

**Goal:**
- Create a safe runtime foundation.

### PR 3 — Minimal Extreme Mode

**Content:**
- Simple action resolver
- Startup-ready and case-close event feedback
- Sound + motor + retraction + LED

**Goal:**
- First complete gameplay path on hardware.

### PR 4 — Sound Categories And Manifest

**Content:**
- Category model
- Folder mapping
- Safe/lose pool layout
- Sound asset manifest
- Weights

**Goal:**
- Data-driven sound selection.

### PR 5 — No-Repeat And Rapid Press

**Content:**
- Shuffle bag
- Attenuation
- Spam reactions

**Goal:**
- Solve the most important pain points from `specs.md`.

### PR 6 — Motion Patterns And Sound Metadata

**Content:**
- Motion pattern engine
- Special sound metadata
- Outcome classification via `valid_actions`
- Waits, overrides, follow-ups

**Goal:**
- Creative runtime behavior and fine tuning.

### PR 7 — Persistence And Nano Service

**Content:**
- EEPROM
- Service commands
- Settings save / restore

**Goal:**
- Field configurability.

### PR 8 — ESP32 Port And Web UI

**Content:**
- ESP32 adapter
- Hotspot
- Captive portal
- Migration path

**Goal:**
- Build the V3 platform.

## 7. Risks In Early Implementation

- The biggest technical mistake would be adding special features too early
  before the base structure exists.
- The biggest product mistake would be silently continuing to use the current
  code as the architectural template.
- The biggest hardware mistake would be not validating motion and debounce early
  on the real device.
- The biggest migration mistake would be storing configuration without a
  versioned schema.

## 8. Definition Of Done For V2

V2 is only functionally complete when all of the following are true:

- The project follows the new PlatformIO structure.
- `Extreme` is fully implemented.
- `Normal` and `Ascending` exist as safe placeholders.
- Startup-ready and case-close feedback work reliably.
- Sound categories, weights, and no-repeat logic work.
- Gameplay sounds are classified and filtered correctly for `Safe` vs. `Lose`.
- Rapid-press attenuation reduces excessive card output.
- Case-open, case-close, and open-too-long events work.
- Persistence works reliably.
- The Nano version builds reproducibly and is tested on hardware.

## 9. Recommended Start

If implementation begins now, start with **PR 1**: new skeleton, typed config,
`main.cpp`, Nano pin configuration, and an empty but buildable runtime
foundation. Everything else depends on that.
