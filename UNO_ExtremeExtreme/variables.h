
volatile bool BtnPressed = false;               // Used by interrupt for ButtonState

SoftwareSerial dfSoftwareSerial(DF_RX_PIN, DF_TX_PIN);  // Initialise COM with the DFPlayer
SoftwareSerial BTSoftwareSerial(BT_RX_PIN, BT_TX_PIN);  // Initialise COM with HC-05

DFRobotDFPlayerMini myDFPlayer;     // define DFPlayer Mini object
CRGB leds[NUM_LEDS];                // define array with NUM_LEDS

/* GENERAL */
bool NormalSounds       = false;    // Use normal or funny sounds
bool ModeAscending      = false;    // Normal or ascending mode
bool ExtremeMode        = true;     // Normal or extreme Mode
bool bComputerSounds    = false;    // Play computergenerated sounds
unsigned int LoseChance = 30;       // Chance to lose in %
bool FRG_Btn = false;               // TODO


/* SOUNDS */
unsigned int VOL        = 20;       // Volume from 0 - 30
unsigned int sound_numbers[sound_folder_num];


/* LED */
unsigned int hue;               // Global variable for hue of LEDs


/* MILLIS */
unsigned long millisSave = 0;   // Saves the millis
// TODO more millis needed



/* ASCENDING MODE */            // Global variables for the ascending mode
bool AscFirstRun = true;        // Define wether to initialise all variables
int InitSteps;                  // Variable to save the initial amount of steps
int ActStep = 5;                // Variable for the actual step
unsigned int color;             // Green
int ColorDec = 0;               // Amount which the color decreases every step
const int soundmap[][10] =            // Create a soundmap for different steps
{ {1},
  {1, 4},
  {1, 3, 5},
  {1, 2, 4, 5},
  {1, 2, 3, 4, 5}
};


/* BLUETOOTH */
bool checkBT     = false;       // wether to check for new settings via BT
bool cmdComplete = false;       // true when new command fully recieved
bool msgComplete = false;       // true when new command and parameter fully recieved
String command;                 // holds the actual command
String buf;                     // temp var for command
int parameter;                  // holds the parameter(s) for the command
bool allInNext   = false;       // is set when an allIn should be next ButtonPress


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


int MotionPatterns[1][4] = {
  {10,10,10,10}
};


special specialSounds[2] = {
  {FOLDER_EXTREME_LOSE, SOUND_OHWAHAHA, 30, 0, 3000},
  {FOLDER_EXTREME_LOSE, SOUND_RUMBLE  , 30, 0, 2000}
};

int w = specialSounds[0].folder;

// map<string, int> speed{ { "ninja", 290 },
//    { "s1000rr", 310 }, { "bullet", 127 },
//    { "Duke", 135 }, { "R1", 286 } };

