#include "stm32f10x.h"                  // Device header
#include "gpio.h"
#include "uart.h"
#include "dma.h"
#include "bluetooth.h"

void BT_Init(void)
{
	//GPIO configuration for USART3 Tx
	GPIO_OutputInit(GPIOB,
									GPIO_PORT_REG_HIGH,
									GPIO_PIN10_OUTPUT_MODE_2MHZ,
									GPIO_PIN10_ALT_FUNC_PUSH_PULL);
	//GPIO configuration for USART3 Rx
	GPIO_InputInit(GPIOB,
								 GPIO_PORT_REG_HIGH,
								 GPIO_PIN11,
								 GPIO_PIN11_INPUT_PULLUP_OR_PULLDOWN,
								 GPIO_PULLUP_ENABLE);	
	USART_Init(USART3,BAUD_9600,RX_DMA_ENABLE,
						(USART_TX_ENABLE | USART_RX_ENABLE));	
}

void BT_RxBufferInit(uint8_t* pBuffer,uint8_t bufferSize)
{
	//DMA1 channel 3 configuration for USART3 Rx
	DMA_USART_Rx_Init(DMA1_Channel3,
										USART3,
										pBuffer,
										bufferSize, 
										DMA_CHANNEL3_MEMORY_INC_MODE |
										DMA_CHANNEL_ENABLE);	
}

void BT_RxBufferReset(btStatus_t status,uint8_t* pBuffer,uint8_t bufferSize)
{
	if(status == IDLE_LINE)
	{
		BT_RxBufferInit(pBuffer,bufferSize);
	}
}

void BT_Transmit(char* pData)
{
	USART_WriteChars(USART3,pData);
}

btStatus_t BT_Receive(void)
{
	btStatus_t status = NO_DATA;
	if(DMA_RxBufferFull(DMA1,DMA_CHANNEL3))
	{
		status = BUFFER_FULL;
	}
	if(USART_RxIdleLineDetected(USART3))
	{
		status = IDLE_LINE;
	}
	return status;
}
