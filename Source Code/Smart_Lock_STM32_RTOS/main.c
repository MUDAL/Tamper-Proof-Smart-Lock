#include "app.h"

//Shared variable(s) 
bool invalidInput = false;
bool invalidPrint = false;
bool deviceTampered = false;
bool invalidBluetoothPswd = false;
//Function(s) called by task 2
static void HandleHMI(void);
static void HandleFingerprint(void);
//Function(s) called by task 4
static void HandleRxBluetoothData(btStatus_t bluetoothStatus,
																  uint8_t* btRxBuffer,char* eepromPswd);
//Task(s)
void Task1(void* pvParameters);
void Task2(void* pvParameters);
void Task3(void* pvParameters);
void Task4(void* pvParameters);
//Timer callback(s)
void TimerCallback(TimerHandle_t xTimer);

int main(void)
{
	System_Config();
	xTaskCreate(Task1,"",100,NULL,1,NULL); //Initializations
	vTaskStartScheduler();
	while(1)
	{
	}
}

//Functions required by task 2
void HandleHMI(void)
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
						SetIntertaskData(&invalidInput,true);
						Display("Incorrect");
						IntruderAlert("Intruder: Wrong inputs from Keypad!!!!!");
						break;
				}    
			}
			vTaskDelay(pdMS_TO_TICKS(1000));
			OLED_ClearScreen();
		}
	}	
}

void HandleFingerprint(void)
{
	static uint8_t numOfInvalidPrints;
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
					numOfInvalidPrints++;
				}
				else
				{
					SetIntertaskData(&invalidPrint,true);
					Display("Invalid\nfingerprint"); 
					IntruderAlert("Unregistered fingerprints detected");
					OLED_ClearScreen();
					numOfInvalidPrints = 0;
				}
				break;
		} 
	}			
}

//Functions required by task 4
void HandleRxBluetoothData(btStatus_t bluetoothStatus,
													 uint8_t* btRxBuffer,char* eepromPswd)
{
	static uint8_t wrongAttempts;
	if(strcmp((char*)btRxBuffer,eepromPswd) == 0)
	{//Send '0' to the app to signify reception of correct password
		BT_Transmit("0"); 
		BT_RxBufferReset(bluetoothStatus,btRxBuffer,BUFFER_SIZE);
		//Awaiting bluetooth code
		btStatus_t btStatus = NO_DATA;
		while(btStatus == NO_DATA)
		{
			btStatus = BT_Receive();
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
				//Transmit times door was opened/closed
				//Transmit time when a tamper was detected
				//Transmit time when wrong attempts were made to open the door
				break;
		}
		memset(btRxBuffer,'\0',BUFFER_SIZE);
		BT_RxBufferReset(btStatus,btRxBuffer,BUFFER_SIZE);
	}
	else
	{//Send '1' to the app to signify reception of wrong password
		BT_Transmit("1"); 
		memset(btRxBuffer,'\0',BUFFER_SIZE); //clear buffer
		BT_RxBufferReset(bluetoothStatus,btRxBuffer,BUFFER_SIZE);
		wrongAttempts++;
		if(wrongAttempts == 3)
		{
			IntruderAlert("Failed attempt via Bluetooth");
			SetIntertaskData(&invalidBluetoothPswd,true);
			wrongAttempts = 0;
		}
	}	
}

/**
	* @brief initializes all modules required by the application
	* This task is deleted after all required modules have been initialized
*/
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
	//Task(s) init
	xTaskCreate(Task2,"",300,NULL,1,NULL); //HMI and Fingerprint
	xTaskCreate(Task3,"",100,NULL,1,NULL); //Buttons, IR sensor, tamper detection
	xTaskCreate(Task4,"",300,NULL,1,NULL); //Bluetooth
	//Software timer init
	TimerHandle_t softwareTimer;
	softwareTimer = xTimerCreate("",pdMS_TO_TICKS(1000),pdTRUE,0,TimerCallback);
	xTimerStart(softwareTimer,0);	
	vTaskDelete(NULL);
	while(1)
	{
	}
}

/**
	* @brief handles the HMI (Keypad and OLED) as well as fingerprint detection
*/
void Task2(void* pvParameters)
{
	while(1)
	{
		HandleHMI();
		HandleFingerprint();
	}
}

/**
	* @brief monitors button presses and tamper detection
	* Runs every 10ms
*/
void Task3(void* pvParameters)
{
	bool indoorPrevState = false;
	bool outdoorPrevState = false;
	while(1)
	{
		vTaskDelay(pdMS_TO_TICKS(10));
		if(Button_IsPressed(INDOOR,&indoorPrevState))
		{ //Open door if closed, close if open
			bool lockState = OutputDev_Read(LOCK);
			OutputDev_Write(LOCK,!lockState);
		}
		if(Button_IsPressed(OUTDOOR,&outdoorPrevState))
		{
			OutputDev_Write(LOCK,false); //close door
		}
		//Tamper detection
		if(!deviceTampered && IRSensor_TamperDetected())
		{
			SetIntertaskData(&deviceTampered,true);
			IntruderAlert("Tamper detected!!!!!");
		}
	}
}

/**
	* @brief handles bluetooth communication between the smart lock and the app
	* Runs every 200ms
*/
void Task4(void* pvParameters)
{
	uint8_t btRxBuffer[BUFFER_SIZE] = {0};
	BT_RxBufferInit(btRxBuffer,BUFFER_SIZE);
	while(1)
	{
		vTaskDelay(pdMS_TO_TICKS(200));
		if(!invalidBluetoothPswd)
		{
			char eepromPswd[BUFFER_SIZE] = {0};
			EEPROM_GetData((uint8_t*)eepromPswd,BUFFER_SIZE,PSWD_EEPROMPAGE);
			btStatus_t bluetoothStatus = BT_Receive();
			if(bluetoothStatus != NO_DATA)
			{
				HandleRxBluetoothData(bluetoothStatus,btRxBuffer,eepromPswd);
			}
		}
		else
		{
			//If timeout hasn't been reached, ignore incoming bluetooth data
			//The timeout is due to wrong password via bluetooth
			if(strcmp((char*)btRxBuffer,"") != 0)
			{
				memset(btRxBuffer,'\0',BUFFER_SIZE);
				BT_RxBufferInit(btRxBuffer,BUFFER_SIZE);
			}
		}
	}
}

/**
	* @brief Callback function called every second by the  
	* timer daemon task to handle all software timeouts
	* required by the application.  
*/
void TimerCallback(TimerHandle_t xTimer)
{
	static uint8_t tLock;
	static uint8_t tBuzzer;
	static uint8_t tKeypadInput;
	static uint8_t tPrint;
	static uint8_t tDeviceTamper;
	static uint8_t tBluetooth;
	
	if(OutputDev_Read(LOCK))
	{
		if(HasTimedOut(&tLock,DEVICE_TIMEOUT))
		{//turn lock off if there's a timeout
			OutputDev_Write(LOCK,false);
		}
	}
	else
	{
		tLock = 0; //reset 'tLock' if door is closed by other means
	}
	if(OutputDev_Read(BUZZER) && HasTimedOut(&tBuzzer,DEVICE_TIMEOUT))
	{
		OutputDev_Write(BUZZER,false);
	}
	//Handling timeouts due to shared data between tasks
	IntertaskTimeout(&invalidInput,&tKeypadInput,DEVICE_TIMEOUT);
	IntertaskTimeout(&invalidPrint,&tPrint,DEVICE_TIMEOUT);
	IntertaskTimeout(&deviceTampered,&tDeviceTamper,TAMPER_TIMEOUT);
	IntertaskTimeout(&invalidBluetoothPswd,&tBluetooth,DEVICE_TIMEOUT);	
}
