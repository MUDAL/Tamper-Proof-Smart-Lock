/*
 * Test code 2
 * Description:
 * Extension of test 1
*/

#include "sd_card.h"
#include "communication.h"
#include "keypad.h"

char keypadBuffer[MAX_PASSWORD_LEN] = {0};
char keypadSDBuffer[MAX_PASSWORD_LEN] = {0};
char bluetoothBuffer[MAX_BT_SERIAL_LEN] = {0};
char bluetoothSDBuffer[MAX_PASSWORD_LEN] = {0};
//Keypad
int rowPins[NUMBER_OF_ROWS] = {16,22,32,33};
int columnPins[NUMBER_OF_COLUMNS] = {25,26,27,14};
Keypad keypad(rowPins,columnPins);
//Bluetooth
BluetoothSerial SerialBT;

//Functions
void ProcessBluetoothData(void);
void InputPassword(void);
void InputNewPassword(void);
void ChangePassword(void);
void AddPhoneNumber(void);

void setup() 
{
  Serial.begin(115200);
  Serial2.begin(9600,SERIAL_8N1,-1,17); //for SIM800L
  SerialBT.begin("Smart Door");
  SD.begin(); //Uses pins 23,19,18 and 5
  SD_ReadFile(SD,"/kp.txt",keypadSDBuffer);
  Serial.print("Keypad password:");
  Serial.println(keypadSDBuffer);
  SD_ReadFile(SD,"/bt.txt",bluetoothSDBuffer);
}

void loop() 
{
  //Bluetooth
  ProcessBluetoothData();
  //Keypad
  char key = keypad.GetChar();
  switch(key)
  {
    case '*':
      InputPassword();
      break;
    case 'C':
      ChangePassword();
      break;
    case 'A':
      AddPhoneNumber();
      break;
  }
}

void ProcessBluetoothData(void)
{
  GetBluetoothData(bluetoothBuffer);
  if(strcmp(bluetoothBuffer,"\0") != 0)
  {
    if(strcmp(bluetoothBuffer,bluetoothSDBuffer) == 0)
    {
      Serial.println("Password is correct");
    }
    /*else if(strcmp(bluetoothBuffer,"Read") == 0)
    {
    }*/
    else
    {
      Serial.println("Incorrect password");
    }
    memset(bluetoothBuffer,'\0',MAX_BT_SERIAL_LEN);
  }  
}

void InputPassword(void)
{
  Serial.println("Entering password mode");
  keypad.GetPassword(keypadBuffer);
  if(strcmp(keypadBuffer,keypadSDBuffer) == 0)
  {
    Serial.println("Password is correct");
  }
  else
  {
    Serial.println("Incorrect password, 2 attempts left");
    int retry = keypad.RetryPassword(keypadBuffer,keypadSDBuffer);
    switch(retry)
    {
      case PASSWORD_CORRECT:
        Serial.println("Password is now correct");
        break;
      case PASSWORD_INCORRECT:
        Serial.println("Intruder!!!!!");
        break;
    }
  }
}

void InputNewPassword(void)
{
  char newPassword[MAX_PASSWORD_LEN] = {0};
  Serial.println("Correct, now enter new password");
  keypad.GetPassword(keypadBuffer);
  strcpy(newPassword,keypadBuffer);
  Serial.println("Reenter the new password");
  keypad.GetPassword(keypadBuffer);
  if(strcmp(keypadBuffer,newPassword) == 0)
  {
    Serial.println("New password successfully created");
    strcpy(keypadSDBuffer,keypadBuffer);
    SD_WriteFile(SD,"/kp.txt",keypadBuffer);
  }
  else
  {
    Serial.println("Incorrect input, 2 attempts left");
    int retry = keypad.RetryPassword(keypadBuffer,newPassword);
    switch(retry)
    {
      case PASSWORD_CORRECT:
        Serial.println("New password successfully created");
        strcpy(keypadSDBuffer,keypadBuffer);
        SD_WriteFile(SD,"/kp.txt",keypadBuffer);
        break;
      case PASSWORD_INCORRECT:
        Serial.println("Unable to create new password");
        break;
    } 
  }  
}

void ChangePassword(void)
{
  Serial.println("Enter previous password");
  keypad.GetPassword(keypadBuffer);
  if(strcmp(keypadBuffer,keypadSDBuffer) == 0)
  {
    InputNewPassword();
  }
  else
  {
    Serial.println("Incorrect password, 2 attempts left");
    int retry = keypad.RetryPassword(keypadBuffer,keypadSDBuffer);
    switch(retry)
    {
      case PASSWORD_CORRECT:
        InputNewPassword();
        break;
      case PASSWORD_INCORRECT:
        Serial.println("Intruder!!!!!");
        break;
    }  
  }
}

void AddPhoneNumber(void)
{
}
