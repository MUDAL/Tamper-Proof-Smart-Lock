#include "stm32f10x.h"                  // Device header
#include "gpio.h"
#include <FreeRTOS.h>
#include <task.h>
#include "button.h"

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

bool Button_IsPressed(button_t button,bool* pPrevPressed)
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
	if(!GPIO_InputRead(GPIOA,gpioPin) && !(*pPrevPressed))
	{
		*pPrevPressed = true;
		return true;
	}
	else if(GPIO_InputRead(GPIOA,gpioPin) && *pPrevPressed)
	{
		*pPrevPressed = false;
	}
	return false;
}
