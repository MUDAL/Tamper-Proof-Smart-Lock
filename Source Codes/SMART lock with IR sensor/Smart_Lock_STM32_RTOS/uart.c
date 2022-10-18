#include "stm32f10x.h"                  // Device header
#include <stdbool.h>
#include "uart.h"

void USART_Init(USART_TypeDef* UARTx,
								uint32_t baud,
								uint8_t dmaMode,
								uint8_t enable)
{
	UARTx->CR1 |= USART_CR1_UE;
	switch(dmaMode)
	{
		case TX_DMA_ENABLE:
			UARTx->CR3 |= USART_CR3_DMAT;
			break;
		case RX_DMA_ENABLE:
			UARTx->CR3 |= USART_CR3_DMAR;
			break;
		case BOTH_DMA_ENABLE:
			UARTx->CR3 |= (USART_CR3_DMAT | USART_CR3_DMAR);
			break;
		case BOTH_DMA_DISABLE:
			UARTx->CR3 &= ~(USART_CR3_DMAT | USART_CR3_DMAR);
			break;
	}
	UARTx->BRR = baud;
	UARTx->CR1 |= enable;
}

void USART_WriteByte(USART_TypeDef* UARTx, uint8_t byte)
{	
	while((UARTx->SR & USART_SR_TXE) != USART_SR_TXE);
	UARTx->DR = byte;
}

void USART_WriteBytes(USART_TypeDef* UARTx, uint8_t* bytes, uint8_t len)
{
	uint8_t i = 0;
	while(i < len)
	{
		while((UARTx->SR & USART_SR_TXE) != USART_SR_TXE);
		UARTx->DR = bytes[i];
		i++;
	}
}

void USART_WriteChars(USART_TypeDef* UARTx, char* pData)
{
	uint8_t i = 0;
	while(pData[i] != '\0')
	{
		while((UARTx->SR & USART_SR_TXE) != USART_SR_TXE);
		UARTx->DR = pData[i];
		i++;
	}	
}

bool USART_RxBufferFull(USART_TypeDef* UARTx)
{
	if((UARTx->SR & USART_SR_RXNE) == USART_SR_RXNE)
	{
		return true;
	}
	return false;
}

bool USART_RxIdleLineDetected(USART_TypeDef* UARTx)
{
	bool idleLineDetected = false;
	if((UARTx->SR & USART_SR_IDLE) == USART_SR_IDLE)
	{
		idleLineDetected = true;
		volatile uint8_t readDR = UARTx->DR;
	}
	return idleLineDetected;
}
