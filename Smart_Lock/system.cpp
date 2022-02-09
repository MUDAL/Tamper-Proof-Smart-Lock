#include <Arduino.h>
#include "system.h"

static volatile bool stateArr[2] = {false};
static volatile bool buzzerOn = false;
static volatile bool doorOpen = false;

void System_SetState(int state,bool val)
{
  stateArr[state] = val;
}

bool System_GetState(int state)
{
  return stateArr[state];
}

void System_ActuateBuzzer(bool val)
{
  digitalWrite(BUZZER,val);
  buzzerOn = val;
}

bool System_IsBuzzerOn(void)
{
  return buzzerOn;
}

void System_ActuateLock(bool val)
{
  digitalWrite(LOCK,val);
  doorOpen = val; 
}

bool System_IsDoorOpen(void)
{
  return doorOpen;
}

