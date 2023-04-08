
/**********************************MOTOR*FUNCTIONS*******************************/

/**
 * Drives the motor with a given speed and duration
*/
void DriveMotor(int tempo, int duration) {
  if (DEBUGGING_ON) {
    if (tempo < 150) {                        // If BEGIN (boosts motor to get started)
      digitalWrite(IN1_PIN, LOW);             // Set forward direction
      digitalWrite(IN2_PIN, HIGH);            // Set forward direction
      analogWrite(EN_PIN, 170);               // Set medium speed
      delay(50);                              // Wait 50ms
    }                                         // If END
    //Forward as long as duration
    digitalWrite(IN1_PIN, LOW);               // Set forward direction
    digitalWrite(IN2_PIN, HIGH);              // Set forward direction
    analogWrite(EN_PIN, tempo);               // Set speed
    delay(duration);                          // Wait for time of duration

    //Backward as long as ReverseTime
    digitalWrite(IN1_PIN, HIGH);              // Set backward direction
    digitalWrite(IN2_PIN, LOW);               // Set backward direction
    analogWrite(EN_PIN, ReverseSpeed);        // Set speed
    delay(ReverseTime);                       // Wait for time of ReverseTime

    //Soft Brake
    analogWrite(EN_PIN, 0);                   // Set speed
  }
  if (DEBUGGING_ON) {
    Serial.println(tempo);
    Serial.println(duration);
  }
}                                           // Function END

/**
 * Outputs the motor speed
 * Takes different probabilities into account
 * Is called when in extreme mode
*/
int getMotorSpeed() {
  int SpeedSel = random(100);
  int range, offset;
  if (SpeedSel < LowSpdChance) {
    range = 49;
    offset = 120;
  }
  else if (SpeedSel < (LowSpdChance + MidSpdChance)) {
    range = 90;
    offset = 150;
  }
  else if (SpeedSel > (LowSpdChance + MidSpdChance)) {
    range = 16;
    offset = 240;
  }
  int speed = random(range) + offset;       // Choose random value within given range
  return speed;                             // Return speed
}

/**
 * Outputs the motor duration
 * Takes different probabilities into account
 * Is called when in extreme mode
*/
int getMotorDuration() {
  int SpeedSel = random(100);
  int range, offset;
  if (SpeedSel < ShortTmChance) {
    range = 51;
    offset = 50;
  }
  else if (SpeedSel < (ShortTmChance + MidTmChance)) {
    range = 251;
    offset = 100;
  }
  else if (SpeedSel > (ShortTmChance + MidTmChance)) {
    range = 651;
    offset = 350;
  }
  int duration = random(range) + offset;        // Choose random value within given range
  return duration;                              // Return duration
}
