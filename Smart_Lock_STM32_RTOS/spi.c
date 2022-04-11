#include "stm32f10x.h"                  // Device header
#include "spi.h"

void SPI_Init(SPI_TypeDef* SPIx, uint32_t config)
{
	SPIx->CR1 |= config;
}
	
void SPI_WriteByte(SPI_TypeDef* SPIx, uint8_t* pByte)
{
	//wait for transmit buffer to be empty
	while((SPIx->SR & SPI_SR_TXE) != SPI_SR_TXE);
	SPIx->DR = *pByte;
}
	
void SPI_WriteBytes(SPI_TypeDef* SPIx, uint8_t* pData, uint16_t length)
{
	for(uint16_t i = 0; i < length; i++)
	{
		//wait for transmit buffer to be empty
		while((SPIx->SR & SPI_SR_TXE) != SPI_SR_TXE); 
		SPIx->DR = pData[i];
	}
}

uint8_t SPI_ReceiveByte(SPI_TypeDef* SPIx)
{
	//wait for receive buffer to contain data
	while((SPIx->SR & SPI_SR_RXNE) != SPI_SR_RXNE); 
	return SPIx->DR;
}

void SPI_Transceive(SPI_TypeDef* SPIx, uint8_t* txData, uint8_t* rxData)
{
	//wait for transmit buffer to be empty
	while((SPIx->SR & SPI_SR_TXE) != SPI_SR_TXE); 
	SPIx->DR = *txData;
	//wait for receive buffer to contain data
	while((SPIx->SR & SPI_SR_RXNE) != SPI_SR_RXNE); 
	*rxData = SPIx->DR;
}

void SPI_ChipSelect(GPIO_TypeDef* nssGpioPort, uint8_t nssGpioPin)
{
	nssGpioPort->ODR &= ~(1<<nssGpioPin);
}

void SPI_ChipDeselect(GPIO_TypeDef* nssGpioPort, uint8_t nssGpioPin)
{
	nssGpioPort->ODR |= (1<<nssGpioPin);
}
