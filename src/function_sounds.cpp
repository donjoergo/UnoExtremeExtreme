#include "Arduino.h"
#include "defines.h"
#include "variables.h"
#include "functions.h"

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
      delay(800);                                       // Wait a bit
      /*Blinks LEDs for Time of playing*/
      while (!digitalRead(Busy_PIN)) {
        Serial.println("Intro busy");
        LightLeds(0, 0, 255);                   // Turn LEDs white
        delay(500);                             // Wait for 0.5s
        LightLeds(0, 0, 0);                     // Turn LEDs black
        delay(500);                             // Wait for 0.5s
      }
      delay(500);                               // Wait for 0.5s
      Serial.println("Intro finished");
    }

    InitSound = false;                          // TODO Kommentar
    FRG_Btn = true;                             // TODO Kommentar
  }
}

/**
 * Plays a given sound via the DfPlayer
 * It also handles the softwareserial communication with the DfPlayer
 * @param folder  Folder number
 * @param sound Sound number
*/
void PlaySound(int folder, int sound) {
  if (DEBUGGING_ON) {
    Serial.println((String)"[Playing Sound] folder: " + folder + ", file: " + sound);
  }
  BTSoftwareSerial.end();                 // Stop listening on bluetooth
  dfSoftwareSerial.begin(9600);           // Start communication with DfPlayer
  myDFPlayer.playFolder(folder, sound);   // Ask DfPlayer to play the given sound
  dfSoftwareSerial.end();                 // Stop communication with DfPlayer
  BTSoftwareSerial.begin(9600);           // Start listening on bluetooth
}

/**
 * Reads how much files are on the SD card
*/
// cppcheck-suppress unusedFunction
void ReadSoundFileNumbers() {
  
  if (DEBUGGING_ON) {
    Serial.println((String)"[Read File numbers]");
  }
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
