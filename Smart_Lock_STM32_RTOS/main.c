#include "app.h"

//Tasks
void Task1(void* pvParameters);
void Task2(void* pvParameters);
void Task3(void* pvParameters);
void Task4(void* pvParameters);

int main(void)
{
	xTaskCreate(Task1,"Init",128,NULL,1,NULL);
	vTaskStartScheduler();
	while(1)
	{
	}
}	

//Tasks
void Task1(void* pvParameters)
{
	System_Config();
	Keypad_Init();
	Button_Init();
	BT_Init();
	OutputDev_Init();
	GSM_Init();
	Fingerprint_Init();
	OLED_Init();
	OLED_ClearScreen();	
	xTaskCreate(Task2,"Bluetooth",256,NULL,1,NULL);
	xTaskCreate(Task3,"Main",256,NULL,1,NULL);
	xTaskCreate(Task4,"Buttons and IR sensor",256,NULL,1,NULL);
	vTaskDelete(NULL);
	while(1)
	{
	}
}

void Task2(void* pvParameters)
{
	static uint8_t btRxBuffer[BUFFER_SIZE];
	BT_RxBufferInit(btRxBuffer,BUFFER_SIZE);
	while(1)
	{
	}
}

void Task3(void* pvParameters)
{
	bool invalidInput = false;
	bool invalidFingerprint = false;
	uint8_t numOfInvalidPrints = 0;
	while(1)
	{
		//HMI (Keypad + OLED)
		if(!invalidInput)
		{
			char key = Keypad_GetChar();
			if((key == '*') || (key == 'A'))
			{
				char pswd[BUFFER_SIZE] = {0};
				char eepromPswd[BUFFER_SIZE] = {0};
				EEPROM_GetData((uint8_t*)eepromPswd,BUFFER_SIZE,PSWD_EEPROMPAGE);
				Display("Enter password");
				GetKeypadData(pswd);
				if(strcmp(pswd,eepromPswd) == 0)
				{
					CheckKey(key);
				}
				else
				{
					pw_s pswdState = RetryPassword(pswd,eepromPswd);
					switch(pswdState)
					{
						case PASSWORD_CORRECT:
							CheckKey(key);
							break;
						case PASSWORD_INCORRECT:
							Display("Incorrect");
							invalidInput = true;
							IntruderAlert("Intruder: Wrong inputs from Keypad!!!!!");
							break;
					}    
				}
				vTaskDelay(pdMS_TO_TICKS(1000));
				OLED_ClearScreen();
				OLED_UpdateScreen();	
			}
		}
		//Fingerprint detection
		if(!invalidFingerprint)
		{
			uint8_t f_status = FindFingerprint();
			switch(f_status)
			{
				case FINGERPRINT_OK:
					numOfInvalidPrints = 0;
					OLED_ClearScreen();
					OLED_UpdateScreen();
					OutputDev_Write(LOCK,true);
					break;
				case FINGERPRINT_NOTFOUND:
					if(numOfInvalidPrints < 2)
					{
						char msg[8] = "Retry:";
						//adding '0' converts int to char
						msg[6] = '0' + numOfInvalidPrints + 1; 
						Display(msg); 
					}
					else
					{
						invalidFingerprint = true;
						Display("Invalid\nfingerprint");
						IntruderAlert("Unregistered fingerprints detected");
						OLED_ClearScreen();
						OLED_UpdateScreen();
						numOfInvalidPrints = 0;        
					}
					numOfInvalidPrints++;
					break;
			} 
		}		
	}
}

void Task4(void* pvParameters)
{
	while(1)
	{
	}
}

