#include "stm32f10x.h"                  // Device header
#include "gpio.h"
#include "button.h"

static bool buttonPrevPressed[2];

void Button_Init(void)
{
	//INDOOR button(PA0) and Outdoor button (PA1) Init
	GPIO_InputInit(GPIOA,
								 GPIO_PORT_REG_LOW,
								 GPIO_PIN0 | GPIO_PIN1,
								 (GPIO_PIN0_INPUT_PULLUP_OR_PULLDOWN | 
								 GPIO_PIN1_INPUT_PULLUP_OR_PULLDOWN),
								 GPIO_PULLUP_ENABLE);	
}

bool Button_IsPressedOnce(button_t button)
{
	uint16_t gpioPin;
	switch(button)
	{
		case INDOOR:
			gpioPin = GPIO_PIN0;
			break;
		case OUTDOOR:
			gpioPin = GPIO_PIN1;
			break;
	}
	if(!GPIO_InputRead(GPIOA,gpioPin) && !buttonPrevPressed[button])
	{
		buttonPrevPressed[button] = true;
		return true;
	}
	else if(GPIO_InputRead(GPIOA,gpioPin) && buttonPrevPressed[button])
	{
		buttonPrevPressed[button] = false;
	}
	return false;
}
