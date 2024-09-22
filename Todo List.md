## Software General
- [ ] Differ between GameMode and SoundMode
- [ ] Better retraction for last card after spit out
- [ ] LED color cycle should never go over red or green, in order to avoid misunderstandings
- [ ] Spit out less cards in general


## GameModes
- [ ] Normal (boring)
- [ ] Ascending (also boring, but like the newer UNO Extremes)
- [ ] Extreme Mode
  - [ ] Here we show off all our skills :)
  - [ ] Very important against boredom: Weighted Randomness
    - [ ] Every sound/action has to be played first before a sound/action is played a 2nd time
    - [ ] Remember how fast the button is pressed/time since last press
  - [ ] You would have different categrories all with its own probability
    - [ ] Slow (very little cards)
    - [ ] Medium (spits out a "normal" amount of cards, mode with the highest probability)
    - [ ] High (large amount of cards)
    - [ ] All In (Motor at full speed, spits out all cards for maximum )


## Creative Modes
- [ ] First spit out 1-2 cards, then pause, and then spit out some more cards again
- [ ] Vary the motor speed, e.g. first slow, then fast
- [ ] A stuttering mode, where the moter stutters, e.g. for the sound "083_money_2Cut1"
- [ ] A mode where the user thinks he has broken the device :)
- [ ] Play annoying sound when user presses button again while a sound is already playing


## SoundModes
- [ ] Normal UNO Sounds (boring)
- [ ] Funny Sounds
  - [ ] If this is chosen, a bitmask must be enabled in order to allow all modes to be enabled/disabled independently
  - [ ] Also it is needed to have a probability per SoundMode to adjust its probability
  - [ ] The following funny sounds should be available:
    - [ ] Funny Sounds (Plays sounds from soundboard)
    - [ ] TtsMode (Device 'speaks', and plays soundfiles which are made with TTS, like a robot speaks with you)
    - [ ] Fetish Sounds (Device plays TTS soundfiles which prompt the user to do kinky stuff)
    - [ ] Cantina Band Sound 30min Mix (Requires a reboot :), or a special command via bluetooth)
  - [ ] Also it may be that some sounds have special requirements
    - [ ] If a sound is special: Definition per sound:
      - [ ] Individual motor waiting times before or/and after the sound
      - [ ] Individual sound volumes
      - [ ] Individual motor "motion patterns"
      - [ ] Sounds can be valid for multiple actions (???)
      - [ ] Possible predecessors
        - [ ] E.g: After a waiting sound a specifically chosen win/lose sound from a pool of possible sounds follows

- [ ] Sounds
  - [ ] Warten bei einigen Sounds
  - [ ] Neuer Soundmodus "Computerstimme"
  - [ ] Besserer Zufall für die Sounds. Es sollen zuerst alle Sounds einmal drankommen, bevor ein Sound erneut abgespielt wird


## Sound Files
- [x] Alle Sounds haben ungefähr die gleiche Lautstärke
- [x] Sounds von computergenerierter Stimme
    - https://voicemaker.in
    - https://ttsfree.com/text-to-speech/german
    - https://ttsmp3.com


## Hardware
- [ ] Wackelkontakt bei den LEDs beheben
- [ ] Button vom Case entprellt (evtl. auch mit Software)
- [ ] Schiebeschalter zur Moduswahl (Normaler Modus, Extremer Modus)
  - [ ] DIP Switch evtl. ?
- [ ] USB-C Anschluss
- [ ] Höhere Spannung (12V), z.B. durch StepUp Wandler, oder 3 Zellen
- [ ] Laderegler integriert und Tiefenentladungssicherung
- [ ] Ladestandsanzeiger z.B. durch Ansage per Soundfile
