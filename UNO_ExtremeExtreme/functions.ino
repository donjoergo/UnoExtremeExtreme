/**********************************FUNCTIONS*************************************/

/**
 * Handles the losing
 * For the ascending mode the sound is being passed to this function 
 * // TODO This needs to be changed
 * Else sound = 0 passed
 * @param sound Sound number, it is passed in ascending mode
*/
void DoLose(int sound) {
  if (ExtremeMode && random(100) < 1) {
    AllIn();                                    // Spit out all card at 1% chance
  }
  else {
    FRG_Btn = false;                            // TODO Kommentar
    int speed, duration;                        // Define local variable

    /*MOTOR*/
    if (ExtremeMode) {
      speed     = getMotorSpeed();              // Get speed from function
      duration  = getMotorDuration();           // Get duration from function
    }
    else {
      speed     = random(141) + 70;             // Between 70 - 180
      duration  = random(101) + 100;            // Between 100ms - 200ms
    }

    /*SOUND*/
    int folder;                                 // Define local variable
    if (VOL) {
      if (sound != 0) {
        folder = FOLDER_ASCENDING;              // Set normal sound folder
        //Sound is used from parameter          // Set sound according to parameter
      }
      else {                                    // kein aufsteigenderModus
        if (NormalSounds) {
          folder = FOLDER_ASCENDING;            // Set normal sound folder
          sound = SOUND_LOSE;                   // Set lose sound
        }
        else {
          if (ExtremeMode && speed >= 240) {
            if (bComputerSounds) {
              folder = FOLDER_EXTREME_LOSE_COMPUTER;         // Set extreme lose sound folder
              sound = random(snds_xtreme_lose_computer) + 1; // Choose random sound
            }
            else {
              folder = FOLDER_EXTREME_LOSE;                 // Set extreme lose sound folder
              sound = random(snds_xtreme_lose) + 1;         // Choose random sound
            }
          }
          else {
            if (bComputerSounds) {
              folder = FOLDER_LOSE_COMPUTER;               // Set lose sound folder
              sound = random(snds_lose_computer) + 1;      // Choose a random lose sound
            }
            else 
              folder = FOLDER_LOSE;                       // Set lose sound folder
              sound = random(snds_lose) + 1;              // Choose a random lose sound
          }
        }
      }
      PlaySound(folder, sound);                 // Play chosen sound
    }

    LightLeds(0, 255, 255);                     // Turn LEDs red

    // When the sound folder is extreme 
    // For some sounds we have to wait a specific time before continuing
    if (folder == FOLDER_EXTREME_LOSE) {
      switch (sound) {
        case SOUND_RUMBLE:
          delay(3000);
          break;
        case SOUND_SPARTA:
          delay(1500);
          break;
        case SOUND_WTF_BOOM:
          delay(2000);
          break;
      }
      // TODO Insert new sounds here
    }

    DriveMotor(speed - comp, duration);         // Run the motor
    //SetComp(speed, duration);                 // Set the compensation for the cards weight
  }
}

/**
 * Set the compensation for the cards weight
 * @param speed
 * @param duration
*/
void SetComp(int speed, int duration) {
  int factor = int(speed * duration / 3000);    // Calculate the factor
  comp += factor;                               // Increase comp
  if (comp >= 80) {
    comp = 80;                                  // Reset comp to 80
  }
  FRG_Btn = true;                               // TODO Kommentar
}                                               // Function END

/**
 * Plays an initial startup sound after switching the machine on
*/
void PlayInit() {
  if (VOL) {
    FRG_Btn = false;

    if (ModeAscending) {
      LightLeds(100, 255, 255);                 // Turn LEDs green
    }

    if (NormalSounds) {
      PlaySound(FOLDER_ASCENDING, SOUND_LOW);   // Plays intial start sound normal
    }
    else {
      PlaySound(FOLDER_INTRO, random(snds_intro) + 1);  // Plays intial start sound
      delay(500);                                       // Wait a bit
      /*Blinks LEDs for Time of playing*/
      while (!digitalRead(Busy_PIN)) {
        LightLeds(0, 0, 255);                   // Turn LEDs white
        delay(500);                             // Wait for 0.5s
        LightLeds(0, 0, 0);                     // Turn LEDs black
        delay(500);                             // Wait for 0.5s
      }
    }

    InitSound = false;                          // TODO Kommentar
    FRG_Btn = true;                             // TODO Kommentar
  }
}

/**
 * Plays a given sound via the DfPlayer
 * It also handles the softwareserial communication with the DfPlayer
 * @param folder Folder number
 * @param sound Sound number
*/
void PlaySound(int folder, int sound) {
  BTSoftwareSerial.end();                 // Stop listening on bluetooth
  dfSoftwareSerial.begin(9600);           // Start communication with DfPlayer
  myDFPlayer.playFolder(folder, sound);   // Ask DfPlayer to play the given sound
  dfSoftwareSerial.end();                 // Stop communication with DfPlayer
  BTSoftwareSerial.begin(9600);           // Start listening on bluetooth
}

/**
 * //TODO Diese Funktion sollte wohl mal verwendet werden; Ist das Kunst oder kann das weg
*/
void LoadEeprom() {

}

/**
 * //TODO Diese Funktion sollte wohl mal verwendet werden; Ist das Kunst oder kann das weg
*/
void SaveEeprom() {

}

/**
 * This plays an extreme lose sound at maximum volume and spits out the entire card stack at maximum motor speed
*/
void AllIn() {
  int tvol = VOL;                                             // Save the previous volume
  LightLeds(0, 255, 255);                                     // Turn LEDs red
  myDFPlayer.volume(30);                                      // Set max volume
  PlaySound(FOLDER_EXTREME_LOSE, random(snds_xtreme_lose));   // Play losing sound
  DriveMotor(255, 5000);                                      // Run AllIn Now
  myDFPlayer.volume(tvol);                                    // Set volume back
}

/**
 * Blocks the program flow for as long as we read a busy signal from the DfPlayer
*/
void CheckBusy() {
  delay(500);                               // Wait for 0.5s
  while (!digitalRead(Busy_PIN)) {          // Wait for as long as we read a busy signal
    delay(500);                             // Wait for 0.5s // TODO maybe smaller waiting time?
  }
}

/**
 * Lights all LEDs in the passed color
 * @param hue Hue
 * @param sat Saturation
 * @param br Brightness
*/
void LightLeds(int hue, int sat, int br) {  // Light LEDs in approppiate color
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i].setHSV(hue, sat, br);           // Loop for every LED
  }
  FastLED.show();                           // Show the LEDs
}                                           // Function END

/**
 * Recognize Button Press. Works only if the button is released
 * An interrupt is directly attached to this function
*/
void PressBtn() {
  if (FRG_Btn) {
    BtnPressed = true;                      // Set BtnPressed if it is released
  }
}

/**
 * Initializes the pins of the microcontroller
*/
void SetPinModes() {
  pinMode(Button_PIN, INPUT_PULLUP);        // Huge Button PIN
  pinMode(Safety_PIN, INPUT_PULLUP);        // Case-Open PIN
  pinMode(Busy_PIN, INPUT);                 // Busy PIN of DFPlayer Mini
  pinMode(EN_PIN, OUTPUT);                  // EN PIN for L293D
  pinMode(IN1_PIN, OUTPUT);                 // IN1 PIN for L293D
  pinMode(IN2_PIN, OUTPUT);                 // IN2 PIN for L293D
}

/**
 * Initializes the DfPlayer
*/
void InitDfPlayer() {
  if (!myDFPlayer.begin(dfSoftwareSerial)) {
    if (DEBUGGING_ON) {
      Serial.println("Error connecting to DFPlayer Mini!");
    }
    while (true);
  }
  if (DEBUGGING_ON) {
    Serial.println("DFPlayer Mini OK");
  }
}

/**
 * Performs an self test
 * Plays all sounds and see if it is playable
*/
void SelfTest() {
  if (DEBUGGING_ON) {
    Serial.println("Performing self test...");
  }
  VOL = 10;
  myDFPlayer.volume(VOL);

  for (int i = 0; i < snds_intro; i++) {
    PlaySound(FOLDER_INTRO, i);
    delay(1500);
    if (DEBUGGING_ON) {
      Serial.println((String)"Playing Sound: " + FOLDER_INTRO + ", " + i);
    }
  }
}