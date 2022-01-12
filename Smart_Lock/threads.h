#ifndef THREADS_H
#define THREADS_H

enum States
{
  DOOR_UNLOCKED = 0,
  FAILED_INPUT,
  BUZZER_ON,
  LOCK_TAMPERED
};

extern void SetState(int state,bool val);
extern bool GetState(int state);
extern void Threads_Init(void);

#endif //THREADS_H

