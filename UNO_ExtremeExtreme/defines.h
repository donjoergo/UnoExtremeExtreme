/*****************************DEFINE/CONSTANTS***********************************/
/* DEBUGGING OPTIONS */
#define DEBUGGING_ON        1  // Enable serial monitor outputs
#define LOG_LEVEL           3  // 1 = , 2 = , 3 = Highly detailed Log
#define MOTOR_SPINNING      1  // 1 = motor spinning disabled
bool bSelfTest = false;        // Perform selftest


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
#define WaitChance      10      // Chance to wait before win or lose (%), only applies in extreme mode
#define ShowWaitChance  50      // Chance to show wait (%)

/* MOTOR */
//int comp = 0;                   // Compensation value for the motor
#define ReverseSpeed    150     // From 1 - 255, Speed which the motor retracts cards
#define ReverseTime     100     // Time in ms which the motor retracts cards
#define LowSpdChance    10      // Chance in % for Low Speed
#define MidSpdChance    50      // Chance in % for Mid Speed
//HighSpdChance not needed as it calculates out of other two values
#define ShortTmChance   10      // Chance in % for Short Time
#define MidTmChance     60      // Chance in % for Mid Time
//LongTmChance not needed as it calculates out of other two values

#define MOTOR_BOOST_THRESHOLD 150   // If the speed is lower than this, it gets boosted
#define MOTOR_BOOST_SPEED     170   // Speed with which the motor should be boosted
#define MOTOR_BOOST_TIME      50    // Time in ms how long the motor should be boosted





/* SOUND SETTINGS */
// TODO should be read out automatically
// value = myDFPlayer.readFileCounts(); //read all file counts in SD card
// value = myDFPlayer.readFileCountsInFolder(3); //read file counts in folder SD:/03
#define sound_folder_num  11    // Amount of sound folders

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




/* LED */
#define NUM_LEDS 2              // Amount of WS2812B LED
#define Brightness 255          // From 0 - 255
#define DelayLED 100            // How Long LED will stay in -color- until it returns to normal mode
#define HueDelay 100            // Time to wait between changes of hue(ms)

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
#define SOUND_MODE_CHANGED            4

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


/* IO PINS */
#define Button_PIN    2         // Huge Button PIN
#define Safety_PIN    3         // Case-Open PIN (Case is open when 0)
#define RGB_DATA_PIN  4         // WS2812B Data PIN
#define DF_RX_PIN     5         // RX Serial with DFPlayer
#define DF_TX_PIN     6         // TX Serial with DFPlayer
#define BT_RX_PIN     10        // RX Serial with HC-05
#define BT_TX_PIN     8         // TX Serial with HC-05
#define Busy_PIN      7         // DFPlayer Busy/Playing PIN (0 when playing)
#define IN2_PIN       9         // L293D IN2 PIN
#define EN_PIN        11        // L293D Enable PIN
#define IN1_PIN       12        // L293D IN1 PIN
