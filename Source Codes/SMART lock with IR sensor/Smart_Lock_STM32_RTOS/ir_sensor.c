#include "stm32f10x.h"                  // Device header
#include "gpio.h"
#include "ir_sensor.h"

/**
	* @brief Initializes the IR sensor
	* @param None
	* @return None
*/
void IRSensor_Init(void)
{
	//GPIO Init (PA4)
	GPIO_InputInit(GPIOA,
								 GPIO_PORT_REG_LOW,
								 GPIO_PIN4,
								 GPIO_PIN4_INPUT_PULLUP_OR_PULLDOWN,
								 GPIO_PULLUP_ENABLE);
}

/**
	* @brief Checks if the device has been tampered with. 
	* (This is due to the IR sensor's state being high)
	* @param None
	* @return true if tamper has been detected
	* @return false if tamper has not been detected
*/
bool IRSensor_TamperDetected(void)
{
	return GPIO_InputRead(GPIOA,GPIO_PIN4);
}
