/*
 * Test code 1
 * Description:
 * 1.Simple program to input a password into the system...
 * using the keypad or bluetooth.
 * 2.If the password is correct, Serial.println("Correct")
 * 3.If the password is incorrect, Serial.println("Incorrect")...
 * and give 2 retries.
 * 4.If the password is incorrect after all 3 attempts, SendSMS("Intruder")
*/

#include "sd_card.h"
#include "communication.h"
#include "keypad.h"

//Password buffers
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

void setup() 
{
  Serial.begin(115200);
  Serial2.begin(9600,SERIAL_8N1,-1,17); //for SIM800L
  SerialBT.begin("Smart Door");
  SD.begin(); //Uses pins 23,19,18 and 5
  SD_ReadFile(SD,"/kp.txt",keypadSDBuffer);
  SD_ReadFile(SD,"/bt.txt",bluetoothSDBuffer);
}

void loop() 
{
  char key = keypad.GetChar();
  if(key == '*')
  {
    Serial.println("Entering password mode");
    keypad.GetPassword(keypadBuffer);
    if(!strcmp(keypadBuffer,keypadSDBuffer))
    {
      Serial.println("Password is correct");
    }
    else
    {
      Serial.println("Incorrect password, 2 attempts left");
      int retry = keypad.RetryPassword(keypadBuffer,keypadSDBuffer);
      if(retry == PASSWORD_CORRECT)
      {
        Serial.println("Password is now correct");
      }
      else if(retry == PASSWORD_INCORRECT)
      {
        Serial.println("Intruder!!!!!");
      }
    }
  }
}
