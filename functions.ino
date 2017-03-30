/**********************************FUNCTIONS*************************************/

void DoLose(int sound) {
    FRG_Btn = false;								//
	int speed, duration;   	    			        //Define local variable
    
	/*MOTOR*/
    if (ExtremeMode) {                  			//IF BEGIN (ExtremeMode activated)
        speed = getMotorSpeed();              		//Get speed from function
        duration = getMotorDuration();            	//Get duration from function
    }   											//If END (ExtremeMode activated)
    else {                      					//Else BEGIN (ExtremeMode deactivated)
        speed = random(111) + 70;                   //Between 70 - 180
        duration = random(101) + 100;   	        //Between 100ms - 200ms
    }                           					//Else END (ExtremeMode deactivated)
	
    /*SOUND*/
    int folder;  	 	  				            //Define local variable
    if (VOL) {										//If BEGIN (NOT muted)
        if (sound != 0) {							//If BEGIN (ModeAscending)
            folder = 6;								//Set normal sound folder
            //Sound is used from parameter			//Set sound according to parameter
        }											//If END (ModeAscending)
        else {												//kein aufsteigenderModus
            if (NormalSounds) {								//If BEGIN (NormalSounds is chosen)
                folder = 6;									//Set normal sound folder
                sound = 1;									//Set medium sound
            }												//If END (NormalSounds is chosen)
            else {											//Else BEGIN (FunnySounds is chosen)
                if (ExtremeMode && speed >= 180) {			//If BEGIN (speed is normal)
                    folder = 5;                   			//Set extreme lose sound folder
                    sound = random(snds_xtreme_lose) + 1;   //Choose random sound
                }											//If END (speed is normal)
                else {										//Else (speed is EXTREME)
                    folder = 4;                   			//Set lose sound folder
                    sound = random(snds_lose) + 1;			//Choose random sound
                }											//Else END (speed is EXTREME)
            }												//Else END (FunnySounds is chosen)
        }													//Else END ()
        myDFPlayer.playFolder(folder, sound);   //Play chosen sound
    }											//If END (NOT muted)

    LightLeds(0, 255, 255);            			//Turn LEDs red
duration = 200; //Debug

    /*DELAY*/
    if (folder == 5) {							//If BEGIN (Sound is extreme)
        switch (sound) {						//Switch BEGIN (sound)
            case 1:								//Sound "ready to rumble"
                delay(3000);					//
                break;
			case 3:								//Sound "this is sparta"
                delay(1500);					//
                break;
            case 5:								//Sound "WTF BOOM"
                delay(2000);					//
                break;
        }  										//Switch END (sound)
    }											//If BEGIN (Sound is extreme)

    DriveMotor(speed - comp, duration);  	 	//Run the motor
    //SetComp(speed, duration);					//Set the compensation for the cards weight
}

void SetComp(int speed, int duration) {			//Set the compensation for the cards weight
    int factor = int(speed * duration / 3000);	//Calculate the factor
    comp += factor;								//Increase comp
    if (comp >= 80) {							//If BEGIN (comp >= 80)
        comp = 80;								//Reset comp to 80
    }											//If END (comp >= 80)
	FRG_Btn = true;								//
}												//Function END

void PlayInit(){
	if (VOL) {
        FRG_Btn = false;
		
		if (ModeAscending) {
			LightLeds(100, 255, 255);			//Turn LEDs green
		}
		
		if (NormalSounds){
			myDFPlayer.playFolder(6, 4); //Plays intial start sound normal
		}
		else{
			myDFPlayer.playFolder(1, random(snds_intro) + 1); //Plays intial start sound
			delay(500);      		            //Wait a bit
			/*Blinks LEDs for Time of playing*/
			while (!digitalRead(Busy_PIN)) {	//While BEGIN (checks if playing)
				LightLeds(0, 0, 255);			//Turn LEDs white
				delay(500);						//Wait for 0.5s
				LightLeds(0, 0, 0);				//Turn LEDs black
				delay(500);						//Wait for 0.5s
			}									//While END
		}
		
		InitSound = false;						//
		FRG_Btn = true;							//
    }
}

void CheckBusy() {								//Check the PlayingStatus
    delay(500);									//Wait for 0.5s
    while (!digitalRead(Busy_PIN)) {			//While BEGIN (check for PlayingStatus)
        delay(500);								//Wait for 0.5s
    }											//While END (check for PlayingStatus)
}												//Function END

void LightLeds(int hue, int sat, int br) {	//Light LEDs in approppiate color
    for (int i = 0; i < NUM_LEDS; i++) {	//For BEGIN
        leds[i].setHSV(hue, sat, br);		//Loop for every LED
    }										//For END
    FastLED.show();							//Show the LEDs
}											//Function END

void PressBtn() {							//Recognize Button Press
    if (FRG_Btn) {							//IF BEGIN (Freigabe)
        BtnPressed = true;					//Set BtnPressed
    }										//If END (Freigabe)
}											//Function END

void SetPinModes() {
    pinMode(Button_PIN, INPUT_PULLUP);  //Huge Button PIN
    pinMode(Safety_PIN, INPUT_PULLUP);  //Case-Open PIN
    pinMode(Busy_PIN, INPUT);           //Busy PIN of DFPlayer Mini
    pinMode(EN_PIN, OUTPUT);            //EN PIN for L293D
    pinMode(IN1_PIN, OUTPUT);           //IN1 PIN for L293D
    pinMode(IN2_PIN, OUTPUT);           //IN2 PIN for L293D
}

void InitDfPlayer() {
    if (!myDFPlayer.begin(dfSoftwareSerial)) {  //Use softwareSerial to communicate with mp3.
        Serial.println("Error connecting to DFPlayer Mini!");
        while (true);
    }
    Serial.println("DFPlayer Mini OK");
}
