#ifndef DOOR_LOCK_SYSTEM_H
#define DOOR_LOCK_SYSTEM_H

#define BUZZER  12
#define LOCK    13

enum States
{
  FAILED_INPUT,
  LOCK_TAMPERED
};

extern void System_SetState(int state,bool val);
extern bool System_GetState(int state);
extern void System_ActuateBuzzer(bool val);
extern bool System_IsBuzzerOn(void);
extern void System_ActuateLock(bool val);
extern bool System_IsDoorOpen(void);

#endif //DOOR_LOCK_SYSTEM_H
