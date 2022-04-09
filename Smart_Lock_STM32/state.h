#ifndef STATE_H
#define STATE_H

#define NUM_OF_STATES	 5

enum States
{
  FAILED_INPUT,
  LOCK_TAMPERED,
  DOOR_UNLOCKED,
  BUZZER_ON,
  INVALID_FINGERPRINT
};

extern void SetState(uint8_t state,bool val);
extern bool GetState(uint8_t state);

#endif //STATE_H
