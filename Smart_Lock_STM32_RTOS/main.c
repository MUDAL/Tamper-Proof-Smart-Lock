#include "app.h"

//Tasks
void Task_Init(void* pvParameters);
void Task_Bluetooth(void* pvParameters);
void Task_HMI(void* pvParameters);
void Task_DetectFingerprint(void* pvParameters);
void Task_DeviceTamper(void* pvParameters);

int main(void)
{
	xTaskCreate(Task_Init,"T1",256,NULL,1,NULL);
	vTaskStartScheduler();
	while(1)
	{
	}
}	

//Tasks
void Task_Init(void* pvParameters)
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
	xTaskCreate(Task_Bluetooth,"T2",256,NULL,1,NULL);
	xTaskCreate(Task_HMI,"T3",256,NULL,1,NULL);
	xTaskCreate(Task_DetectFingerprint,"T4",256,NULL,1,NULL);
	xTaskCreate(Task_DeviceTamper,"T5",256,NULL,1,NULL);
	vTaskDelete(NULL);
	while(1)
	{
	}
}

void Task_Bluetooth(void* pvParameters)
{
	static uint8_t btRxBuffer[BUFFER_SIZE];
	BT_RxBufferInit(btRxBuffer,BUFFER_SIZE);
	while(1)
	{
	}
}

void Task_HMI(void* pvParameters)
{
	while(1)
	{
	}
}

void Task_DetectFingerprint(void* pvParameters)
{
	while(1)
	{
	}
}

void Task_DeviceTamper(void* pvParameters)
{
	while(1)
	{
	}
}

