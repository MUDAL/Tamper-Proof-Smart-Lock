#include "stm32f10x.h"                  // Device header
#include <stdbool.h>
#include <string.h>
#include "system.h"
#include "keypad.h"
#include "oled.h"
#include "bluetooth.h"
#include "button.h"
#include "output_device.h"
#include "fingerprint.h"
#include "eeprom.h"
#include "state.h"
#include "gsm.h"
//SD card
#include "diskio.h"
#include "sd_card.h"
#include "ff.h"

#define BUFFER_SIZE  		  20 //Max buffer size 
#define PSWD_EEPROMPAGE	  0
#define PHONE_EEPROMPAGE	5

//Password states
typedef enum 
{
  PASSWORD_INCORRECT = 0,
  PASSWORD_CORRECT
}pw_s;

//Functions
void Display(char* msg);
void GetKeypadData(char* keyBuffer);
pw_s RetryPassword(char* keyBuffer,char* password);
void InputNewPassword(void);
void InputPhoneNumber(void);
void IntruderAlert(char* msg);
void CheckKey(char key);
void HMI(char key);
void StoreFingerprint(void);
uint8_t FindFingerprint(void);

int main(void)
{
	static uint8_t btRxBuffer[BUFFER_SIZE];
	//static char sdBuffer[BUFFER_SIZE];
	//static FATFS fs; //file system
	//static FIL fil; //file
	
	System_Config();
	Keypad_Init();
	OLED_Init();
	OLED_ClearScreen();
	BT_Init();
	BT_RxBufferInit(btRxBuffer,BUFFER_SIZE);
	Button_Init();
	OutputDev_Init();
	SD_Init();
	
	//SD
	/*Mount SD card*/
	//f_mount(&fs, "", 0);
	/*Open file to read file*/
  //f_open(&fil,"pn.txt",FA_READ);
	//Read
	//f_gets(sdBuffer,20,&fil);
	//close file
	//f_close(&fil);
	
	while(1)
	{
		//Bluetooth
		
		//HMI (Keypad + OLED)
		if(!GetState(FAILED_INPUT))
		{
			char key = Keypad_GetChar();
			if((key == '*') || (key == 'A'))
			{
				HMI(key);
			}
		}
		//Fingerprint detection
		
	}
}

void Display(char* msg)
{
	const uint8_t xPos = 2;
	const uint8_t yPos = 10;
	uint8_t y = yPos;
	
	OLED_ClearScreen();
	OLED_SetCursor(xPos,y);
	uint8_t i = 0;
	while(msg[i] != '\0')
	{
		if(msg[i] == '\n')
		{
			y += 10; //+10 pixels for newline 
			OLED_SetCursor(xPos,y);
		}
		OLED_WriteChar(msg[i],White);
		i++;
	}
	OLED_UpdateScreen();
}

void GetKeypadData(char* keyBuffer)
{
  uint8_t i = 0;
  while(1)
  {
    char key = Keypad_GetChar();
    switch(key)
    {
      case '\0':
        break;
      case '#':
        keyBuffer[i] = '\0';
        return;
      default:
        if(i < BUFFER_SIZE)
        {
          keyBuffer[i] = key;
          i++;
        }
        break;
    }
  }  
}

pw_s RetryPassword(char* keyBuffer,char* password)
{
	char errorMsg[18] = "Incorrect\nRetry:";
	char nTries;
  pw_s passwordState = PASSWORD_INCORRECT;
  uint8_t retry = 1;
	
  while(retry <= 2)
  {
		nTries = retry + '0'; //adding '0' converts int to char
		errorMsg[16] = nTries; //number of attempts
		Display(errorMsg);
    GetKeypadData(keyBuffer);
    retry++;
    if(strcmp(keyBuffer,password) == 0)
    {
      passwordState = PASSWORD_CORRECT;
      break;
    }
  }
  return passwordState;   	
}

void InputNewPassword(void)
{
  char pswd[BUFFER_SIZE] = {0};
  char newPassword[BUFFER_SIZE] = {0};
  GetKeypadData(pswd);
  strcpy(newPassword,pswd);
  Display("Reenter new\npassword");
  GetKeypadData(pswd);
  if(strcmp(pswd,newPassword) == 0)
  {
    Display("New password\ncreated");
    EEPROM_StoreData((uint8_t*)pswd,BUFFER_SIZE,PSWD_EEPROMPAGE);
  }
  else
  {
    pw_s pswdState = RetryPassword(pswd,newPassword);
    switch(pswdState)
    {
      case PASSWORD_CORRECT:
        Display("New password\ncreated");
        EEPROM_StoreData((uint8_t*)pswd,BUFFER_SIZE,PSWD_EEPROMPAGE);
        break;
      case PASSWORD_INCORRECT:
        Display("Could not create");
        break;
    } 
  } 	
}

void InputPhoneNumber(void)
{
	const char countryCode[] = "+234";
 	char phoneNumber[15] = {0};
	char displayMsg[30] = "Phone number:\n";
	GetKeypadData(phoneNumber+3);
	//Insert country code
	for(uint8_t i = 0; i < 4; i++)
	{
		phoneNumber[i] = countryCode[i];
	}
	strcat(displayMsg,phoneNumber);
	Display(displayMsg);
	EEPROM_StoreData((uint8_t*)phoneNumber,BUFFER_SIZE,PHONE_EEPROMPAGE);
}

void IntruderAlert(char* msg)
{
	char phoneNumber[15] = {0};
	OutputDev_Write(BUZZER,true);
	EEPROM_GetData((uint8_t*)phoneNumber,BUFFER_SIZE,PHONE_EEPROMPAGE);
	GSM_SendText(phoneNumber,msg);
}

void CheckKey(char key)
{
  if(key == '*')
  {
    Display("Correct");
		OutputDev_Write(LOCK,true);
  }
  else if(key == 'A')
  {
    Display("Correct. Press:\n"
            "0.New password\n"
            "1.Phone number\n"
            "2.Store print\n"
            "3.Delete prints\n");
    char getKey = '\0';
    while(getKey != 'B')
    {
      getKey = Keypad_GetChar();
      if(getKey == '0')
      {
        Display("Enter new password");
        InputNewPassword();
        break;
      }
      else if(getKey == '1')
      {
        Display("Enter phone number");
        InputPhoneNumber();
        break;
      }
      else if(getKey == '2')
      {
        Display("Place finger"); 
        StoreFingerprint();
        break;
      }
      else if(getKey == '3')
      {
        Fingerprint_EmptyDatabase();
        Display("Database cleared");
        System_DelayMs(1000);
        OLED_ClearScreen();
        OLED_UpdateScreen();
        break;
      }
    }
  }
}

void HMI(char key)
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
        SetState(FAILED_INPUT,true);
        IntruderAlert("Intruder: Wrong inputs from Keypad!!!!!");
        break;
    }    
  }
  System_DelayMs(1000);
  OLED_ClearScreen();
  OLED_UpdateScreen();	
}

void StoreFingerprint(void)
{
  uint8_t id = 0;
  //Searching for free slot to store the fingerprint template
  for(uint8_t i = 1; i <= 127; i++)
  {
    if(Fingerprint_LoadModel(i) != FINGERPRINT_OK)
    {
      id = i;
      break;
    }
  }
  if(id == 0)
  {
    Display("Memory full");
    System_DelayMs(1000);
    OLED_ClearScreen();
    OLED_UpdateScreen();
    return; //Exit 
  }
  while(Fingerprint_GetImage() != FINGERPRINT_OK){}
  while(Fingerprint_Image2Tz(1) != FINGERPRINT_OK){}
  Display("Remove finger then\nplace same finger\nagain");
  while(Fingerprint_GetImage() != FINGERPRINT_OK){}
  while(Fingerprint_Image2Tz(2) != FINGERPRINT_OK){} 
  //Creating model for fingerprint
  if(Fingerprint_CreateModel() == FINGERPRINT_OK) 
  {
    if(Fingerprint_StoreModel(id) == FINGERPRINT_OK) 
    {
      Display("Fingerprint\nstored!");
    } 
  } 
  else
  {
    Display("Fingerprints\nunmatched!");
  }
  System_DelayMs(1000);
  OLED_ClearScreen();
  OLED_UpdateScreen();	
}

uint8_t FindFingerprint(void)
{
  uint8_t f_status = Fingerprint_GetImage();
  if(f_status != FINGERPRINT_OK)
  {
    return f_status;
  }
  f_status = Fingerprint_Image2Tz(1);
  if(f_status != FINGERPRINT_OK) 
  { 
    return f_status;
  }
  f_status = Fingerprint_FingerFastSearch();
  if(f_status != FINGERPRINT_OK)  
  {
    return f_status;
  }
  return f_status;	
}
