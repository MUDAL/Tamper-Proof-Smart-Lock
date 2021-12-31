#include <Wire.h>
#include "RTClib.h" //Version 1.3.3
#include "sd_card.h"
#include "communication.h"
#include "keypad.h"

/*
 * Components:
 * ESP32 inbuilt bluetooth --> Bluetooth serial
 * Keypad --> GPIO
 * SD card [+3.3v power] --> SPI
 * RTC module [+3.3v power] --> I2C
 * GSM module [External 4.3v power] --> UART
 * Fingerprint scanner --> UART
 * Button(s)
 * LED(s)
 * Buzzer
 * Electromagnetic lock
 * 
 * Helpful libraries:
 * Wire.h
 * RTClib.h
 * DateTime
 * SPI.h
 * SD.h
 * FS.h
 * BluetoothSerial.h
*/

//Password states
enum 
{
  DEF = 0, //Default
  PASSWORD_CORRECT,
  PASSWORD_INCORRECT
};
//Buffers
char keypadBuffer[MAX_PASSWORD_LEN] = {0};
char keypadSDBuffer[MAX_PASSWORD_LEN] = {0};
char bluetoothBuffer[MAX_BT_SERIAL_LEN] = {0};
char bluetoothSDBuffer[MAX_PASSWORD_LEN] = {0};
//Keypad
int rowPins[NUMBER_OF_ROWS] = {16,22,32,33};
int columnPins[NUMBER_OF_COLUMNS] = {25,26,27,14};
Keypad keypad(rowPins,columnPins);
//Real-time clock
RTC_DS3231 rtc;
//Bluetooth
BluetoothSerial SerialBT;
//Functions
void ProcessBluetoothData(void);
void GetKeypadData(char* keyBuffer);
void InputPassword(void);
int RetryPassword(char* keyBuffer,char* password);
void InputNewPassword(void);
void ChangePassword(void);
void InputPhoneNumber(void);
void AddPhoneNumber(void);

void setup() 
{
  setCpuFrequencyMhz(80);
  Wire.begin(21,4); //SDA pin, SCL pin
  Serial.begin(115200);
  Serial2.begin(9600,SERIAL_8N1,-1,17); //for SIM800L
  SerialBT.begin("Smart Door");
  SD.begin(); //Uses pins 23,19,18 and 5
  SD_ReadFile(SD,"/kp.txt",keypadSDBuffer);
  Serial.print("Keypad password:");
  Serial.println(keypadSDBuffer);
  SD_ReadFile(SD,"/bt.txt",bluetoothSDBuffer);
}

void loop() 
{  
  //Bluetooth
  ProcessBluetoothData();
  //Keypad
  char key = keypad.GetChar();
  switch(key)
  {
    case '*':
      InputPassword();
      break;
    case 'C':
      ChangePassword();
      break;
    case 'A':
      AddPhoneNumber();
      break;
  }
}

void ProcessBluetoothData(void)
{
  GetBluetoothData(bluetoothBuffer);
  if(strcmp(bluetoothBuffer,"\0") != 0)
  {
    if(strcmp(bluetoothBuffer,bluetoothSDBuffer) == 0)
    {
      /*
       * Features to be added here:
       * The if statement executes if the password from the app is valid.
       * In body of this if statement, place the logic to handle communication...
       * between the app and the smart door lock. e.g.
       *   1.Now that password is correct, send "open" from the app to open the door
       *   2.Using the app to check when the door was opened/closed
       *   3.Using the app to check whether the door lock's time is correct and set it if incorrect
      */
      Serial.println("Password is correct");
      Serial.println("Door Open");
    }
    else
    {
      Serial.println("Incorrect password");
    }
    memset(bluetoothBuffer,'\0',MAX_BT_SERIAL_LEN);
  }  
}

void GetKeypadData(char* keyBuffer)
{
  int i = 0;
  while(1)
  {
    char key = keypad.GetChar();
    switch(key)
    {
      case '\0':
        break;
      case '#':
        keyBuffer[i] = '\0';
        return;
      default:
        if(i < MAX_PASSWORD_LEN)
        {
          keyBuffer[i] = key;
          i++;
        }
        break;
    }
  }  
}

void InputPassword(void)
{
  char countryCodePhoneNo[15] = {0};
  Serial.println("Entering password mode");
  GetKeypadData(keypadBuffer);
  if(strcmp(keypadBuffer,keypadSDBuffer) == 0)
  {
    Serial.println("Password is correct");
    Serial.println("Door Open");
  }
  else
  {
    Serial.println("Incorrect password, 2 attempts left");
    int retry = RetryPassword(keypadBuffer,keypadSDBuffer);
    switch(retry)
    {
      case PASSWORD_CORRECT:
        Serial.println("Password is now correct");
        Serial.println("Door Open");
        break;
      case PASSWORD_INCORRECT:
        Serial.println("Intruder!!!!!");
        SD_ReadFile(SD,"/pn.txt",countryCodePhoneNo);  
        SendSMS(countryCodePhoneNo,"Intruder!!!!!");
        break;
    }
  }
}

int RetryPassword(char* keyBuffer,char* password)
{
  int passwordState = DEF;
  int retry = 1;
  while(retry <= 2)
  {
    Serial.print("Retry: ");
    Serial.println(retry);
    GetKeypadData(keyBuffer);
    retry++;
    if(strcmp(keyBuffer,password) == 0)
    {
      passwordState = PASSWORD_CORRECT;
      break;
    }
    else
    {
      passwordState = PASSWORD_INCORRECT;
    }
  }
  return passwordState;   
}

void InputNewPassword(void)
{
  char newPassword[MAX_PASSWORD_LEN] = {0};
  GetKeypadData(keypadBuffer);
  strcpy(newPassword,keypadBuffer);
  Serial.println("Reenter the new password");
  GetKeypadData(keypadBuffer);
  if(strcmp(keypadBuffer,newPassword) == 0)
  {
    Serial.println("New password successfully created");
    strcpy(keypadSDBuffer,keypadBuffer);
    SD_WriteFile(SD,"/kp.txt",keypadBuffer);
  }
  else
  {
    Serial.println("Incorrect input, 2 attempts left");
    int retry = RetryPassword(keypadBuffer,newPassword);
    switch(retry)
    {
      case PASSWORD_CORRECT:
        Serial.println("New password successfully created");
        strcpy(keypadSDBuffer,keypadBuffer);
        SD_WriteFile(SD,"/kp.txt",keypadBuffer);
        break;
      case PASSWORD_INCORRECT:
        Serial.println("Unable to create new password");
        break;
    } 
  }  
}

void ChangePassword(void)
{
  char countryCodePhoneNo[15] = {0};
  Serial.println("Enter previous password");
  GetKeypadData(keypadBuffer);
  if(strcmp(keypadBuffer,keypadSDBuffer) == 0)
  {
    Serial.println("Correct, now enter new password");
    InputNewPassword();
  }
  else
  {
    Serial.println("Incorrect password, 2 attempts left");
    int retry = RetryPassword(keypadBuffer,keypadSDBuffer);
    switch(retry)
    {
      case PASSWORD_CORRECT:
        Serial.println("Correct, now enter new password");
        InputNewPassword();
        break;
      case PASSWORD_INCORRECT:
        Serial.println("Intruder!!!!!");
        SD_ReadFile(SD,"/pn.txt",countryCodePhoneNo);  
        SendSMS(countryCodePhoneNo,"Intruder!!!!!");
        break;
    }  
  }
}

void InputPhoneNumber(void)
{
  Serial.println("Enter phone number");
  char phoneNumber[12] = {0};
  char countryCodePhoneNo[15] = {0};
  GetKeypadData(phoneNumber);
  GetCountryCodePhoneNo(countryCodePhoneNo,phoneNumber);
  Serial.print("Phone number:");
  Serial.println(countryCodePhoneNo);
  SD_WriteFile(SD,"/pn.txt",countryCodePhoneNo);  
}

void AddPhoneNumber(void)
{
  char countryCodePhoneNo[15] = {0};
  Serial.println("Enter the password");
  GetKeypadData(keypadBuffer);
  if(strcmp(keypadBuffer,keypadSDBuffer) == 0)
  {
    Serial.println("Password is correct");
    InputPhoneNumber();
  }
  else
  {
    Serial.println("Incorrect password, 2 attempts left");
    int retry = RetryPassword(keypadBuffer,keypadSDBuffer);
    switch(retry)
    {
      case PASSWORD_CORRECT:
        Serial.println("Password is correct");
        InputPhoneNumber();
        break;
      case PASSWORD_INCORRECT:
        Serial.println("Intruder!!!!!");
        SD_ReadFile(SD,"/pn.txt",countryCodePhoneNo);  
        SendSMS(countryCodePhoneNo,"Intruder!!!!!");
        break;
    }    
  }  
}
