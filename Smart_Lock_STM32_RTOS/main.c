#include "app.h"

//Tasks
void Task1(void* pvParameters);
void Task2(void* pvParameters);
void Task3(void* pvParameters);
//void Task4(void* pvParameters);

int main(void)
{
	System_Config();
	xTaskCreate(Task1,"",100,NULL,1,NULL); //Initializations
	vTaskStartScheduler();
	while(1)
	{
	}
}	

//Tasks
void Task1(void* pvParameters)
{
	Keypad_Init();
	Button_Init();
	BT_Init();
	OutputDev_Init();
	GSM_Init();
	OLED_Init();
	OLED_ClearScreen();	
	Fingerprint_Init();
	xTaskCreate(Task2,"",300,NULL,1,NULL); //HMI and Fingerprint
	xTaskCreate(Task3,"",100,NULL,1,NULL); //Buttons, IR sensor, tamper detection
	//xTaskCreate(Task4,"",300,NULL,1,NULL); //Bluetooth
	vTaskDelete(NULL);
	while(1)
	{
	}
}

void Task2(void* pvParameters)
{
	bool invalidInput = false;
	bool invalidFingerprint = false;
	bool prevPressed[4][4] = {0};
	uint8_t numOfInvalidPrints = 0;
	while(1)
	{
		//HMI (Keypad + OLED)
		if(!invalidInput)
		{
			char key = Keypad_GetChar(prevPressed);
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
						OLED_ClearScreen();
						numOfInvalidPrints = 0; 
						IntruderAlert("Unregistered fingerprints detected");
					}
					numOfInvalidPrints++;
					break;
			} 
		}		
	}
}

void Task3(void* pvParameters)
{
	bool indoorPrevState = false;
	bool outdoorPrevState = false;
	while(1)
	{
		if(Button_IsPressed(INDOOR,&indoorPrevState))
		{
			//Open door if closed, close if open
			bool lockState = OutputDev_Read(LOCK);
			OutputDev_Write(LOCK,!lockState);
		}
		if(Button_IsPressed(OUTDOOR,&outdoorPrevState))
		{
			OutputDev_Write(LOCK,false); //close door
		}
	}
}

/*void Task4(void* pvParameters)
{
	uint8_t btRxBuffer[BUFFER_SIZE] = {0};
	BT_RxBufferInit(btRxBuffer,BUFFER_SIZE);
	while(1)
	{
		char eepromPswd[BUFFER_SIZE] = {0};
		EEPROM_GetData((uint8_t*)eepromPswd,BUFFER_SIZE,PSWD_EEPROMPAGE);
		btStatus_t bluetoothStatus = BT_Receive();
		if(bluetoothStatus != NO_DATA)
		{
			if(strcmp((char*)btRxBuffer,eepromPswd) == 0)
			{
				BT_Transmit("\nSmart lock bluetooth codes:\n" 
                    "0. To open the door\n"
                    "1. To close the door\n"
                    "2. To get security report\n"
                    "3. To set the time\n");
				//Reset buffer if IDLE line is detected
				if(bluetoothStatus == IDLE_LINE)
				{
					BT_RxBufferInit(btRxBuffer,BUFFER_SIZE); 
				}
				//Awaiting bluetooth code
				btStatus_t btStatus = NO_DATA;
				while(1)
				{
					btStatus = BT_Receive();
					if(btStatus != NO_DATA)
					{
						break;
					}
				}
				//Check which code was received
				switch(btRxBuffer[0])
				{
					case '0':
						OutputDev_Write(LOCK,true);
						break;
					case '1':
						OutputDev_Write(LOCK,false);
						break;
					case '2':
						break;
					case '3':
						break;
				}
				//Reset buffer if IDLE line is detected
				if(btStatus == IDLE_LINE)
				{
					BT_RxBufferInit(btRxBuffer,BUFFER_SIZE); 
				}
			}
		}
	}
}
*/
