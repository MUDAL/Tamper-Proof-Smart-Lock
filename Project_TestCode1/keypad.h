#ifndef KEYPAD_H
#define KEYPAD_H

#define NUMBER_OF_ROWS      4
#define NUMBER_OF_COLUMNS   4
#define MAX_PASSWORD_LEN  20

enum PasswordStates
{
  DEF = 0, //Default
  PASSWORD_CORRECT,
  PASSWORD_INCORRECT
};

class Keypad
{
  private:
    int* pRow;
    int* pCol;
    bool pinPrevPressed[NUMBER_OF_ROWS][NUMBER_OF_COLUMNS];
    void SelectRow(int pinIndex);
    bool IsDebounced(int pinIndex);
    
  public:
    Keypad(int* pRowPins,int* pColPins);
    char GetChar(void);
    void GetPassword(char* keyBuffer);
    int RetryPassword(char* keyBuffer,char* sdBuffer);
};

#endif //KEYPAD_H

