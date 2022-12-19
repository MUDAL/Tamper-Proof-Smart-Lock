#include "app.h"
//Enum(s)
//Bluetooth commands/requests from the app to the smart lock
enum AppCmd
{
	OPEN_DOOR = '0',
	CLOSE_DOOR = '1',
	SEND_REPORT = '2'
};
//Type of security event
typedef enum
{
	TAMPER_DETECTION_EVENT = 0,
	FAILED_ACCESS_EVENT
}securityEvent_t;

//Shared variable(s) 
bool invalidInput = false;
bool invalidPrint = false;
bool deviceTampered = false;
bool invalidBluetoothPswd = false;
static ds3231_t tamperDetectionTimes[NUM_OF_SECURITY_REPORTS];
static ds3231_t failedAccessTimes[NUM_OF_SECURITY_REPORTS];

//Function(s)
static void HandleHMI(void);
static void HandleFingerprint(void);
static void StoreSecurityTimestamp(securityEvent_t timestamp);
static void SendSecurityReportToApp(char* reportName,ds3231_t* timeOfReport);
static void HandleRxBluetoothData(btStatus_t bluetoothStatus,
																	uint8_t* btRxBuffer,char* eepromPswd);
//Tasks and callbacks
void SetupTask(void* pvParameters);
void HMI_FingerprintTask(void* pvParameters);
void Button_TamperTask(void* pvParameters);
void BluetoothTask(void* pvParameters);
void TimerCallback(TimerHandle_t xTimer);
 
int main(void)
{
	System_Config();
	xTaskCreate(SetupTask,"",100,NULL,1,NULL); //Initializations
	vTaskStartScheduler();
	while(1)
	{
	}
}

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
						StoreSecurityTimestamp(FAILED_ACCESS_EVENT);
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
	if(!invalidPrint)
	{
		uint8_t fingerprintStatus = FindFingerprint();
		switch(fingerprintStatus)
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
					StoreSecurityTimestamp(FAILED_ACCESS_EVENT);
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

/**
	* @brief Store time at which security event took place.  
	* Security event could be 'tamper detection' or 'failed
	* access' due to incorrect password (keypad or app) or
	* incorrect fingerprint.  
*/
void StoreSecurityTimestamp(securityEvent_t secEvent)
{
	static uint8_t tamperDetectionCounter;
	static uint8_t failedAccessCounter;
	switch(secEvent)
	{
		case TAMPER_DETECTION_EVENT:
			RTC_GetTime(&tamperDetectionTimes[tamperDetectionCounter]);
			tamperDetectionCounter++;
			tamperDetectionCounter %= NUM_OF_SECURITY_REPORTS;
			break;
		case FAILED_ACCESS_EVENT:
			RTC_GetTime(&failedAccessTimes[failedAccessCounter]);
			failedAccessCounter++;
			failedAccessCounter %= NUM_OF_SECURITY_REPORTS;
			break;
	}
}

void SendSecurityReportToApp(char* reportName,ds3231_t* timeOfReport)
{
	BT_Transmit(reportName);
	for(uint8_t i = 0; i < NUM_OF_SECURITY_REPORTS; i++)
	{
		char hour[3] = {0};
		char minute[3] = {0};
		//convert time of report (hour and min) to string then transmit to the app
		IntegerToString(timeOfReport[i].hours,hour);
		if(timeOfReport[i].hours < 10)
		{
			BT_Transmit("0");
			BT_Transmit(hour);
		}
		else
		{
			BT_Transmit(hour);
		}
		
		BT_Transmit(":");
		
		IntegerToString(timeOfReport[i].minutes,minute);
		if(timeOfReport[i].minutes < 10)
		{
			BT_Transmit("0");
			BT_Transmit(minute);
		}
		else
		{
			BT_Transmit(minute);
		}
		BT_Transmit(" "); //to separate the times
	}
}

void HandleRxBluetoothData(btStatus_t bluetoothStatus,
													 uint8_t* btRxBuffer,char* eepromPswd)
{
	const uint8_t maxNumOfWrongAttempts = 3;
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
			case OPEN_DOOR:
				OutputDev_Write(LOCK,true);
				break;
			case CLOSE_DOOR:
				OutputDev_Write(LOCK,false);
				break;
			case SEND_REPORT:
				//Transmit time when a tamper was detected
				SendSecurityReportToApp("Tamper detected-> ",tamperDetectionTimes);
				BT_Transmit("\n\n");
				//Transmit time when wrong attempts were made to open the door
				SendSecurityReportToApp("Failed access-> ",failedAccessTimes);
				BT_Transmit(END_OF_REPORT); //end of data transmission to app
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
		if(wrongAttempts == maxNumOfWrongAttempts)
		{
			StoreSecurityTimestamp(FAILED_ACCESS_EVENT);
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
void SetupTask(void* pvParameters)
{
	Keypad_Init();
	Button_Init();
	Sensor_Init();
	BT_Init();
	OutputDev_Init();
	GSM_Init();
	OLED_Init();
	OLED_ClearScreen();	
	Fingerprint_Init();
	//Task(s) init
	xTaskCreate(HMI_FingerprintTask,"",300,NULL,1,NULL); //HMI and Fingerprint
	xTaskCreate(Button_TamperTask,"",100,NULL,1,NULL); //Buttons, sensor, tamper detection
	xTaskCreate(BluetoothTask,"",300,NULL,1,NULL); //Bluetooth
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
void HMI_FingerprintTask(void* pvParameters)
{
	while(1)
	{
		HandleHMI();
		HandleFingerprint();
	}
}

/**
	* @brief monitors button presses and tamper detection
*/
void Button_TamperTask(void* pvParameters)
{
	bool indoorPrevState = false;
	bool outdoorPrevState = false;
	while(1)
	{
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
		if(!deviceTampered && Sensor_GetDistance() > 6)
		{
			StoreSecurityTimestamp(TAMPER_DETECTION_EVENT);
			SetIntertaskData(&deviceTampered,true);
			IntruderAlert("Tamper detected!!!!!");
		}
		vTaskDelay(pdMS_TO_TICKS(10));
	}
}

/**
	* @brief handles bluetooth communication between the smart lock and the app
*/
void BluetoothTask(void* pvParameters)
{
	uint8_t btRxBuffer[BUFFER_SIZE] = {0};
	BT_RxBufferInit(btRxBuffer,BUFFER_SIZE);
	while(1)
	{
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
		vTaskDelay(pdMS_TO_TICKS(200));
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
	
	if(OutputDev_Read(BUZZER))
	{
		if(HasTimedOut(&tBuzzer,DEVICE_TIMEOUT))
		{
			OutputDev_Write(BUZZER,false);
		}
	}
	//Handling timeouts due to shared data between tasks
	IntertaskTimeout(&invalidInput,&tKeypadInput,DEVICE_TIMEOUT);
	IntertaskTimeout(&invalidPrint,&tPrint,DEVICE_TIMEOUT);
	IntertaskTimeout(&deviceTampered,&tDeviceTamper,DEVICE_TIMEOUT);
	IntertaskTimeout(&invalidBluetoothPswd,&tBluetooth,DEVICE_TIMEOUT);	
}
