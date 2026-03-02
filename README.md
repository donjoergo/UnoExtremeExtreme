![UNO](https://github.com/donjoergo/UnoExtremeExtreme/blob/master/Images/UNO_Extreme.jpg "UNO ExtremeExtreme")

### This project turns an Arduino Nano into an advanced controller board for the table game "UNO Extreme / Attack"

As additional hardware components I utilized a L293D motor driver and a DFPlayerMini Sound module for funny sounds.
Later I integrated an HC-05 bluetooth module to change settings via phone.
The big yellow button will be printed in translucent filament and backlit by two WS2812b LEDs, lighting up accordingly to the game.
<br /><br />

### Build With PlatformIO

This project is now configured for PlatformIO via [platformio.ini](platformio.ini).

1. Install PlatformIO Core (CLI) or open the project in VS Code with the PlatformIO extension.
2. Build:
   <br />
   `pio run`
3. Upload to Arduino Nano:
   <br />
   `pio run -t upload`
4. Open serial monitor:
   <br />
   `pio device monitor`

The required libraries (`DFRobotDFPlayerMini` and `FastLED`) are declared in `platformio.ini` and will be installed automatically by PlatformIO.
<br /><br />
I also made an APP with Thunkable, with which you can change volume, mode and other settings :)
  
You can find the file for the button also on <a href="https://www.thingiverse.com/thing:2492998">Thingiverse</a>.
