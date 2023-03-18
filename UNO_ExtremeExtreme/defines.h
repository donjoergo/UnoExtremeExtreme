/*****************************DEFINE/CONSTANTS***********************************/

/*GAME SETTINGS*/
bool NormalSounds = true;       // Use normal or funny sounds
bool ModeAscending = false;     // Normal or ascending mode
bool ExtremeMode = false;       // Normal or extreme Mode
unsigned int LoseChance = 40;   // Chance to lose in %
#define WaitChance 10           // Chance to wait before win or lose (%)
#define ShowWaitChance 50       // Chance to show wait (%)
bool FRG_Btn = false;           // TODO


/*MOTOR*/
int comp = 0;                   // Compensation value for the motor
#define ReverseSpeed 150        // From 1 - 255 - Speed which the motor retracts cards
#define ReverseTime  100        // Time in ms which the motor retracts cards
#define LowSpdChance 10         // Chance for Low Speed
#define MidSpdChance 50         // Chance for Mid Speed
//HighSpdChance not needed as it calculates out of other two values
#define ShortTmChance 10        // Chance for Short Time
#define MidTmChance 60          // Chance for Mid Time
//LongTmChance not needed as it calculates out of other two values


/*SOUND SETTINGS*/
unsigned int VOL = 25;          // Volume from 0 - 30
#define snds_intro 6            // Number of sounds
#define snds_wait 6             // Number of sounds
#define snds_win 53             // Number of sounds
#define snds_lose 39            // Number of sounds
#define snds_xtreme_lose 7      // Number of sounds
bool InitSound = true;


/*LED*/
#define NUM_LEDS 2              // Amount of WS2812B LED
#define Brightness 255          // From 0 - 255
#define DelayLED 100            // How Long LED will stay in -color- until it returns to normal mode
#define HueDelay 100            // Time to wait between changes of hue(ms)
unsigned int hue;               // Global variable for hue of LEDs
unsigned long millisSave = 0;   // Saves the millis

/*ASCENDING MODE*/              // Global variables for the ascending mode
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

/*BLUETOOTH*/
bool checkBT = false;           // wether to check for new settings via BT
bool cmdComplete = false;       // true when new command fully recieved
bool msgComplete = false;       // true when new command and parameter fully recieved
String command;                 // holds the actual command
String buf;                     // temp var for command
int parameter;                  // holds the parameter(s) for the command
bool allInNext = false;         // is set when an allIn should be next ButtonPress

/*IO PINS*/
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
