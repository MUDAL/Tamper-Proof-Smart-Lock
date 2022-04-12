#ifndef KEYPAD_H
#define KEYPAD_H

extern void Keypad_Init(void);
extern char Keypad_GetChar(bool prevPressed[4][4]);

#endif //KEYPAD_H

