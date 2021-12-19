#include <Arduino.h>
#include "keypad_password.h"

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

