#include "stm32f10x.h"                  // Device header
#include "gpio.h"
#include "output_device.h"

void OutputDev_Init(void)
{
	//Lock(PA3) Init
	GPIO_OutputInit(GPIOA,
								  GPIO_PORT_REG_LOW,
									GPIO_PIN3_OUTPUT_MODE_2MHZ,
									GPIO_GEN_PUR_OUTPUT_PUSH_PULL);	
	//Buzzer(PA8) Init
	GPIO_OutputInit(GPIOA,
								  GPIO_PORT_REG_HIGH,
									GPIO_PIN8_OUTPUT_MODE_2MHZ,
									GPIO_GEN_PUR_OUTPUT_PUSH_PULL);
	//Keep Lock and Buzzer off
	GPIO_OutputWrite(GPIOA,(GPIO_PIN3 | GPIO_PIN8),false);
}

void OutputDev_Write(outputDev_t dev,bool state)
{
	uint16_t gpioPin;
	switch(dev)
	{
		case LOCK:
			gpioPin = GPIO_PIN3;
			break;
		case BUZZER:
			gpioPin = GPIO_PIN8;
			break;
	}
	GPIO_OutputWrite(GPIOA,gpioPin,state);
}

bool OutputDev_Read(outputDev_t dev)
{
	uint16_t gpioPin;
	switch(dev)
	{
		case LOCK:
			gpioPin = GPIO_PIN3;
			break;
		case BUZZER:
			gpioPin = GPIO_PIN8;
			break;
	}
	return GPIO_OutputRead(GPIOA,gpioPin);
}

void OutputDev_Timeout(outputDev_t dev,uint8_t* tCount,uint8_t timeout)
{
	if(OutputDev_Read(dev))
	{
		(*tCount)++;
		if(*tCount == timeout)
		{
			OutputDev_Write(dev,false);
		}
	}
	else
	{
		*tCount = 0;
	}
}
