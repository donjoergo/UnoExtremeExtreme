#ifndef UNO_EXTREMEEXTREME_VARIABLES_H
#define UNO_EXTREMEEXTREME_VARIABLES_H

#include "Arduino.h"
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"
#include "FastLED.h"
#include "defines.h"

extern volatile bool BtnPressed;

extern SoftwareSerial dfSoftwareSerial;
extern SoftwareSerial BTSoftwareSerial;

extern DFRobotDFPlayerMini myDFPlayer;
extern CRGB leds[NUM_LEDS];

/* GENERAL */
extern bool NormalSounds;
extern bool ModeAscending;
extern bool ExtremeMode;
extern bool bComputerSounds;
extern unsigned int LoseChance;
extern bool FRG_Btn;

/* SOUNDS */
extern unsigned int VOL;
extern unsigned int sound_numbers[sound_folder_num];

/* LED */
extern unsigned int hue;

/* MILLIS */
extern unsigned long millisSave;

/* ASCENDING MODE */
extern bool AscFirstRun;
extern int InitSteps;
extern int ActStep;
extern unsigned int color;
extern int ColorDec;
extern const int soundmap[][10];

/* BLUETOOTH */
extern bool checkBT;
extern bool cmdComplete;
extern bool msgComplete;
extern String command;
extern String buf;
extern int parameter;
extern bool allInNext;

/* MOTION PATTERN */
struct special {
  int folder;
  int sound;
  int volume;
  int waitBefore;
  int waitAfter;
  // TODO Motion pattern incoming
};

struct MotionPattern {
  int duration;
  int speed;
};

extern int MotionPatterns[1][4];
extern special specialSounds[2];
extern int w;

#endif
