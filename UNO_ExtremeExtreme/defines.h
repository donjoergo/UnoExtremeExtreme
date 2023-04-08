/*****************************DEFINE/CONSTANTS***********************************/
#define DEBUGGING_ON 1            // Enable Serial monitor outputs and disable moto spinning
#define DISABLE_MOTOR_SPINNING 1  // 
const bool bSelfTest = false;       // 

/* GENERAL */
bool NormalSounds     = false;      // Use normal or funny sounds
bool ModeAscending    = false;      // Normal or ascending mode

bool ExtremeMode      = true;      // Normal or extreme Mode
bool bComputerSounds  = false;      // Play computergenerated sounds

unsigned int LoseChance = 30;   // Chance to lose in %
unsigned int VOL = 20;          // Volume from 0 - 30


// enum GameMode {     // Enumeration for the selected game mode
//   NormalMode      = 1,    // Binary: 0001
//   AscendingMode   = 2,    // Binary: 0010
//   bExtremeMode     = 4     // Binary: 0100
// };

// enum SoundMode {    // Enumeration for the selected sound mode
//   bNormalSounds    = 1,    // Binary: 00001
//   AscendingSounds = 2,    // Binary: 00010
//   ExtremeSounds   = 4,    // Binary: 00100
//   ComputerSounds  = 8,    // Binary: 01000
//   FetishSounds    = 16    // Binary: 10000
// };
// unsigned short int Mode     = ExtremeMode;                      // TODO implementieren
// unsigned short int Sounds   = ExtremeSounds | ComputerSounds;   // TODO implementieren

/* EXTREME  */
#define WaitChance 10           // Chance to wait before win or lose (%), only applies in extreme mode
#define ShowWaitChance 50       // Chance to show wait (%)
bool FRG_Btn = false;           // TODO


/* MOTOR */
int comp = 0;                   // Compensation value for the motor
#define ReverseSpeed 150        // From 1 - 255 - Speed which the motor retracts cards
#define ReverseTime  100        // Time in ms which the motor retracts cards
#define LowSpdChance 10         // Chance for Low Speed
#define MidSpdChance 50         // Chance for Mid Speed
//HighSpdChance not needed as it calculates out of other two values
#define ShortTmChance 10        // Chance for Short Time
#define MidTmChance 60          // Chance for Mid Time
//LongTmChance not needed as it calculates out of other two values


/* SOUND SETTINGS */
#define snds_intro        14    // Number of sounds
#define snds_wait         36    // Number of sounds
#define snds_win          128   // Number of sounds
#define snds_lose         85    // Number of sounds
#define snds_xtreme_lose  25    // Number of sounds

#define snds_win_computer         24 // Number of sounds
#define snds_lose_computer        13 // Number of sounds
#define snds_xtreme_lose_computer 5  // Number of sounds
#define snds_wait_computer        18  // Number of sounds
bool InitSound = true;              // TODO Kommentar was ist das?

// const // TODO kombinierte variablen für normal + computer sounds

/* LED */
#define NUM_LEDS 2              // Amount of WS2812B LED
#define Brightness 255          // From 0 - 255
#define DelayLED 100            // How Long LED will stay in -color- until it returns to normal mode
#define HueDelay 100            // Time to wait between changes of hue(ms)
unsigned int hue;               // Global variable for hue of LEDs

/* OTHER */
unsigned long millisSave = 0;   // Saves the millis

/* ASCENDING MODE */            // Global variables for the ascending mode
bool AscFirstRun = true;        // Define wether to initialise all variables
int InitSteps;                  // Variable to save the initial amount of steps
int ActStep = 5;                // Variable for the actual step
unsigned int color;             // Green
int ColorDec = 0;               // Amount which the color decreases every step
int soundmap[][10] =            // Create a soundmap for different steps
{ {1},
  {1, 4},
  {1, 3, 5},
  {1, 2, 4, 5},
  {1, 2, 3, 4, 5}
};

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


int MotionPatterns[1][4] = {
  {10,10,10,10}
};

/* BLUETOOTH */
bool checkBT = false;           // wether to check for new settings via BT
bool cmdComplete = false;       // true when new command fully recieved
bool msgComplete = false;       // true when new command and parameter fully recieved
String command;                 // holds the actual command
String buf;                     // temp var for command
int parameter;                  // holds the parameter(s) for the command
bool allInNext = false;         // is set when an allIn should be next ButtonPress

/* IO PINS */
#define Button_PIN 2            // Huge Button PIN
#define Safety_PIN 3            // Case-Open PIN (Case is open when 0)
#define RGB_DATA_PIN 4          // WS2812B Data PIN
#define DF_RX 5                 // RX Serial with DFPlayer
#define DF_TX 6                 // TX Serial with DFPlayer
#define BT_RX 10                // RX Serial with HC-05
#define BT_TX 8                 // TX Serial with HC-05
#define Busy_PIN 7              // DFPlayer Busy/Playing PIN (0 when playing)
#define IN2_PIN 9               // L293D IN2 PIN
#define EN_PIN 11               // L293D Enable PIN
#define IN1_PIN 12              // L293D IN1 PIN

/* SD CARD FOLDERS */
#define FOLDER_INTRO                  1
#define FOLDER_WAIT                   2
#define FOLDER_WIN                    3
#define FOLDER_LOSE                   4
#define FOLDER_EXTREME_LOSE           5
#define FOLDER_ASCENDING              6
#define FOLDER_SETTINGS               7
#define FOLDER_WIN_COMPUTER           8
#define FOLDER_LOSE_COMPUTER          9
#define FOLDER_EXTREME_LOSE_COMPUTER  10
#define FOLDER_WAIT_COMPUTER          11

// map<string, int> speed{ { "ninja", 290 },
//    { "s1000rr", 310 }, { "bullet", 127 },
//    { "Duke", 135 }, { "R1", 286 } };




/* SD CARD EXTREME SOUNDS */
#define SOUND_RUMBLE          1
#define SOUND_OHWAHAHA        2
#define SOUND_SPARTA          3
#define SOUND_TROLOLOL        4
#define SOUND_WTF_BOOM        5
#define SOUND_YEAH            6

/* SD CARD ASCENDING SOUNDS */
#define SOUND_LOSE            1
#define SOUND_VERY_HIGH       2
#define SOUND_HIGH            3
#define SOUND_MID             5
#define SOUND_LOW             4

/* SD CARD SETTINGS SOUNDS */
#define SOUND_BT_SETTINGS_ACTIVATED   1
#define SOUND_SETTINGS_SAVED          2
#define SOUND_VOLUME_CHANGED          3
#define SOUND_MODE_CHANGED            3

// TODO Hier kommen neue Sounds
#define SOUND_INVALID_SETTING         3

#define SOUND_VOLUME_20               3
#define SOUND_VOLUME_40               3
#define SOUND_VOLUME_60               3
#define SOUND_VOLUME_80               3
#define SOUND_VOLUME_100              3

#define SOUND_MODE_NORMAL             3
#define SOUND_MODE_ASCENDING          3
#define SOUND_MODE_EXTREME            3

#define SOUND_SOUNDS_NORMAL           3
#define SOUND_SOUNDS_FUNNY            3
#define SOUND_SOUNDS_COMPUTER         3



special specialSounds[2] = {
  {FOLDER_EXTREME_LOSE, SOUND_OHWAHAHA, 30, 0, 3000},
  {FOLDER_EXTREME_LOSE, SOUND_RUMBLE  , 30, 0, 2000}
};

int w = specialSounds[0].folder;
