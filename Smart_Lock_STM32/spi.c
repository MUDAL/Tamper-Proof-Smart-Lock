#include "stm32f10x.h"                  // Device header
#include "spi.h"

void SPI_Init(SPI_TypeDef* spiPort, uint32_t config)
{
	spiPort->CR1 |= config;
}
	
void SPI_WriteByte(SPI_TypeDef* spiPort, uint8_t* pByte)
{
	//wait for transmit buffer to be empty
	while((spiPort->SR & SPI_SR_TXE) != SPI_SR_TXE);
	spiPort->DR = *pByte;
}
	
void SPI_WriteBytes(SPI_TypeDef* spiPort, uint8_t* pData, uint16_t length)
{
	for(uint16_t i = 0; i < length; i++)
	{
		//wait for transmit buffer to be empty
		while((spiPort->SR & SPI_SR_TXE) != SPI_SR_TXE); 
		spiPort->DR = pData[i];
	}
}

uint8_t SPI_ReceiveByte(SPI_TypeDef* spiPort)
{
	//wait for receive buffer to contain data
	while((spiPort->SR & SPI_SR_RXNE) != SPI_SR_RXNE); 
	return spiPort->DR;
}

void SPI_Transceive(SPI_TypeDef* spiPort, uint8_t* txData, uint8_t* rxData)
{
	//wait for transmit buffer to be empty
	while((spiPort->SR & SPI_SR_TXE) != SPI_SR_TXE); 
	spiPort->DR = *txData;
	//wait for receive buffer to contain data
	while((spiPort->SR & SPI_SR_RXNE) != SPI_SR_RXNE); 
	*rxData = spiPort->DR;
}

void SPI_ChipSelect(GPIO_TypeDef* nssGpioPort, uint8_t nssGpioPin)
{
	nssGpioPort->ODR &= ~(1<<nssGpioPin);
}

void SPI_ChipDeselect(GPIO_TypeDef* nssGpioPort, uint8_t nssGpioPin)
{
	nssGpioPort->ODR |= (1<<nssGpioPin);
}
