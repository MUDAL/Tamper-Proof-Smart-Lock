#include <Arduino.h>
#include "password.h"

void GetKeypadPassword(char* keypadBuffer)
{
  int i = 0;
  while(1)
  {
    char key = keypad.GetChar();
    switch(key)
    {
      case '\0':
        break;
      case '#':
        keypadBuffer[i] = '\0';
        return;
      default:
        if(i < MAX_PASSWORD_LEN)
        {
          keypadBuffer[i] = key;
          i++;
        }
        break;
    }
  }
}

void GetBluetoothPassword(char* bluetoothBuffer)
{
  int i = 0;
  bool bufferCleared = false;
  char clearSerialBT = '\0';
  while(SerialBT.available() > 0)
  {
    if(!bufferCleared)
    {
      memset(bluetoothBuffer,'\0',MAX_PASSWORD_LEN);
      bufferCleared = true;
    }
    if(i < MAX_PASSWORD_LEN)
    {
      bluetoothBuffer[i] = SerialBT.read();
    }
    else
    {
      clearSerialBT = SerialBT.read();
    }
    i++;
  }
  Serial.print("Bluetooth data = ");
  Serial.println(bluetoothBuffer);
}

int RetryKeypadPassword(char* keypadBuffer, char* keypadSDBuffer)
{ 
  int passwordState = DEF;
  int retry = 1;
  while(retry <= 2)
  {
    Serial.print("Retry: ");
    Serial.println(retry);
    GetKeypadPassword(keypadBuffer);
    retry++;
    if(!strcmp(keypadBuffer,keypadSDBuffer))
    {
      passwordState = PASSWORD_CORRECT;
      break;
    }
    else
    {
      passwordState = PASSWORD_INCORRECT;
    }
  }
  return passwordState; 
}

