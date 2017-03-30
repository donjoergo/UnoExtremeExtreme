/*****************************DEFINE/CONSTANTS***********************************/

/*GAME SETTINGS*/
bool NormalSounds = false;		//Use normal or funny sounds
bool ModeAscending = false;		//Normal or ascending mode
bool ExtremeMode = true;		//Normal or extreme Mode
unsigned int LoseChance = 40;	//Chance to lose in %
#define WaitChance 10			//Chance to wait before win or lose (%)
#define ShowWaitChance 50		//Chance to show wait (%)
bool FRG_Btn = false;			//


/*MOTOR*/
int comp = 0;					//Compensation value for the motor
#define ReverseSpeed 150   		//From 1 - 255 - Speed which the motor retracts cards
#define ReverseTime  100   		//Time in ms which the motor retracts cards
#define LowSpdChance 20			//Chance for Low Speed
#define MidSpdChance 70			//Chance for Mid Speed
//HighSpdChance not needed as it calculates out of other two values
#define ShortTmChance 10		//Chance for Short Time
#define MidTmChance 60			//Chance for Mid Time
//LongTmChance not needed as it calculates out of other two values


/*SOUND SETTINGS*/
unsigned int VOL = 20;			//Volume from 0 - 30
#define snds_intro 6       		//Number of sounds
#define snds_wait 6        		//Number of sounds
#define snds_win 53        		//Number of sounds
#define snds_lose 39       		//Number of sounds
#define snds_xtreme_lose 7 		//Number of sounds
bool InitSound = true;


/*LED*/
#define NUM_LEDS 2         		//Amount of WS2812B LED
#define Brightness 150     		//From 0 - 255
#define DelayLED 100			//How Long LED will stay in -color- until it returns to normal mode
#define HueDelay 100			//Time to wait between changes of hue(ms)
unsigned int hue;				//Global variable for hue of LEDs
unsigned long millisSave = 0;	//Saves the millis


/*MENU*/
bool MenuActivated = 0;			//Menu is deactivated at startup
unsigned int BtnCounter = 0;	//Variable to increment while Btn is pressed
unsigned long millisMenuSave = 0;//Saves the millis at moment of MenuActivated
#define MenuTimeout 30      	//Timeout in sec


/*ASCENDING MODE*/				//Global variables for the ascending mode
bool AscFirstRun = true;		//Define wether to initialise all variables
int InitSteps;					//Variable to save the initial amount of steps
int ActStep = 5;				//Variable for the actual step
unsigned int color;				//Green
int ColorDec = 0;				//Amount which the color decreases every step
int soundmap[][10] =			//Create a soundmap for different steps
{ {1},
  {1, 4},
  {1, 3, 5},
  {1, 2, 4, 5},
  {1, 2, 3, 4, 5}
};


/*IO PINS*/
#define Button_PIN 2       		//Huge Button PIN
#define Safety_PIN 3       		//Case-Open PIN (Case is open when 0)
#define RGB_DATA_PIN 4     		//WS2812B Data PIN
#define DF_TX 6            		//TX Serial with DFPlayer
#define DF_RX 5            		//RX Serial with DFPlayer
#define Busy_PIN 7         		//DFPlayer Busy/Playing PIN (0 when playing)
#define IN2_PIN 9         		//L293D IN2 PIN
#define EN_PIN 11          		//L293D Enable PIN
#define IN1_PIN 12         		//L293D IN1 PIN
