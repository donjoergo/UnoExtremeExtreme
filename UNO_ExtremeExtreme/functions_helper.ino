/**
 * Performs an self test
 * Plays all sounds and see if it is playable
*/
void SelfTest() {
  if (DEBUGGING_ON) {
    Serial.println("Performing self test...");
  }
  VOL = 10;
  myDFPlayer.volume(VOL);

  for (int i = 0; i < snds_intro; i++) {
    PlaySound(FOLDER_INTRO, i);
    delay(1500);
    // if (DEBUGGING_ON) {
    //   Serial.println((String)"Playing Sound: " + FOLDER_INTRO + ", " + i);
    // }
  }
}

/**
 * Set the compensation for the cards weight
 * @param speed
 * @param duration
*/
// void SetComp(int speed, int duration) {
//   int factor = int(speed * duration / 3000);    // Calculate the factor
//   comp += factor;                               // Increase comp
//   if (comp >= 80) {
//     comp = 80;                                  // Reset comp to 80
//   }
//   FRG_Btn = true;                               // TODO Kommentar
// }

/**
 * //TODO Diese Funktion sollte wohl mal verwendet werden; Ist das Kunst oder kann das weg
*/
void LoadEeprom() {

}

/**
 * //TODO Diese Funktion sollte wohl mal verwendet werden; Ist das Kunst oder kann das weg
*/
void SaveEeprom() {

}