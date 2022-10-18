#include "stm32f10x.h"                  // Device header
#include "gpio.h"
#include "output_device.h"

/**
	* @brief Initialize output devices (LOCK and BUZZER)
	* @param None
	* @return None
*/
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

/**
	* @brief Turns output device on/off
	* @param dev: Output device (LOCK or BUZZER)
	* @param state: @arg true (turns dev on)
	*								@arg false (turns dev off)
	* @return None
*/
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

/**
	* @brief Get current state of output device
	* @param dev: Output device (LOCK or BUZZER)
	* @return true if dev is on,
	* @return false if dev is off.
*/
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
