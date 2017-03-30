
/**********************************MOTOR*FUNCTIONS*******************************/

void DriveMotor(int tempo, int duration) {	//Driving the motor
    if (tempo < 150) {						//If BEGIN (boosts motor to get started)
        digitalWrite(IN1_PIN, LOW);				//Set forward direction
        digitalWrite(IN2_PIN, HIGH);			//Set forward direction
        analogWrite(EN_PIN, 170);				//Set medium speed
        delay(50);						    	//Wait 50ms
    }											//If END
    //Forward as long as duration
    digitalWrite(IN1_PIN, LOW);				//Set forward direction
    digitalWrite(IN2_PIN, HIGH);				//Set forward direction
    analogWrite(EN_PIN, tempo);				//Set speed
    delay(duration);							//Wait for time of duration

    //Backward as long as ReverseTime
    digitalWrite(IN1_PIN, HIGH);				//Set backward direction
    digitalWrite(IN2_PIN, LOW);				//Set backward direction
    analogWrite(EN_PIN, ReverseSpeed);		//Set speed
    delay(ReverseTime);						//Wait for time of ReverseTime

    //Soft Brake
    analogWrite(EN_PIN, 0);                   //Set speed
}											//Function END

int getMotorSpeed() {
    int SpeedSel = random(100);
    int range, offset;
    if (SpeedSel < LowSpdChance) {
        range = 61;
        offset = 70;
        Serial.println("Low");
    }
    else if (SpeedSel < (LowSpdChance + MidSpdChance)) {
        range = 51;
        offset = 130;
        Serial.println("Mid");
    }
    else if (SpeedSel > (LowSpdChance + MidSpdChance)) {
        range = 76;
        offset = 180;
        Serial.println("High");
    }
    int speed = random(range) + offset;		//Choose random value within given range
    return speed;							//Return speed
}

int getMotorDuration() {
    int SpeedSel = random(100);
    int range, offset;
    if (SpeedSel < ShortTmChance) {
        range = 51;
        offset = 50;
        Serial.println("Short");
    }
    else if (SpeedSel < (ShortTmChance + MidTmChance)) {
        range = 251;
        offset = 100;
        Serial.println("Mid");
    }
    else if (SpeedSel > (ShortTmChance + MidTmChance)) {
        range = 651;
        offset = 350;
        Serial.println("Long");
    }
    int duration = random(range) + offset;		//Choose random value within given range
    return duration;							//Return duration
}
