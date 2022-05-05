#ifndef APP_H
#define APP_H

#include "stm32f10x.h"                  // Device header
#include <stdbool.h>
#include <string.h>
#include <FreeRTOS.h>
#include <task.h>
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
#include "ir_sensor.h"

//Password states
typedef enum 
{
  PASSWORD_INCORRECT = 0,
  PASSWORD_CORRECT
}pw_s;

#define BUFFER_SIZE  		  	20 //Max buffer size 
#define PSWD_EEPROMPAGE	   	0
#define PHONE_EEPROMPAGE	 	5

//Timeouts (in seconds)
#define TIMEOUT_LOCK			 	8
#define TIMEOUT_BUZZER			10
#define TIMEOUT_KEYPAD			10
#define TIMEOUT_FINGERPRINT	10
#define TIMEOUT_TAMPER			25

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
extern void IntertaskTimeout(bool* pSharedData,
														 uint8_t* tCount,
														 uint8_t timeout);

#endif //APP_H
