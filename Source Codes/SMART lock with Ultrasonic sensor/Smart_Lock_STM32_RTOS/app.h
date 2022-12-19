#ifndef APP_H
#define APP_H

#include "stm32f10x.h"                  // Device header
#include <stdbool.h>
#include <string.h>
#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>
#include "system.h"
#include "keypad.h"
#include "oled.h"
#include "bluetooth.h"
#include "button.h"
#include "output_device.h"
#include "fingerprint.h"
#include "eeprom.h"
#include "gsm.h"
#include "rtc.h"
#include "sensor.h"

//Password states
typedef enum 
{
  PASSWORD_INCORRECT = 0,
  PASSWORD_CORRECT
}pw_s;

#define BUFFER_SIZE  		  				20 //Max buffer size 
#define PSWD_EEPROMPAGE	   				0
#define PHONE_EEPROMPAGE	 				5
#define NUM_OF_SECURITY_REPORTS		5 //Max number of security reports
#define END_OF_REPORT							"\r"
//Timeouts (in seconds)
#define DEVICE_TIMEOUT						8

extern void Display(char* msg);
extern void GetKeypadData(char* keyBuffer);
extern pw_s RetryPassword(char* keyBuffer,char* password);
extern void InputNewPassword(void);
extern void InputPhoneNumber(void);
extern void IntruderAlert(char* msg);
extern void CheckKey(char key);
extern void StoreFingerprint(void);
extern uint8_t FindFingerprint(void);
extern bool HasTimedOut(uint8_t* tCount,uint8_t timeout);
extern void SetIntertaskData(bool* pSharedData,bool state);
extern void IntertaskTimeout(bool* pSharedData,
														 uint8_t* tCount,
														 uint8_t timeout);
extern void IntegerToString(uint32_t integer, char* pBuffer);

#endif //APP_H
