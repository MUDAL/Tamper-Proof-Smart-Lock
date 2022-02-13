#include <Wire.h>
#include "EEPROM.h" //To access ESP32 flash memory
#include "RTClib.h" //Version 1.3.3
#include "sd_card.h"
#include "wireless_comm.h"
#include "keypad.h"
#include "threads.h"

/*
 * Components:
 * ESP32 inbuilt bluetooth --> Bluetooth serial
 * Keypad --> GPIO --> [(row pins: 16,22,32,33), (column pins: 25,26,27,14)]
 * SD card [+3.3v power] --> SPI --> [SPI pins: 23(MOSI),19(MISO),18(SCK),5(CS)]
 * RTC module [+3.3v power] --> I2C --> [pins: 21(SDA),4(SCL)]
 * GSM module [External 4.4v power] --> UART --> [UART2 Tx pin: 17]
 * -Fingerprint scanner [+5v power] --> UART --> [UART1 pins: Rx = 9(D2), Tx = 10(D3)]
 * Indoor button to open/close the door --> GPIO with external pullup + Timer Interrupt --> 34
 * Outdoor button to close the door --> GPIO with external pullup + Timer Interrupt --> 35
 * -Electromagnetic lock --> GPIO --> 13
 * IR sensor --> GPIO Interrupt --> 36
 * LED to signify expectation of specific inputs (e.g. phone no) --> GPIO --> 0
 * LED to signify expectation of password --> GPIO --> 2
 * LED to signify an intrusion --> GPIO --> 15
 * Active Buzzer --> GPIO --> 12
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
#define LED_INPUT             0 //Yellow
#define LED_PASSWORD          2 //Blue
#define LED_INTRUSION         15 //Red
#define MAX_PASSWORD_LEN      20 

//Password states
typedef enum 
{
  PASSWORD_INCORRECT = 0,
  PASSWORD_CORRECT
}pw_s;

//Variables
BluetoothSerial SerialBT; 
RTC_DS3231 rtc; 
int rowPins[NUMBER_OF_ROWS] = {16,22,32,33};  
int columnPins[NUMBER_OF_COLUMNS] = {25,26,27,14};
Keypad keypad(rowPins,columnPins); 
bool intruderDetected = false;

//Functions
void ProcessBluetoothData(void);
void GetKeypadData(char* keyBuffer);
void StorePassword(char* password);
void GetPassword(char* pswdBuffer);
void IndicatePasswordReentry(void);
pw_s RetryPassword(char* keyBuffer,char* password);
void InputNewPassword(void);
void InputPhoneNumber(void);
void CheckKey(char key);
void HMI(char key);

void setup() 
{
  setCpuFrequencyMhz(80);
  pinMode(BUZZER,OUTPUT);
  pinMode(LOCK,OUTPUT);
  pinMode(LED_INPUT,OUTPUT);
  pinMode(LED_PASSWORD,OUTPUT);
  pinMode(LED_INTRUSION,OUTPUT);
  Wire.begin(21,4); //SDA pin, SCL pin
  EEPROM.begin(MAX_PASSWORD_LEN);
  rtc.begin();
  Serial.begin(115200);
  Serial2.begin(9600,SERIAL_8N1,-1,17); //for SIM800L
  SerialBT.begin("Smart Door");
  SD.begin(); //Uses pins 23,19,18 and 5
  Threads_Init();
}

void loop() 
{
  //Bluetooth
  ProcessBluetoothData();
  //Keypad
  if(!System_GetState(FAILED_INPUT))
  {
    char key = keypad.GetChar();
    HMI(key);
  }
  //Intruder detection
  if(intruderDetected)
  {
    char countryCodePhoneNo[15] = {0};
    ActuateOutput(BUZZER,true);
    SD_ReadFile(SD,"/pn.txt",countryCodePhoneNo);  
    SendSMS(countryCodePhoneNo,"Intruder: Wrong inputs from Keypad!!!!!");
    Serial.println("Intruder: Wrong inputs from Keypad!!!!!");
    intruderDetected = false;
  }
  //Tamper detection
  if(System_GetState(LOCK_TAMPERED))
  {
    digitalWrite(LED_INTRUSION,HIGH);
    char countryCodePhoneNo[15] = {0};
    ActuateOutput(BUZZER,true);
    SD_ReadFile(SD,"/pn.txt",countryCodePhoneNo);  
    SendSMS(countryCodePhoneNo,"Tamper detected!!!!!");
    Serial.println("Tamper detected!!!!!"); 
    System_SetState(LOCK_TAMPERED,false);
  }
  //Fingerprint
  /*
   * Place code here
  */
}

void ProcessBluetoothData(void)
{
  char pswd[MAX_PASSWORD_LEN] = {0};
  char eepromPswd[MAX_PASSWORD_LEN] = {0};
  GetPassword(eepromPswd);
  GetBluetoothData(pswd,MAX_PASSWORD_LEN);
  if(strcmp(pswd,"\0") != 0)
  {
    Serial.print("Received data = ");
    Serial.println(pswd);
    if(strcmp(pswd,eepromPswd) == 0)
    {
      Serial.println("Password is correct");
      SerialBT.print("\nSmart lock bluetooth codes:\n" 
                     "1. To open the door\n"
                     "2. To close the door\n"
                     "3. To set the time\n"
                     "4. To get security report\n");
      char btCode = '\0';
      while(1)
      {
        digitalWrite(LED_INPUT,HIGH); //Awaiting numeric bluetooth codes
        GetBluetoothData(&btCode,1);
        if(btCode != '\0') 
        {
          digitalWrite(LED_INPUT,LOW);
          break;
        }
      }
      switch(btCode)
      {
        case '1':
          ActuateOutput(LOCK,true);
          Serial.println("Door opened via bluetooth");
          break;
        case '2':
          ActuateOutput(LOCK,false);
          Serial.println("Door closed via bluetooth");
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

void StorePassword(char* password)
{
  DisableThreads();
  //Critical section to avoid Guru Meditation Error
  //This error occurs when an interrupt is fired while accessing the flash memory
  EEPROM.writeString(0,password); //starting address --> 0
  EEPROM.commit();
  EnableThreads();
}

void GetPassword(char* pswdBuffer)
{
  DisableThreads();
  //Critical section to avoid Guru Meditation Error
  int i = 0;
  while(EEPROM.readChar(i) != '\0')
  {
    pswdBuffer[i] = EEPROM.readChar(i);
    i++;
  }
  EnableThreads();
}

void IndicatePasswordReentry(void)
{
  digitalWrite(LED_PASSWORD,LOW);
  delay(250);
  digitalWrite(LED_PASSWORD,HIGH);
}

pw_s RetryPassword(char* keyBuffer,char* password)
{
  pw_s passwordState = PASSWORD_INCORRECT;
  int retry = 1;
  while(retry <= 2)
  {
    Serial.print("Retry: ");
    Serial.println(retry);
    IndicatePasswordReentry();
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
  digitalWrite(LED_INPUT,HIGH); //Awaiting input of new password
  IndicatePasswordReentry();
  GetKeypadData(pswd);
  strcpy(newPassword,pswd);
  Serial.println("Reenter the new password");
  IndicatePasswordReentry();
  GetKeypadData(pswd);
  if(strcmp(pswd,newPassword) == 0)
  {
    Serial.println("New password successfully created");
    StorePassword(pswd);
  }
  else
  {
    digitalWrite(LED_INTRUSION,HIGH);
    Serial.println("Incorrect input, 2 attempts left");
    pw_s pswdState = RetryPassword(pswd,newPassword);
    switch(pswdState)
    {
      case PASSWORD_CORRECT:
        digitalWrite(LED_INTRUSION,LOW);
        Serial.println("New password successfully created");
        StorePassword(pswd);
        break;
      case PASSWORD_INCORRECT:
        Serial.println("Unable to create new password");
        break;
    } 
  }
  digitalWrite(LED_INPUT,LOW);  
}

void InputPhoneNumber(void)
{
  char phoneNumber[12] = {0};
  char countryCodePhoneNo[15] = {0};
  digitalWrite(LED_INPUT,HIGH); //Awaiting phone number input
  GetKeypadData(phoneNumber);
  GetCountryCodePhoneNo(countryCodePhoneNo,phoneNumber);
  Serial.print("Phone number:");
  Serial.println(countryCodePhoneNo);
  SD_WriteFile(SD,"/pn.txt",countryCodePhoneNo);  
  digitalWrite(LED_INPUT,LOW);
}

void CheckKey(char key)
{
  switch(key)
  {
    case '*':
      ActuateOutput(LOCK,true);
      Serial.println("Door Open");
      break;
    case 'C':
      Serial.println("Enter new password");
      InputNewPassword();
      break;
    case 'A':
      Serial.println("Enter phone number");
      InputPhoneNumber();
      break;
  }  
}

void HMI(char key)
{
  if(key != '*' && key != 'C' && key != 'A')
  {
    return;
  }
  char pswd[MAX_PASSWORD_LEN] = {0};
  char eepromPswd[MAX_PASSWORD_LEN] = {0};
  GetPassword(eepromPswd);
  digitalWrite(LED_PASSWORD,HIGH); //Awaiting password 
  Serial.println("Enter the password");
  GetKeypadData(pswd);
  if(strcmp(pswd,eepromPswd) == 0)
  {
    digitalWrite(LED_INTRUSION,LOW);
    Serial.println("Password is correct");
    CheckKey(key);
  }
  else
  {
    digitalWrite(LED_INTRUSION,HIGH);
    Serial.println("Incorrect password, 2 attempts left");
    pw_s pswdState = RetryPassword(pswd,eepromPswd);
    switch(pswdState)
    {
      case PASSWORD_CORRECT:
        digitalWrite(LED_INTRUSION,LOW);
        Serial.println("Password is correct");
        CheckKey(key);
        break;
      case PASSWORD_INCORRECT:
        System_SetState(FAILED_INPUT,true);
        intruderDetected = true;
        break;
    }    
  }
  digitalWrite(LED_PASSWORD,LOW);   
}

