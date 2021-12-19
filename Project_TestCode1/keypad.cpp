#include <Arduino.h>
#include "keypad.h"

const char keypadMatrix[NUMBER_OF_ROWS][NUMBER_OF_COLUMNS] =
{{'1','2','3','A'},
 {'4','5','6','B'},
 {'7','8','9','C'},
 {'*','0','#','D'}};

void Keypad::SelectRow(int pinIndex)
{
  for(int i = 0; i < NUMBER_OF_ROWS; i++)
  {
    if(i == pinIndex)
    {
      digitalWrite(pRow[i],LOW);
    }
    else
    {
      digitalWrite(pRow[i],HIGH);
    }
  }
}

bool Keypad::IsDebounced(int pinIndex)
{
  if(digitalRead(pCol[pinIndex]) == LOW)
  {
    delay(15); //De-bounce delay
    if(digitalRead(pCol[pinIndex]) == LOW)
    {
      return true;
    }
  }
  return false;
}

Keypad::Keypad(int* pRowPins,int* pColPins)
{
  //Initialize private variables
  pRow = pRowPins;
  pCol = pColPins;
  for(int i = 0; i < NUMBER_OF_ROWS; i++)
  {
    for(int j = 0; j < NUMBER_OF_COLUMNS; j++)
    {
      pinPrevPressed[i][j] = false;
    }
  }  
  //Initialize rows
  for(int i = 0; i < NUMBER_OF_ROWS; i++)
  {
    pinMode(pRowPins[i],OUTPUT);
  }
  //Initialize columns
  for(int i = 0; i < NUMBER_OF_COLUMNS; i++)
  {
    pinMode(pColPins[i],INPUT_PULLUP);
  }
}

char Keypad::GetChar(void)
{
  for(int i = 0; i < NUMBER_OF_ROWS; i++)
  {
    Keypad::SelectRow(i);
    for(int j = 0; j < NUMBER_OF_COLUMNS; j++)
    {
      if(Keypad::IsDebounced(j) && !pinPrevPressed[i][j])
      {
        pinPrevPressed[i][j] = true;
        return keypadMatrix[i][j];
      }
      else if(!Keypad::IsDebounced(j) && pinPrevPressed[i][j])
      {
        pinPrevPressed[i][j] = false;
      }
    }
  }
  return '\0';
}

void Keypad::GetPassword(char* keyBuffer)
{
  int i = 0;
  while(1)
  {
    char key = Keypad::GetChar();
    switch(key)
    {
      case '\0':
        break;
      case '#':
        keyBuffer[i] = '\0';
        return;
      default:
        if(i < MAX_PASSWORD_LEN)
        {
          keyBuffer[i] = key;
          i++;
        }
        break;
    }
  }  
}

int Keypad::RetryPassword(char* keyBuffer,char* sdBuffer)
{
  int passwordState = DEF;
  int retry = 1;
  while(retry <= 2)
  {
    Serial.print("Retry: ");
    Serial.println(retry);
    Keypad::GetPassword(keyBuffer);
    retry++;
    if(!strcmp(keyBuffer,sdBuffer))
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

