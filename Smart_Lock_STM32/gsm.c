#include "stm32f10x.h"                  // Device header
#include "gpio.h"
#include "uart.h"
#include "systick.h"
#include "gsm.h"
#include <string.h>

void GSM_Init(void)
{
	//GPIO configuration for USART2 Tx pin
	GPIO_OutputInit(GPIOA,
									GPIO_PORT_REG_LOW,
									GPIO_PIN2_OUTPUT_MODE_2MHZ,
									GPIO_PIN2_ALT_FUNC_PUSH_PULL);
	//USART 2 Tx configuration
	USART_Init(USART2,BAUD_9600,BOTH_DMA_DISABLE,USART_TX_ENABLE);	
}

void GSM_SendText(char* phoneNo,char* msg)
{
	char atCmgsCmd[27] = "AT+CMGS=\"";
	USART_WriteChars(USART2,"AT+CMGF=1");
	SysTick_DelayMs(500);
	strcat(atCmgsCmd,phoneNo);
	strcat(atCmgsCmd,"\"\r\n");
	USART_WriteChars(USART2,atCmgsCmd);
	SysTick_DelayMs(500);
	USART_WriteChars(USART2,msg);
	SysTick_DelayMs(500);
	USART_WriteByte(USART2,26); //command termination
	SysTick_DelayMs(500);
}
