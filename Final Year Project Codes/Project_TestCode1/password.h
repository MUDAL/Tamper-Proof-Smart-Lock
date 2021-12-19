#ifndef PASSWORD_H
#define PASSWORD_H

#include "keypad.h"
#include "BluetoothSerial.h"

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
extern BluetoothSerial SerialBT;
extern void GetKeypadPassword(char* keypadBuffer);
extern void GetBluetoothPassword(char* bluetoothBuffer);
extern int RetryKeypadPassword(char* keypadBuffer, char* keypadSDBuffer);

#endif //PASSWORD_H

