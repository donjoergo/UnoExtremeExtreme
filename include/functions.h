#ifndef UNO_EXTREMEEXTREME_FUNCTIONS_H
#define UNO_EXTREMEEXTREME_FUNCTIONS_H

void PlayInit();
void PlaySound(int folder, int sound);
void ReadSoundFileNumbers();
void InitDfPlayer();

void DriveMotor(int tempo, int duration);
int getMotorSpeed();
int getMotorDuration();

void DoLose(int sound);
void AllIn();
void CheckBusy();
void LightLeds(int hue, int sat, int br);
void PressBtn();
void SetPinModes();

void CheckBT();
void ReadCmd(char ReadChar);
void checkCmd();

void SelfTest();
void LoadEeprom();
void SaveEeprom();

#endif
