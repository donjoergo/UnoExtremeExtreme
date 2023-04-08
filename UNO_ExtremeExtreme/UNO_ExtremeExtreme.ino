/*********************************************************************************
  +------------------+-----------------------------------------------------------+
  |  Project         | UNO ExtremeExtreme                                        |
  |  Author          | Jörg Dorlach (donjoergo.bplaced.de)                       |
  |  Last Modified   | 31.03.17                                                  |
  |  Date            |Author                     |What is done?                  |
  |  20.02.17        |Jörg Dorlach               |Created                        |
  |  17.09.17        |Jörg Dorlach               |Added BT support               |
  |  17.04.23        |Jörg Dorlach               |//TODO                         |
  +------------------+---------------------------+-------------------------------+
  |  Version         | V0.1                                                      |
  |  Microcontroller | Atmel Atmega328 (Arduino Nano)                            |
  |  File            | UNO_ExtremeExtreme.ino                                    |
  +------------------+---------------------------+-------------------------------+
  |  Arduino Sketch for a UNO Exteme/Attack with custom built circuit board,     |
  |  featuring a L293D motor driver and a DFPlayer Mini. The speed and duration  |
  |  of the motor is chosen randomly by the Arduino. The big push button in the  |
  |  front is made out of transparent filament and backlit by two WS2812B LEDs   |
  |  which light up accordingly to the game.                                     |
  +------------------------------------------------------------------------------+
*********************************************************************************/

/*********************************INCLUDE****************************************/
#include "Arduino.h"                // Standart library
#include "SoftwareSerial.h"         // Required to communicate with DFPlayer Mini
#include "DFRobotDFPlayerMini.h"    // Required for playing sounds from the DFPlayer Mini
#include "FastLED.h"                // Required for the WS2812B LED stripe
#include "defines.h"                // Include all defines

/********************************INITIALISE**************************************/

volatile bool BtnPressed = false;               // Used by interrupt for ButtonState

SoftwareSerial dfSoftwareSerial(DF_RX, DF_TX);  // Initialise COM with the DFPlayer
SoftwareSerial BTSoftwareSerial(BT_RX, BT_TX);  // Initialise COM with HC-05

DFRobotDFPlayerMini myDFPlayer;                 // define DFPlayer Mini object
CRGB leds[NUM_LEDS];                            // define array with NUM_LEDS

//void printDetail(uint8_t type, int value);    // define function

/**********************************SETUP*****************************************/
void setup() {
  if (DEBUGGING_ON) {
    Serial.begin(9600);
  }

  /*RANDOM*/
  randomSeed(analogRead(0));        // Getting better random values
  hue = random(256);                // Setting a random start hue between 0 - 255


  /*DFPlayer Mini*/
  dfSoftwareSerial.begin(9600);     // Start serial communication with DFPlayer Mini
  InitDfPlayer();                   // Init the DFPlayer
  myDFPlayer.volume(VOL);           // Set volume value

  SetPinModes();                    // Sets all necessary PinModes

  /*FAST LED*/
  FastLED.addLeds<WS2812B, RGB_DATA_PIN, GRB>(leds, NUM_LEDS); // Initialise LEDs
  FastLED.setBrightness(Brightness);                           // Set LED Brightness


  /*INTERRUPT*/
  attachInterrupt(digitalPinToInterrupt(Button_PIN), PressBtn, FALLING); // Define interrupt

  dfSoftwareSerial.end();
  BTSoftwareSerial.begin(9600);
  FRG_Btn = true;

  // Perform self test
  if (bSelfTest) {
    SelfTest();
  }
}

/***********************************LOOP*****************************************/
void loop() {
  if (BtnPressed && !digitalRead(Safety_PIN)) {
    FRG_Btn = false;
    if (ModeAscending) {
      if (AscFirstRun) {
        ActStep = random(5);                            // Choose new round
        if (ActStep != 0) {                             // Only initialise if necessary
          color = 100;
          InitSteps = ActStep;                          // Save amount of steps
          ColorDec = int(color / (InitSteps + 1));      // Calculate ColorDecrease
          AscFirstRun = false;                          // Reset FirstRun
        }
      }
      if (ActStep > 0) {
        color -= ColorDec;                              // Change color
        if (ActStep == 1) {
          color = 10;
        }
        LightLeds(color, 255, 255);                     // Turn LEDs orange
        if (VOL) {
          PlaySound(FOLDER_ASCENDING, soundmap[InitSteps][ActStep]);  // Play sound
        }
        ActStep--;
        if (DEBUGGING_ON) {
          Serial.println(color);
        }
      }
      else if (ActStep == 0) {                          // Lose
        AscFirstRun = true;                             // Set FirstRun again
        DoLose(soundmap[InitSteps][ActStep]);           // Call the lose function
        LightLeds(100, 255, 255);                       // Turn LEDs green
      }
    }
    else {
      if (ExtremeMode) {
        int wait = random(100);                                     // Decide randomly wether to wait
        if (wait < WaitChance) {
          int showWait = random(100);                               // Decide randomly wether to showWait
          if (showWait < ShowWaitChance && VOL && !NormalSounds) {
            if (bComputerSounds) {
              PlaySound(FOLDER_WAIT, random(snds_wait) + 1);          // Play waiting sound
            }
            else {
              PlaySound(FOLDER_WAIT_COMPUTER, random(snds_wait) + 1); // Play waiting sound
            }
            delay(500);                                             // Wait a bit
            /*Blinks LEDs for Time of playing*/
            while (!digitalRead(Busy_PIN)) {
              LightLeds(0, 0, 255);                                 // Turn LEDs white
              delay(500);                                           // Wait for 0.5s
              LightLeds(0, 0, 0);                                   // Turn LEDs black
              delay(500);                                           // Wait for 0.5s
            }
          }
          else {
            LightLeds(0, 0, 0);                                     // Turn LEDs black
            delay(random(3000) + 2000);                             // Wait 2s - 5s
          }
        }
      }

      if (allInNext) {
        AllIn();                                                    // Go ALLIN NOW
        allInNext = false;                                          // Reset variable
      }
      else {
        int lose = random(100);                                     // Decide randomly wether to win
        if (lose < LoseChance) {
          DoLose(0);                                                // Call the DoLose function
        }
        else {
          LightLeds(100, 255, 255);                                 // Turn LEDs green
          if (VOL) {
            if (NormalSounds) {
              PlaySound(FOLDER_ASCENDING, SOUND_LOW);               // Play winning sound
            }
            if (bComputerSounds) {
              PlaySound(FOLDER_WIN_COMPUTER, random(snds_win_computer) + 1); // Play winning sound
            }
            else {
              PlaySound(FOLDER_WIN, random(snds_win) + 1);          // Play winning sound
            }
          }
        }
      }

      CheckBusy();                                              // Check PlayingStatus
      delay(DelayLED);                                          // Wait a bit
    }
    FRG_Btn = true;
    BtnPressed = false;                                         // Reset BtnPressed
  }

  else if (BtnPressed && !checkBT && digitalRead(Safety_PIN)) { // Btn pressed and case open
    checkBT = true;                                             // release BT-checking
    LightLeds(160, 255, 255);                                   // blue
    PlaySound(FOLDER_SETTINGS, SOUND_BT_SETTINGS_ACTIVATED);    // Play "BT activated"
    BtnPressed = false;
    dfSoftwareSerial.end();
    BTSoftwareSerial.begin(9600);                               // Begin COM with HC-05
  }

  /*Case open and BT not released*/
  if (!BtnPressed && !checkBT && digitalRead(Safety_PIN)) {     // Case open and BT not released
    LightLeds(0, 0, 0);                                         // Turn LEDs black
    InitSound = true;                                           // TODO
  }

  /*Case closed*/
  if (InitSound && !digitalRead(Safety_PIN)) {
    //BTSoftwareSerial.end();
    //dfSoftwareSerial.begin(9600);                             // Start serial communication with DFPlayer Mini
    checkBT = false;                                            // lock BT-checking
    PlayInit();                                                 // Play initial start sound
  }

  if (!digitalRead(Safety_PIN) && !ModeAscending && millis() > millisSave + HueDelay) {   // Cycle through hue when NormalMode
    LightLeds(hue, 255, 255);                                                             // Light LEDs in hue color
    hue++;                                                                                // TODO
    millisSave = millis();                                                                // TODO
  }
  CheckBT();                                                                              // Check BT for new commands
}
