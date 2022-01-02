#include <Wire.h>
#include "RTClib.h" //Version 1.3.3
#include "sd_card.h"
#include "gsm_bt.h"
#include "keypad.h"

/*
 * Components:
 * ESP32 inbuilt bluetooth --> Bluetooth serial
 * Keypad --> GPIO --> [(row pins: 16,22,32,33), (column pins: 25,26,27,14)]
 * SD card [+3.3v power] --> SPI --> [SPI pins: 23,19,18,5]
 * RTC module [+3.3v power] --> I2C --> [pins: 21(SDA),4(SCL)]
 * GSM module [External 4.3v power] --> UART --> [UART2 Tx pin: 17]
 * Fingerprint scanner --> UART --> [UART1 pins: 9,10]
 * Indoor button to open the door --> GPIO Interrupt -->
 * Indoor button to close the door --> GPIO Interrupt -->
 * Outdoor button to close the door --> GPIO Interrupt -->
 * Electromagnetic lock
 * IR sensor --> GPIO -->
 * LED(s)
 * Buzzer
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
typedef enum 
{
  PASSWORD_INCORRECT = 0,
  PASSWORD_CORRECT
}pw_s;

//Variables
BluetoothSerial SerialBT; 
RTC_DS3231 rtc; 
char sdPassword[MAX_PASSWORD_LEN] = {0}; //Stored password
int rowPins[NUMBER_OF_ROWS] = {16,22,32,33};  
int columnPins[NUMBER_OF_COLUMNS] = {25,26,27,14};
Keypad keypad(rowPins,columnPins); 

//Functions
void ProcessBluetoothData(void);
void GetKeypadData(char* keyBuffer);
void InputPassword(void);
pw_s RetryPassword(char* keyBuffer,char* password);
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
  SD_ReadFile(SD,"/pw.txt",sdPassword);
  Serial.print("password:");
  Serial.println(sdPassword);
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
  char pswd[MAX_PASSWORD_LEN] = {0};
  GetBluetoothData(pswd,MAX_PASSWORD_LEN);
  if(strcmp(pswd,"\0") != 0)
  {
    Serial.print("Received data = ");
    Serial.println(pswd);
    if(strcmp(pswd,sdPassword) == 0)
    {
      SerialBT.print("\nSmart lock request codes:\n" 
                     "1. To open the door\n"
                     "2. To close the door\n"
                     "3. To set the time\n"
                     "4. To get security report\n");
      char requestCode = '\0';
      while(1)
      {
        GetBluetoothData(&requestCode,1);
        if(requestCode != '\0') 
        {
          break;
        }
      }
      switch(requestCode)
      {
        case '1':
          Serial.println("Password is correct");
          Serial.println("Door Open");
          break;
      }
    }
    else
    {
      Serial.println("Incorrect password");
    }
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
  char pswd[MAX_PASSWORD_LEN] = {0};
  char countryCodePhoneNo[15] = {0};
  Serial.println("Entering password mode");
  GetKeypadData(pswd);
  if(strcmp(pswd,sdPassword) == 0)
  {
    Serial.println("Password is correct");
    Serial.println("Door Open");
  }
  else
  {
    Serial.println("Incorrect password, 2 attempts left");
    pw_s pswdState = RetryPassword(pswd,sdPassword);
    switch(pswdState)
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

pw_s RetryPassword(char* keyBuffer,char* password)
{
  pw_s passwordState = PASSWORD_INCORRECT;
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
  }
  return passwordState;   
}

void InputNewPassword(void)
{
  char pswd[MAX_PASSWORD_LEN] = {0};
  char newPassword[MAX_PASSWORD_LEN] = {0};
  GetKeypadData(pswd);
  strcpy(newPassword,pswd);
  Serial.println("Reenter the new password");
  GetKeypadData(pswd);
  if(strcmp(pswd,newPassword) == 0)
  {
    Serial.println("New password successfully created");
    strcpy(sdPassword,pswd);
    SD_WriteFile(SD,"/pw.txt",sdPassword);
  }
  else
  {
    Serial.println("Incorrect input, 2 attempts left");
    pw_s pswdState = RetryPassword(pswd,newPassword);
    switch(pswdState)
    {
      case PASSWORD_CORRECT:
        Serial.println("New password successfully created");
        strcpy(sdPassword,pswd);
        SD_WriteFile(SD,"/pw.txt",sdPassword);
        break;
      case PASSWORD_INCORRECT:
        Serial.println("Unable to create new password");
        break;
    } 
  }  
}

void ChangePassword(void)
{
  char pswd[MAX_PASSWORD_LEN] = {0};
  char countryCodePhoneNo[15] = {0};
  Serial.println("Enter previous password");
  GetKeypadData(pswd);
  if(strcmp(pswd,sdPassword) == 0)
  {
    Serial.println("Correct, now enter new password");
    InputNewPassword();
  }
  else
  {
    Serial.println("Incorrect password, 2 attempts left");
    pw_s pswdState = RetryPassword(pswd,sdPassword);
    switch(pswdState)
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
  char pswd[MAX_PASSWORD_LEN] = {0};
  char countryCodePhoneNo[15] = {0};
  Serial.println("Enter the password");
  GetKeypadData(pswd);
  if(strcmp(pswd,sdPassword) == 0)
  {
    Serial.println("Password is correct");
    InputPhoneNumber();
  }
  else
  {
    Serial.println("Incorrect password, 2 attempts left");
    pw_s pswdState = RetryPassword(pswd,sdPassword);
    switch(pswdState)
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
