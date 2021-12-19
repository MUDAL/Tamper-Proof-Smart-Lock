#ifndef KEYPAD_PASSWORD_H
#define KEYPAD_PASSWORD_H

#include "keypad.h"

#ifndef MAX_PASSWORD_LEN
#define MAX_PASSWORD_LEN  20
#endif

enum PasswordStates
{
  DEF = 0, //Default
  PASSWORD_CORRECT,
  PASSWORD_INCORRECT
};

extern Keypad keypad;
extern void GetKeypadPassword(char* keypadBuffer);
extern int RetryKeypadPassword(char* keypadBuffer, char* keypadSDBuffer);

#endif //KEYPAD_PASSWORD_H

