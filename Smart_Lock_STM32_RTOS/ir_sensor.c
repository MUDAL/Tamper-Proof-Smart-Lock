#include "stm32f10x.h"                  // Device header
#include "gpio.h"
#include "ir_sensor.h"

void IRSensor_Init(void)
{
	//GPIO Init (PA4)
	GPIO_InputInit(GPIOA,
								 GPIO_PORT_REG_LOW,
								 GPIO_PIN4,
								 GPIO_PIN4_INPUT_PULLUP_OR_PULLDOWN,
								 GPIO_PULLUP_ENABLE);
}

bool IRSensor_TamperDetected(void)
{
	if(GPIO_InputRead(GPIOA,GPIO_PIN4))
	{
		return true;
	}
	return false;
}
