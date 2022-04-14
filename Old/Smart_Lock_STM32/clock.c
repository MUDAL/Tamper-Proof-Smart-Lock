#include "stm32f10x.h"                  // Device header
#include "clock.h"

void Clock_HSI_8MHz_Init(void)
{
	while((RCC->CR & RCC_CR_HSIRDY) != RCC_CR_HSIRDY); //wait for internal oscillator to be stable
	RCC->CFGR = 0; //Reset CFGR
	RCC->CR &= ~(RCC_CR_PLLON | RCC_CR_HSEON);	//disable PLL and external oscillator
	//enable clock for DMA1
	RCC->AHBENR |= RCC_AHBENR_DMA1EN;
	//enable clock access for GPIOA,GPIOB,GPIOC,USART1,SPI1
	RCC->APB2ENR |= (RCC_APB2ENR_IOPAEN 	| 
								   RCC_APB2ENR_IOPBEN 	| 
									 RCC_APB2ENR_IOPCEN   |
									 RCC_APB2ENR_USART1EN |
									 RCC_APB2ENR_SPI1EN);
	//enable clock for I2C1,USART2,USART3
	RCC->APB1ENR |= (RCC_APB1ENR_I2C1EN   |
									 RCC_APB1ENR_USART2EN |
									 RCC_APB1ENR_USART3EN);
}
