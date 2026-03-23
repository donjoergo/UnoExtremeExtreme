# Legacy Nano Baseline

This directory contains the archived pre-V2 Nano firmware source tree exactly as
it was used before the rewrite foundation became the active root build.

- Legacy source files live under `legacy/nano_v0/src/`.
- Legacy headers live under `legacy/nano_v0/include/`.
- The copied `legacy/nano_v0/platformio.ini` preserves the old build context as
  a reference.

The root PlatformIO build now targets the V2 rewrite foundation instead of this
legacy layout.
