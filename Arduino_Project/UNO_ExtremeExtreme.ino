/*********************************************************************************
  +------------------+-----------------------------------------------------------+
  |  Project         | UNO ExtremeExtreme                                        |
  |  Author          | Jörg Dorlach (donjoergo.bplaced.de)                       |
  |  Last Modified   | 31.03.17                                                  |
  |  Version         | V0.1                                                      |
  |  Microcontroller | Atmel Atmega328 (Arduino Nano)                            |
  |  File            | UNO_ExtremeExtreme.ino                                    |
  +------------------+---------------------------+-------------------------------+
  |  Date            |Author                     |What is done?                  |
  |  20.02.17        |Jörg Dorlach               | Created                       |
  |                  |                           |                               |
  +------------------+---------------------------+-------------------------------+
  |  Arduino Sketch for a UNO Exteme/Attack with custom built circuit board,     |
  |  featuring a L293D motor driver and a DFPlayer Mini. The speed and duration  |
  |  of the motor is chosen randomly by the Arduino. The big push button in the  |
  |  front is made out of transparent filament and backlit by two WS2812B LEDs   |
  |  which light up accordingly to the game.                                     |
  +------------------------------------------------------------------------------+
*********************************************************************************/

/*********************************INCLUDE****************************************/
#include "Arduino.h"				//Standart library
#include "SoftwareSerial.h"			//Required to communicate with DFPlayer Mini
#include "DFRobotDFPlayerMini.h"	//Required for playing sounds from the DFPlayer Mini
#include "FastLED.h"				//Required for the WS2812B LED stripe
#include "defines.h"				//Include all defines

/********************************INITIALISE**************************************/

volatile bool BtnPressed = false; 				//Used by interrupt for ButtonState

SoftwareSerial dfSoftwareSerial(DF_RX, DF_TX);	//Initialise COM to the DFPlayer
DFRobotDFPlayerMini myDFPlayer;					//define DFPlayer Mini object
CRGB leds[NUM_LEDS];							//define array with NUM_LEDS

//void printDetail(uint8_t type, int value);     //define function

/**********************************SETUP*****************************************/
void setup() {					//Setup BEGIN
    Serial.begin(9600);         //Just for debugging


    /*RANDOM*/
    randomSeed(analogRead(0));	//Getting better random values
    hue = random(256);          //Setting a random start hue between 0 - 255


    /*DFPlayer Mini*/
    dfSoftwareSerial.begin(9600); 		//Start serial comuincation with DFPlayer Mini
    InitDfPlayer();               		//Debugging?
    myDFPlayer.volume(VOL);     		//Set volume value

    SetPinModes();						//Sets all necessary PinModes

    /*FAST LED*/
    FastLED.addLeds<WS2812B, RGB_DATA_PIN, GRB>(leds, NUM_LEDS); //Initialise LEDs
    FastLED.setBrightness(Brightness);                           //Set LED Brightness


    /*INTERRUPT*/
    attachInterrupt(digitalPinToInterrupt(Button_PIN), PressBtn, FALLING); //Define interrupt

	FRG_Btn = true;
}											//Setup END

/***********************************LOOP*****************************************/
void loop()															//Loop BEGIN
{
    if (BtnPressed && !digitalRead(Safety_PIN)) {          			//If START (Button pressed)
        FRG_Btn = false;
		if (ModeAscending) {										//If START (Ascending Mode)
            if (AscFirstRun) {										//If START (First Run)
                ActStep = random(5);								//Choose new round
                if (ActStep != 0) {									//Only initialise if necessary
                    color = 100;
					InitSteps = ActStep;							//Save amount of steps
                    ColorDec = int(color / (InitSteps + 1));		//Calculate ColorDecrease
                    AscFirstRun = false;							//Reset FirstRun				
				}
				
            }														//If END (First Run)
            if (ActStep > 0) {
                color -= ColorDec;									//Change color
				if (ActStep == 1) {color = 10;}
                LightLeds(color, 255, 255);							//Turn LEDs orange
				if (VOL){myDFPlayer.playFolder(6, soundmap[InitSteps][ActStep]);}//Play sound
                ActStep--;
				Serial.println(color);
            }
            else if (ActStep == 0) {								//Lose
                AscFirstRun = true;									//Set FirstRun again
                DoLose(soundmap[InitSteps][ActStep]);				//Call the lose function
                LightLeds(100, 255, 255);							//Turn LEDs green
            }
        }															//If END (Ascending Mode)
		else {															//Else BEGIN (Normal Mode)
			if (ExtremeMode) {											//If BEGIN (ExtremeMode)
				int wait = random(100); 								//Decide randomly wether to wait
				if (wait < WaitChance) {								//If BEGIN (WAIT)
					int showWait = random(100);  						//Decide randomly wether to showWait
					if (showWait < ShowWaitChance && VOL && !NormalSounds) {	//If BEGIN (showWait)
						myDFPlayer.playFolder(2, random(snds_wait) + 1);		//Play waiting sound
						delay(500);      		             					//Wait a bit
						/*Blinks LEDs for Time of playing*/
						while (!digitalRead(Busy_PIN)) {						//While BEGIN (checks if playing)
							LightLeds(0, 0, 255);								//Turn LEDs white
							delay(500);											//Wait for 0.5s
							LightLeds(0, 0, 0);									//Turn LEDs black
							delay(500);											//Wait for 0.5s
						}
					}													//IF END showWait
					else {												//ELSE BEGIN showWait
						LightLeds(0, 0, 0);								//Turn LEDs black
						delay(random(3000) + 2000);						//Wait 2s - 5s
					}
				}														//If END (WAIT)
			}															//IF END (ExtremeMode)

			int lose = random(100);									  	//Decide randomly wether to win
			if (lose < LoseChance) {									//If BEGIN (LOSE)
				DoLose(0);												//Call the DoLose function
			}                											//If END (LOSE)
			else {														//Else BEGIN (Winning)
				LightLeds(100, 255, 255);								//Turn LEDs green
				if (VOL) {
					if (NormalSounds){myDFPlayer.playFolder(6, 4);} 		//Play winning sound
					else {myDFPlayer.playFolder(3, random(snds_win) + 1);} 	//Play winning sound
				}
			}															//Else END (Winning)

			CheckBusy();												//Check PlayingStatus
			delay(DelayLED);											//Wait a bit
        }                                                       		//ELSE END (Normal Mode)
		FRG_Btn = true;
		BtnPressed = false;												//Reset BtnPressed
    }
    
	/*Menü*/
	else if (BtnPressed && digitalRead(Safety_PIN)) {				//If BEGIN (Menu)
        if (!MenuActivated) {										//If BGIN (MenuActivated)
            BtnCounter++;											//Increment BtnCounter
        }															//If END (MenuActivated)
        if (BtnCounter >= 300) {									//If BEGIN (BtnCounter)
            millisMenuSave = millis();									//Save the actual millis value
            MenuActivated = 1;										//Set MenuActivated
            BtnCounter = 0;											//Reset BtnCounter
        }															//If END (BtnCounter)
        if (millis() - millisMenuSave >= MenuTimeout * 1000) {		//If BEGIN (MenuTimeout)
            MenuActivated = 0;										//Reset MenuActivated
        }															//If END (MenuTimeout)

        if (MenuActivated) {										//If BEGIN (MenuActivated)
            if (Brightness) {										//If BEGIN (Brightness)
                LightLeds(0, 0, 255); 								//Turn LEDs white
            }														//If END (Brightness)
            myDFPlayer.playFolder(7, 1);							//Play sound "Menü"
            CheckBusy();											//Check PlayingStatus
            BtnPressed = false;										//Reset BtnPressed
            delay(500);												//Wait 5s
       }															//If END (MenuActivated)
    }                   											//If END (Menu)

	/*Case open*/
    if (!BtnPressed && digitalRead(Safety_PIN)) {					//Case open
        LightLeds(0, 0, 0);											//Turn LEDs black
		InitSound = true;											//
    }

	/*Case closed*/
	if (!digitalRead(Safety_PIN) && InitSound){PlayInit();}		//Play initial start sound
	
	if (!digitalRead(Safety_PIN) && !ModeAscending && millis() > millisSave+HueDelay) {		//Cycle through hue when NormalMode
        LightLeds(hue, 255, 255);								//Light LEDs in hue color
        hue++;													//
		millisSave = millis();									//
	}															//If END (Cycle)
	
    //delay(10);													//Wait for 10ms
}																//Loop END
