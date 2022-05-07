#include "app.h"

//Shared variables 
bool invalidInput = false;
bool invalidPrint = false;
bool deviceTampered = false;
//Tasks
void Task1(void* pvParameters);
void Task2(void* pvParameters);
void Task3(void* pvParameters);
void Task4(void* pvParameters);
void Task5(void* pvParameters);

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
	IRSensor_Init();
	BT_Init();
	OutputDev_Init();
	GSM_Init();
	OLED_Init();
	OLED_ClearScreen();	
	Fingerprint_Init();
	xTaskCreate(Task2,"",300,NULL,1,NULL); //HMI and Fingerprint
	xTaskCreate(Task3,"",100,NULL,1,NULL); //Buttons, IR sensor, tamper detection
	xTaskCreate(Task4,"",300,NULL,1,NULL); //Bluetooth
	xTaskCreate(Task5,"",100,NULL,1,NULL); //Timeouts
	vTaskDelete(NULL);
	while(1)
	{
	}
}

void Task2(void* pvParameters)
{
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
							taskENTER_CRITICAL();
							invalidInput = true; //critical section
							taskEXIT_CRITICAL();
							Display("Incorrect");
							IntruderAlert("Intruder: Wrong inputs from Keypad!!!!!");
							break;
					}    
				}
				vTaskDelay(pdMS_TO_TICKS(1000));
				OLED_ClearScreen();
			}
		}
		//Fingerprint detection
		if(!invalidPrint)
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
						taskENTER_CRITICAL();
						invalidPrint = true; //critical section
						taskEXIT_CRITICAL();
						Display("Invalid\nfingerprint"); 
						IntruderAlert("Unregistered fingerprints detected");
						OLED_ClearScreen();
						numOfInvalidPrints = 0;
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
		vTaskDelay(pdMS_TO_TICKS(10));
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
		//Tamper detection
		if(!deviceTampered)
		{
			if(IRSensor_TamperDetected())
			{
				taskENTER_CRITICAL();
				deviceTampered = true; //critical section
				taskEXIT_CRITICAL();
				IntruderAlert("Tamper detected!!!!!");
			}
		}
	}
}

void Task4(void* pvParameters)
{
	uint8_t btRxBuffer[BUFFER_SIZE] = {0};
	BT_RxBufferInit(btRxBuffer,BUFFER_SIZE);
	while(1)
	{
		vTaskDelay(pdMS_TO_TICKS(200));
		char eepromPswd[BUFFER_SIZE] = {0};
		EEPROM_GetData((uint8_t*)eepromPswd,BUFFER_SIZE,PSWD_EEPROMPAGE);
		btStatus_t bluetoothStatus = BT_Receive();
		if(bluetoothStatus != NO_DATA)
		{
			if(strcmp((char*)btRxBuffer,eepromPswd) == 0)
			{
				BT_Transmit("0"); //Send '0' to the app to signify reception of correct password
				BT_RxBufferReset(bluetoothStatus,btRxBuffer,BUFFER_SIZE);
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
				}
				BT_RxBufferReset(btStatus,btRxBuffer,BUFFER_SIZE);
			}
			else
			{
				BT_Transmit("1"); //Send '1' to the app to signify reception of wrong password
				BT_RxBufferReset(bluetoothStatus,btRxBuffer,BUFFER_SIZE);
			}
		}
	}
}

void Task5(void* pvParameters)
{
	uint8_t tLock = 0;
	uint8_t tBuzzer = 0;
	uint8_t tKeypadInput = 0;
	uint8_t tPrint = 0;
	uint8_t tDeviceTamper = 0;
	while(1)
	{
		vTaskDelay(pdMS_TO_TICKS(1000));
		//Timeout for lock
		if(OutputDev_Read(LOCK))
		{
			if(HasTimedOut(&tLock,TIMEOUT_LOCK))
			{
				OutputDev_Write(LOCK,false);
			}
		}
		else
		{
			tLock = 0; //due to button press
		}
		//Timeout for buzzer
		if(OutputDev_Read(BUZZER))
		{
			if(HasTimedOut(&tBuzzer,TIMEOUT_BUZZER))
			{
				OutputDev_Write(BUZZER,false);
			}
		}
		//Handling timeouts due to shared data between tasks
		IntertaskTimeout(&invalidInput,&tKeypadInput,TIMEOUT_KEYPAD);
		IntertaskTimeout(&invalidPrint,&tPrint,TIMEOUT_FINGERPRINT);
		IntertaskTimeout(&deviceTampered,&tDeviceTamper,TIMEOUT_TAMPER);
	}
}
