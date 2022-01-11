#ifndef BUTTON_H
#define BUTTON_H

class Button
{
  private:
    int pin;
    bool prevPressed;
    
  public:
    Button(int pinNumber);
    bool IsPressedOnce(void);
};

#endif //BUTTON_H

