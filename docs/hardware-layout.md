# Arduino Nano Hardware Layout

This layout is derived from the hardware interfaces in `specs.md` and the pin constants in `include/defines.h`.
Where comments and runtime behavior differ, the mapping follows the active code in `src/UNO_ExtremeExtreme.cpp`, `src/functions_general.cpp`, and `src/globals.cpp`.

## ASCII Nano Pinout

Top view, Mini-USB at the top, matching the classic Arduino Nano header layout.
The left/right header placement is cross-checked against Arduino's official Nano pinout.

```text
     Front / Left Header                                  Arduino Nano          Front / Right Header

                                                     +-------------------+
  unused                               D13   <------ | o               o | ------> D12  -> L293D IN1
  unused                               3V3   <------ | o               o | ------> D11  -> L293D EN / PWM
  unused                              AREF   <------ | o               o | <------ D10  <- HC-05 TX
  optional floating / no component      A0   <------ | o  ATmega328P   o | ------> D9   -> L293D IN2
  unused                                A1   <------ | o               o | ------> D8   -> HC-05 RX
  unused                                A2   <------ | o               o | <------ D7   <- DFPlayer BUSY
  unused                                A3   <------ | o               o | ------> D6   -> DFPlayer RX
  unused                                A4   <------ | o               o | <------ D5   <- DFPlayer TX
  unused                                A5   <------ | o               o | ------> D4   -> WS2812B DIN
  unused                                A6   <------ | o               o | <------ D3   <- Safety switch
  unused                                A7   <------ | o               o | <------ D2   <- Front button
  +5V rail to DFPlayer / HC-05 / LEDs   5V   <------ | o               o | ------> GND  -> switch return / common GND
  unused                              RESET  <------ | o               o | ------> RESET unused
  common GND                           GND   <------ | o               o | ------> D0/RX unused
  VIN from external input if used      VIN   <------ | o___ Mini-USB __o | ------> D1/TX unused
                                                     +-------------------+

  Connected modules

  Front button:
    D2 -> button signal
    GND -> other side of button

  Case safety switch:
    D3 -> switch signal
    GND -> switch closes to GND in the "case closed" state

  WS2812B LEDs (2x under front button):
    D4 -> DIN
    5V -> VCC
    GND -> GND

  DFPlayer Mini:
    D5  <- DFPlayer TX
    D6  -> DFPlayer RX
    D7  <- DFPlayer BUSY
    5V  -> VCC
    GND -> GND
    SPK+ / SPK- -> speaker

  HC-05 Bluetooth:
    D8  -> HC-05 RX
    D10 <- HC-05 TX
    5V  -> VCC
    GND -> GND

  L293D motor driver:
    D12 -> IN1
    D9  -> IN2
    D11 -> EN
    5V  -> Vcc1 (logic) *
    GND -> GND
    Motor PSU -> Vs *
    OUT1 / OUT2 -> DC motor

  * inferred from the driver role in the FSD and firmware; motor supply voltage is not specified
```

## Pin Map

| Nano pin | Define | Connects to | Notes |
|----------|--------|-------------|-------|
| D2 | `Button_PIN` | Front button | Configured as `INPUT_PULLUP`; button is assumed to short to GND when pressed |
| D3 | `Safety_PIN` | Case safety switch | Configured as `INPUT_PULLUP`; runtime logic uses `HIGH = case open`, `LOW = case closed` |
| D4 | `RGB_DATA_PIN` | WS2812B LED chain data input | Drives both front-button LEDs |
| D5 | `DF_RX_PIN` | DFPlayer TX | `SoftwareSerial(rxPin, txPin)` makes this the Nano receive pin |
| D6 | `DF_TX_PIN` | DFPlayer RX | Nano transmit pin for DFPlayer control |
| D7 | `Busy_PIN` | DFPlayer BUSY | Playback status input, active low while playing |
| D8 | `BT_TX_PIN` | HC-05 RX | Nano transmit pin for Bluetooth settings traffic |
| D9 | `IN2_PIN` | L293D IN2 | Motor direction control |
| D10 | `BT_RX_PIN` | HC-05 TX | `SoftwareSerial(rxPin, txPin)` makes this the Nano receive pin |
| D11 | `EN_PIN` | L293D EN | PWM speed control for motor driver |
| D12 | `IN1_PIN` | L293D IN1 | Motor direction control |
| 5V | n/a | DFPlayer, HC-05, WS2812B, L293D logic | Power rail is not fully specified in the firmware sources |
| GND | n/a | All modules and switches | Common ground is required, including the motor supply ground |

## Wiring Notes

- The button and safety switch wiring is inferred from `SetPinModes()` using `INPUT_PULLUP`; neither component should need an external pull-up resistor in the current design.
- The `Safety_PIN` comment in `include/defines.h` says "Case is open when 0", but the runtime logic in `src/UNO_ExtremeExtreme.cpp` behaves as `HIGH = case open`. This diagram follows the runtime behavior.
- The DFPlayer and HC-05 serial directions are based on the `SoftwareSerial(rxPin, txPin)` constructors in `src/globals.cpp`, not only on the macro names.
- The L293D output pins to the motor and the exact motor supply voltage are not defined in firmware, but the driver requires a separate motor supply rail and a shared ground with the Nano.
