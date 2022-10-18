#include "stm32f10x.h"                  // Device header
#include <stdbool.h>
#include "clock.h"
#include "gpio.h"
#include "i2c.h"
#include "system.h"

/**
	* @brief Configures clocks for all peripherals used, .......... 
	* resets GPIO registers, and initializes a common I2C bus .....
	* for all external components that will use the bus.
	* @param None
	* @return None
*/
void System_Config(void)
{
	Clock_HSI_8MHz_Init();
	GPIO_Reset();
	//GPIO configuration for common I2C pins
	GPIO_OutputInit(GPIOB,
								GPIO_PORT_REG_LOW,
							 (GPIO_PIN6_OUTPUT_MODE_2MHZ | GPIO_PIN7_OUTPUT_MODE_2MHZ),
							 (GPIO_PIN6_ALT_FUNC_OPEN_DRAIN | GPIO_PIN7_ALT_FUNC_OPEN_DRAIN));
	//I2C configuration
	I2C_Init(I2C1,
					 I2C_PERIPH_FREQ_8MHZ,
					 I2C_STANDARD_MODE_8MHZ_CCR, 
					 I2C_STANDARD_MODE_8MHZ_TRISE);
}

/**
	* @brief Initiates a software reset
	* @param None
	* @return None
*/
void System_Reset(void)
{
	NVIC_SystemReset();
}

