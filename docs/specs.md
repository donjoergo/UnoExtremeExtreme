# New Specs

## V2.0

### Game Modes

For now only implement the extreme game mode.
For the futre we can also have the normal game mode ande the ascending game mode.
add already placeholders for normal and ascending modes.
mode can be chosen in config (defines.h). should be an enum.


### Sound Modes

the available sounds are sorted into different categrories. each of them can be switched on or off individually.
each of the sound modes corresponds to a different soundset and folder on the sd card. internally we could work with a bitmask.
each soundmode should have their own probablitity. the base sounds should build the foundation.

- base sounds
  - Normal Sounds (boring)
  - Funny Sounds (Plays sounds from soundboard)
  - TtsMode (Device 'speaks', and plays soundfiles which are made with TTS, like a robot speaks with you)
- Cantina Band light (maybe only 1 min long)
- Cantina Band Sound 30min Mix (Requires a reboot :), or a special command via bluetooth)
- Fetish Sounds (Device plays TTS soundfiles which prompt the user to do kinky stuff)
- Drinking game: Add drinking prompts
  - "Trinke 100ml"
  - "Alle trinken 200ml"
- BDSM mode: Device speaks rude to the users
  - "Schluck die Karten du Luder!"
  - "Ich peitsche dich gleich aus, wenn du weiter so schlecht spielst!"
  - etc.
- Also play sound when the device is opened/closed or while the device is opened for too long
  - When device is left open for too long: "Jetzt misch die scheiss Karten mal schneller!"
  - When device is closed:
    - "Ahh jetzt muss ich euch nicht mehr sehen"
    - "Endlich wieder frische Karten"

### Fix current pain points

#### Sounds played twice after short amount of time

Very important against boredom: Weighted Randomness
Every sound/action has to be played first before a sound/action is played a 2nd time !!!

#### Users got too many cards

Users got too many cards when they had to press the button multiple times in a row.
Idea: Remember how fast the button is pressed/time since last press. If the button is pressed multiple times in a row, the device should spit out less cards.



### Small improvements

- Better retraction for last card after spit out
- LED color cycle should never go over red or green, in order to avoid misunderstandings
- Debounce the button of the device case


### Other funny stuff

Maybe add a mode where the device plays sounds when the user presses the button multiple times in a row.
Maybe add mini games like simon says, but with only one button.


## V 2.1

### Creative Modes

- First spit out 1-2 cards, then pause, and then spit out some more cards again. Play this sound together with a blessing sound
- Vary the motor speed, e.g. first slow, then fast
- A stuttering mode, where the moter stutters, e.g. for the sound "083_money_2Cut1"
- A mode where the user thinks he has broken the device :)
- Play sound when user presses button again while a sound is already playing, stating that "the decive is annoyed by the user"
- Also it may be that some sounds have special requirements
  - If a sound is special: Definition per sound:
    - Individual motor waiting times before or/and after the sound
    - Individual sound volumes
    - Individual motor "motion patterns"
    - Sounds can be valid for multiple actions (???)
    - [ ] Possible predecessors
      - [ ] E.g: After a waiting sound a specifically chosen win/lose sound from a pool of possible sounds follows


## V3

Migrate to a ESP32 with hotspot and a captive portal which hosts the settings webpage for the game device.

Add normal and acscending game modes (the boring stuff)

