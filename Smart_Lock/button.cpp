#include <Arduino.h>
#include "button.h"

/*
 * @brief Initializes an object of the Button class
 * @param None
 * @return None
*/
Button::Button(int pinNumber)
{
  pin = pinNumber;
  prevPressed = false;
  pinMode(pin,INPUT);
}

/*
 * @brief Checks if a button is pressed once
 * @param None
 * @return true if button is pressed once and false if otherwise
*/
bool Button::IsPressedOnce(void)
{
  if(!digitalRead(pin) && !prevPressed)
  {
    prevPressed = true;
    return true;
  }
  else if(digitalRead(pin) && prevPressed)
  {
    prevPressed = false;
  }
  return false;   
}
