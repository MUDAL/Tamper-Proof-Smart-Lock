#ifndef DOOR_LOCK_SYSTEM_H
#define DOOR_LOCK_SYSTEM_H

#define BUZZER  12
#define LOCK    13

enum States
{
  FAILED_INPUT,
  LOCK_TAMPERED,
  DOOR_UNLOCKED,
  BUZZER_ON
};

extern void System_SetState(int state,bool val);
extern bool System_GetState(int state);
extern void ActuateOutput(int outputDev,bool val);

#endif //DOOR_LOCK_SYSTEM_H
