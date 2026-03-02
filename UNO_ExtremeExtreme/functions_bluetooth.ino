/**
 * Checks the bluetooth serial connection for new commands
*/
void CheckBT() {
  while (BTSoftwareSerial.available()) {
    char ReadChar = (char)BTSoftwareSerial.read();
    if (ReadChar != '\n') { //Allowed Digits
      // Get Command
      if (!cmdComplete) ReadCmd(ReadChar);

      // Get Parameter
      if (cmdComplete && !msgComplete && ReadChar != '(') {
        if (ReadChar != ')') {
          buf += ReadChar;
        }
        else if (ReadChar == ')') {
          parameter = atoi(buf.c_str());
          buf = "";
          msgComplete = true;
        }
      }
    }
    if (DEBUGGING_ON) {
    Serial.println(command);
    }
  }
  if (msgComplete) {
    checkCmd();
    command = "";
    cmdComplete = false;
    msgComplete = false;
  }
}

/**
 * Reads the incoming buffer 
*/
void ReadCmd(char ReadChar) {
  if (ReadChar == '(') cmdComplete = true;
  else if (!cmdComplete) command += ReadChar;
}

/**
 * //TODO Kommentar
*/
void checkCmd() {
  if (command == "connect") {
    BTSoftwareSerial.println("OK");         // acknowledge
  }
  else if (command == "end") {
    PlaySound(FOLDER_SETTINGS, SOUND_SETTINGS_SAVED); // Play Sound "Einstellungen gespeichert"
    SaveEeprom();                                     // Save to EEPROM
  }
  else if (command == "vol") {
    VOL = map(parameter, 0, 100, 0, 30);              // Remap the value
    myDFPlayer.volume(VOL);                           // Set volume value
    PlaySound(FOLDER_SETTINGS, SOUND_VOLUME_CHANGED); // Play Sound "Lautstärke geändert"
  }
  else if (command == "allin") {
    if (parameter == 0) {
      AllIn();
    }
    if (parameter == 1) {
      allInNext = true;                     // Set AllIn for Next
    }
  }
  
  else if (checkBT) {
    if (command == "mode") {
      switch (parameter) {
        case 0:
          ModeAscending = false;            // Set NormalMode
          ExtremeMode = false;
          break;
        case 1:
          ModeAscending = true;             // Set AscendingMode
          ExtremeMode = false;
          break;
        case 2:
          ModeAscending = false;            // Set ExtremeMode
          ExtremeMode = true;
          break;
      }
      PlaySound(FOLDER_SETTINGS, SOUND_SETTINGS_SAVED); // Play Sound "Einstellungen gespeichert"
    }
    else if (command == "normalsounds") {
      if (parameter == 0) {
        NormalSounds = false;                           // Set FunnySounds
      }
      else if (parameter == 1) {
        NormalSounds = true;                            // Set NormalSounds
      }
      PlaySound(FOLDER_SETTINGS, SOUND_SETTINGS_SAVED); // Play Sound "Einstellungen gespeichert"
    }
    else if (command == "losechance") {
      LoseChance = parameter;                           // Set LoseChance
      PlaySound(FOLDER_SETTINGS, SOUND_SETTINGS_SAVED); // Play Sound "Einstellungen gespeichert"
    }
  }
}
