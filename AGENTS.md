# Repository Guidelines

## Project Structure & Module Organization
Firmware sources live in `src/` and shared headers in `include/` (default PlatformIO layout).  
Main control flow is in `src/UNO_ExtremeExtreme.cpp` (`setup()` / `loop()`), while domain logic is split into modules such as `functions_general.cpp`, `functions_motor.cpp`, `function_sounds.cpp`, and `functions_bluetooth.cpp`.  
Shared interfaces and globals are organized via `include/functions.h`, `include/variables.h`, `include/defines.h`, and `src/globals.cpp`.

Non-firmware artifacts are kept separately:
- `App-Files/` for mobile app assets (`.aia`, `.apk`)
- `Images/` for documentation/media
- `3D-Files/` for printable/mechanical parts

## Build, Test, and Development Commands
Use PlatformIO from repository root:

```bash
pio run                      # Compile firmware for nanoatmega328
pio run -t upload            # Flash firmware to connected Arduino Nano
pio device monitor -b 9600   # Open serial monitor
pio check                    # Run static analysis (cppcheck)
```

If needed, target the explicit environment with `-e nanoatmega328`.

## Coding Style & Naming Conventions
Use Arduino C++ conventions already present in the codebase:
- 2-space indentation; opening brace on the same line.
- Keep constants/macros in `defines.h` as `UPPER_SNAKE_CASE` (for example, `SOUND_BT_SETTINGS_ACTIVATED`).
- Use descriptive function names consistent with existing style (for example, `PlaySound`, `SetPinModes`, `CheckBT`).
- Add new declarations to headers and keep global definitions centralized in `globals.cpp`.
- Prefer short, hardware-relevant comments over long narrative blocks.

## Testing Guidelines
There is currently no automated unit-test suite. Minimum validation before a PR:
1. `pio run`
2. `pio check`
3. Hardware smoke test on Arduino Nano (button interrupt, motor behavior, sound playback, Bluetooth command handling).

Document the tested board setup and any wiring assumptions when behavior depends on hardware.

## Commit & Pull Request Guidelines
Recent history uses Conventional Commit style; follow `type(scope): summary` where possible (for example, `refactor(core): split functions and globals into dedicated modules`).

For pull requests:
1. Keep changes focused on one concern.
2. Explain behavioral impact and touched modules.
3. Link related issue/TODO item.
4. Include manual test evidence (serial output snippet, plus photo/video for LED/motor behavior changes).
