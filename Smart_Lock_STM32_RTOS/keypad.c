#include "stm32f10x.h"                  // Device header
#include "gpio.h"
#include <FreeRTOS.h>
#include <task.h>
#include "keypad.h"

static bool IsDebounced(uint8_t colIndex)
{
	uint16_t colPin;
	switch(colIndex)
	{
		case 0:
			colPin = GPIO_PIN9;
			break;
		case 1:
			colPin = GPIO_PIN12;
			break;
		case 2:
			colPin = GPIO_PIN13;
			break;
		case 3:
			colPin = GPIO_PIN14;
			break;
	}
	if(!GPIO_InputRead(GPIOB,colPin))
	{
		vTaskDelay(pdMS_TO_TICKS(15)); //De-bounce
		if(!GPIO_InputRead(GPIOB,colPin))
		{
			return true;
		}
	}
	return false;
}	

static void SelectRow(uint8_t rowIndex)
{
	switch(rowIndex)
	{
		case 0:
			//clear PB0, set the rest
			GPIO_OutputWrite(GPIOB,GPIO_PIN0,false);
			GPIO_OutputWrite(GPIOB,(GPIO_PIN1 | GPIO_PIN5 | GPIO_PIN8),true);
			break;
		case 1:
			//clear PB1, set the rest
			GPIO_OutputWrite(GPIOB,GPIO_PIN1,false);
			GPIO_OutputWrite(GPIOB,(GPIO_PIN0 | GPIO_PIN5 | GPIO_PIN8),true);
			break;
		case 2:
			//clear PB5, set the rest
			GPIO_OutputWrite(GPIOB,GPIO_PIN5,false);
			GPIO_OutputWrite(GPIOB,(GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN8),true);
			break;
		case 3:
			//clear PB8, set the rest
			GPIO_OutputWrite(GPIOB,GPIO_PIN8,false);
			GPIO_OutputWrite(GPIOB,(GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN5),true);
			break;
	}
}
 
void Keypad_Init(void)
{
	//Row pins as output [PB0,1,5,8]
	GPIO_OutputInit(GPIOB,
									GPIO_PORT_REG_LOW,
									(GPIO_PIN0_OUTPUT_MODE_2MHZ | 
									 GPIO_PIN1_OUTPUT_MODE_2MHZ |
									 GPIO_PIN5_OUTPUT_MODE_2MHZ |
									 GPIO_PIN8_OUTPUT_MODE_2MHZ),
									 GPIO_GEN_PUR_OUTPUT_PUSH_PULL);
	GPIO_OutputInit(GPIOB,
									GPIO_PORT_REG_HIGH,
									GPIO_PIN8_OUTPUT_MODE_2MHZ,
									GPIO_GEN_PUR_OUTPUT_PUSH_PULL);
	//Column pins as input [PB9,12,13,14]
	GPIO_InputInit(GPIOB,
								 GPIO_PORT_REG_HIGH,
								 (GPIO_PIN9 | GPIO_PIN12 | GPIO_PIN13 | GPIO_PIN14),
								 (GPIO_PIN9_INPUT_PULLUP_OR_PULLDOWN  |
									GPIO_PIN12_INPUT_PULLUP_OR_PULLDOWN | 
									GPIO_PIN13_INPUT_PULLUP_OR_PULLDOWN |
									GPIO_PIN14_INPUT_PULLUP_OR_PULLDOWN),
									GPIO_PULLUP_ENABLE);
}

char Keypad_GetChar(bool prevPressed[4][4])
{
	char keypadMatrix[4][4] =
	{{'1','2','3','A'},
	 {'4','5','6','B'},
	 {'7','8','9','C'},
	 {'*','0','#','D'}
	};

	for(uint8_t i = 0; i < 4; i++)
	{
		SelectRow(i);
		for(uint8_t j = 0; j < 4; j++)
		{
			if(IsDebounced(j) && !prevPressed[i][j])
			{
				prevPressed[i][j] = true;
				return keypadMatrix[i][j];
			}
			else if(!IsDebounced(j) && prevPressed[i][j])
			{
				prevPressed[i][j] = false;
			}
		}
	}
	return '\0';
}
