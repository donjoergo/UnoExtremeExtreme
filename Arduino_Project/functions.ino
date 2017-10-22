/**********************************FUNCTIONS*************************************/

void DoLose(int sound) {
  if (ExtremeMode && random(100) < 1) {
    AllIn();                                    //Spit out all card at 1% chance
  }
  else {
    FRG_Btn = false;							//
    int speed, duration;   	    			    //Define local variable

    /*MOTOR*/
    if (ExtremeMode) {
      speed = getMotorSpeed();              	//Get speed from function
      duration = getMotorDuration();         	//Get duration from function
    }
    else {
      speed = random(141) + 70;                   //Between 70 - 180
      duration = random(101) + 100;   	        //Between 100ms - 200ms
    }

    /*SOUND*/
    int folder;  	 	  				            //Define local variable
    if (VOL) {
      if (sound != 0) {
        folder = 6;								//Set normal sound folder
        //Sound is used from parameter			//Set sound according to parameter
      }
      else {										//kein aufsteigenderModus
        if (NormalSounds) {
          folder = 6;							    //Set normal sound folder
          sound = 1;								//Set medium sound
        }
        else {
          if (ExtremeMode && speed >= 240) {
            folder = 5;                   		//Set extreme lose sound folder
            sound = random(snds_xtreme_lose) + 1; //Choose random sound
          }
          else {
            folder = 4;                   		//Set lose sound folder
            sound = random(snds_lose) + 1;		//Choose random sound
          }
        }
      }
      PlaySound(folder, sound);               //Play chosen sound
    }

    LightLeds(0, 255, 255);            	    //Turn LEDs red

    /*DELAY*/
    if (folder == 5) {
      switch (sound) {
        case 1:								//Sound "ready to rumble"
          delay(3000);					    //
          break;
        case 3:								//Sound "this is sparta"
          delay(1500);				    	//
          break;
        case 5:								//Sound "WTF BOOM"
          delay(2000);					    //
          break;
      }
    }

    DriveMotor(speed - comp, duration);  	 	//Run the motor
    //SetComp(speed, duration);				//Set the compensation for the cards weight
  }
}

void SetComp(int speed, int duration) {			//Set the compensation for the cards weight
  int factor = int(speed * duration / 3000);	//Calculate the factor
  comp += factor;								//Increase comp
  if (comp >= 80) {
    comp = 80;							    	//Reset comp to 80
  }
  FRG_Btn = true;								//
}												//Function END

void PlayInit() {
  if (VOL) {
    FRG_Btn = false;

    if (ModeAscending) {
      LightLeds(100, 255, 255);			        //Turn LEDs green
    }

    if (NormalSounds) {
      PlaySound(6, 4);                          //Plays intial start sound normal
    }
    else {
      PlaySound(1, random(snds_intro) + 1);     //Plays intial start sound
      delay(500);      		                    //Wait a bit
      /*Blinks LEDs for Time of playing*/
      while (!digitalRead(Busy_PIN)) {
        LightLeds(0, 0, 255);	        		//Turn LEDs white
        delay(500);					        	//Wait for 0.5s
        LightLeds(0, 0, 0);			        	//Turn LEDs black
        delay(500);				        		//Wait for 0.5s
      }
    }

    InitSound = false;						//
    FRG_Btn = true;							//
  }
}

void PlaySound(int folder, int sound) {
  BTSoftwareSerial.end();
  dfSoftwareSerial.begin(9600);
  myDFPlayer.playFolder(folder, sound);
  dfSoftwareSerial.end();
  BTSoftwareSerial.begin(9600);
}

void LoadEeprom() {

}

void SaveEeprom() {

}

void AllIn() {
  int tvol = VOL;
  LightLeds(0, 255, 255);                    //Turn LEDs red
  myDFPlayer.volume(30);                     //Set max volume
  PlaySound(5, random(snds_xtreme_lose));    //Play losing sound
  DriveMotor(255, 5000);                     //Run AllIn Now
  myDFPlayer.volume(tvol);                   //Set volume back
}

void CheckBusy() {							//Check the PlayingStatus
  delay(500);								//Wait for 0.5s
  while (!digitalRead(Busy_PIN)) {
    delay(500);								//Wait for 0.5s
  }
}

void LightLeds(int hue, int sat, int br) {	//Light LEDs in approppiate color
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i].setHSV(hue, sat, br);		    //Loop for every LED
  }
  FastLED.show();							//Show the LEDs
}											//Function END

void PressBtn() {							//Recognize Button Press
  if (FRG_Btn) {
    BtnPressed = true;					    //Set BtnPressed
  }
}

void SetPinModes() {
  pinMode(Button_PIN, INPUT_PULLUP);  //Huge Button PIN
  pinMode(Safety_PIN, INPUT_PULLUP);  //Case-Open PIN
  pinMode(Busy_PIN, INPUT);           //Busy PIN of DFPlayer Mini
  pinMode(EN_PIN, OUTPUT);            //EN PIN for L293D
  pinMode(IN1_PIN, OUTPUT);           //IN1 PIN for L293D
  pinMode(IN2_PIN, OUTPUT);           //IN2 PIN for L293D
}

void InitDfPlayer() {
  if (!myDFPlayer.begin(dfSoftwareSerial)) {  //Use dfSoftwareSerial to communicate with mp3.
    Serial.println("Error connecting to DFPlayer Mini!");
    while (true);
  }
  Serial.println("DFPlayer Mini OK");
}
