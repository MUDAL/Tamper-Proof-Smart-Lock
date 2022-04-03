#include "stm32f10x.h"                  // Device header
#include <stdbool.h>
#include <string.h>
#include "system.h"
#include "keypad.h"
#include "oled.h"
#include "bluetooth.h"
#include "button.h"
#include "output_device.h"
//SD card
#include "diskio.h"
#include "sd_card.h"
#include "ff.h"

//Keypad, OLED, SD, Bluetooth, BUzzer, Lock, Buttons work

//Test codes
static uint8_t btRxBuffer[BT_BUFFERSIZE];
//SD
FATFS fs; //file system
FIL fil; //file
//FRESULT fresult; //to store the result
uint32_t fresult;
char buffer[1024]; //to store data

//Test code for button
uint8_t indoor = 0;
uint8_t outdoor = 0;

int main(void)
{
	System_Config();
	Keypad_Init();
	OLED_Init();
	OLED_ClearScreen();
	BT_Init();
	BT_RxBufferInit(btRxBuffer,BT_BUFFERSIZE);
	BT_Transmit("What're you doing at home?"); //testing BT transmit
	Button_Init();
	OutputDev_Init();
	
	//SD
	SD_Init();
	/*Mount SD card*/
	fresult = f_mount(&fs, "", 0);
	/*Open file to read file*/
  fresult = f_open(&fil,"pn.txt",FA_READ);
	//Read
	f_gets(buffer,30,&fil);
	//close file
	fresult = f_close(&fil);
	
	//Buzzer test
	//OutputDev_Write(BUZZER,true);
	//System_DelayMs(3000);
	//OutputDev_Write(BUZZER,false);
	
	//Lock test
	OutputDev_Write(LOCK,true);
	System_DelayMs(5000);
	OutputDev_Write(LOCK,false);
	
	while(1)
	{
		//Testing buttons
		if(Button_IsPressedOnce(INDOOR))
		{
			indoor++;
		}
		if(Button_IsPressedOnce(OUTDOOR))
		{
			outdoor++;
		}
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
