#include "stm32f10x.h"                  // Device header
#include <stdbool.h>
#include <string.h>
#include "system.h"
#include "keypad.h"
#include "oled.h"
#include "bluetooth.h"

//Keypad, OLED, Bluetooth work
//Port the code to CubeIDE
//Use the FATFS library from CubeIDE

//Test codes
static uint8_t btRxBuffer[BT_BUFFERSIZE];


int main(void)
{
	System_Config();
	Keypad_Init();
	OLED_Init();
	OLED_ClearScreen();
	BT_Init();
	BT_RxBufferInit(btRxBuffer,BT_BUFFERSIZE);
	BT_Transmit("What're you doing at home?"); //testing BT transmit
	
	while(1)
	{
		//Testing BT receive
		btStatus_t status = BT_Receive();
		switch(status)
		{
			case NO_DATA:
				break;
			case BUFFER_FULL:
				break;
			case IDLE_LINE:
				BT_RxBufferInit(btRxBuffer,BT_BUFFERSIZE);
				System_DelayMs(5000);
				memset(btRxBuffer,'\0',BT_BUFFERSIZE);
				break;
		}
		//Testing Keypad and OLED
		char key = Keypad_GetChar();
		switch(key)
		{
			case '1':
				OLED_SetCursor(2,0);
				OLED_WriteString("Hello world",White);
				OLED_UpdateScreen();
				break;
			case '2':
				OLED_SetCursor(2,10);
				OLED_WriteString("What's for dinner?", White);
				OLED_UpdateScreen();
				break;
			case '3':
				OLED_SetCursor(2,20);
				OLED_WriteString("Store fingerprint", White);
				OLED_UpdateScreen();
				break;
			case '4':
				OLED_SetCursor(2,30);
				OLED_WriteString("Delete fingerprint", White);
				OLED_UpdateScreen();
				break;
		}
	}
}
