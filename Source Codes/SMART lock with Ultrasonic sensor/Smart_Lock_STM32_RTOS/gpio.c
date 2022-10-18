#include "stm32f10x.h"                  // Device header
#include <stdbool.h>
#include "gpio.h"

/**
	* Resets GPIO port configuration registers. 
	* PA13,PA14,PA15,PB2,PB3 and PB4 are left in their reset states.
*/
void GPIO_Reset(void)
{
	GPIOA->CRL &= ~0xCCCCCCCC;
	GPIOA->CRH &= ~0x000CCCCC;
	GPIOB->CRL &= ~0xCCC000CC;
	GPIOB->CRH &= ~0xCCCCCCCC;
	GPIOC->CRL &= ~0xCCCCCCCC;
	GPIOC->CRH &= ~0xCCCCCCCC;
}

void GPIO_InputInit(GPIO_TypeDef* GPIOx, 
										uint8_t portLevel, 
										uint16_t gpioPins,
										uint32_t config,  
										bool pullupEn)
											 
{
	if(portLevel == GPIO_PORT_REG_HIGH)
	{
		GPIOx->CRH |= config;
	}
	else
	{
		GPIOx->CRL |= config;
	}
	if(pullupEn)
	{
		GPIOx->ODR |= (gpioPins);
	}
}

void GPIO_OutputInit(GPIO_TypeDef* GPIOx,
										 uint8_t portLevel,
										 uint32_t mode,
										 uint32_t config)

{
	if(portLevel == GPIO_PORT_REG_HIGH)
	{
		GPIOx->CRH |= mode;
		GPIOx->CRH |= config;
	}
	else
	{
		GPIOx->CRL |= mode;
		GPIOx->CRL |= config;
	}
}

void GPIO_OutputWrite(GPIO_TypeDef* GPIOx,
											uint16_t gpioPins,
											bool gpioPinLogic)
																
{
	GPIOx->ODR &= ~gpioPins;
	if(gpioPinLogic)
	{
		GPIOx->ODR |= gpioPins;
	}
}

bool GPIO_InputRead(GPIO_TypeDef* GPIOx, uint16_t gpioPin)
{
	return ((GPIOx->IDR & gpioPin) == gpioPin);
}

bool GPIO_OutputRead(GPIO_TypeDef* GPIOx, uint16_t gpioPin)
{
	return ((GPIOx->ODR & gpioPin) == gpioPin);
}
	
