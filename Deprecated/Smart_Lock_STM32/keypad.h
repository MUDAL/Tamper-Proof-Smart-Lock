#ifndef KEYPAD_H
#define KEYPAD_H

#define NUMBER_OF_ROWS      4
#define NUMBER_OF_COLUMNS   4

extern void Keypad_Init(void);
extern char Keypad_GetChar(void);

#endif //KEYPAD_H

