#include "stm32f10x.h"                  // Device header
#include "gpio.h"
#include "button.h"

void Button_Init(button_t* pButton)
{
	GPIO_InputInit(pButton->GPIOx,
								 pButton->portLevel,
								 pButton->pin,
								 pButton->config,
								 GPIO_PULLUP_ENABLE);
}

bool Button_IsPressedOnce(button_t* pButton)
{
	if(!GPIO_InputRead(pButton->GPIOx,pButton->pin) && !pButton->prevPressed)
	{
		pButton->prevPressed = true;
		return true;
	}
	else if(GPIO_InputRead(pButton->GPIOx,pButton->pin) && pButton->prevPressed)
	{
		pButton->prevPressed = false;
	}
	return false;
}
