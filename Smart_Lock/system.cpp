#include <Arduino.h>
#include "system.h"

static volatile bool stateArr[4] = {false};

void System_SetState(int state,bool val)
{
  stateArr[state] = val;
}

bool System_GetState(int state)
{
  return stateArr[state];
}

/*
 * @brief Actuates the 'buzzer' or 'lock' of the system
 * @param outputDev: pin mapped to either the 'buzzer' or 'lock'
 * @param val: if val is 'true', the 'buzzer' or 'lock' will be actuated,
 * if val is 'false', the 'buzzer' or 'lock' will be de-activated.
 * @return None
*/
void ActuateOutput(int outputDev,bool val)
{
  switch(outputDev)
  {
    case BUZZER:
      digitalWrite(BUZZER,val);
      System_SetState(BUZZER_ON,val);
      break;
    case LOCK:
      digitalWrite(LOCK,val);
      System_SetState(DOOR_UNLOCKED,val);
      break;
  }
}


