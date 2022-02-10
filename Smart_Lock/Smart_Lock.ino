#include <Wire.h>
#include "RTClib.h" //Version 1.3.3
#include "sd_card.h"
#include "wireless_comm.h"
#include "keypad.h"
#include "threads.h"

/*
 * Components:
 * ESP32 inbuilt bluetooth --> Bluetooth serial
 * Keypad --> GPIO --> [(row pins: 16,22,32,33), (column pins: 25,26,27,14)]
 * SD card [+5v power] --> SPI --> [SPI pins: 23(MOSI),19(MISO),18(SCK),5(CS)]
 * RTC module [+3.3v power] --> I2C --> [pins: 21(SDA),4(SCL)]
 * GSM module [External 4.3v power] --> UART --> [UART2 Tx pin: 17]
 * -Fingerprint scanner --> UART --> [UART1 pins: 9(D2),10(D3)]
 * Indoor button to open/close the door --> GPIO with external pullup + Timer Interrupt --> 34
 * Outdoor button to close the door --> GPIO with external pullup + Timer Interrupt --> 35
 * -Electromagnetic lock --> GPIO --> 13
 * IR sensor --> GPIO Interrupt --> 36
 * LED to signify the lock is awaiting an input --> GPIO --> 2
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
bool intruderDetected = false;

//Functions
void ProcessBluetoothData(void);
void GetKeypadData(char* keyBuffer);
void IndicatePasswordQuery(void);
void InputPassword(void);
pw_s RetryPassword(char* keyBuffer,char* password);
void InputNewPassword(void);
void ChangePassword(void);
void InputPhoneNumber(void);
void AddPhoneNumber(void);

void setup() 
{
  setCpuFrequencyMhz(80);
  pinMode(BUZZER,OUTPUT);
  pinMode(LOCK,OUTPUT);
  ActuateOutput(LOCK,false); 
  pinMode(LED_INPUT,OUTPUT);
  pinMode(LED_PASSWORD,OUTPUT);
  pinMode(LED_INTRUSION,OUTPUT);
  Wire.begin(21,4); //SDA pin, SCL pin
  rtc.begin();
  Serial.begin(115200);
  Serial2.begin(9600,SERIAL_8N1,-1,17); //for SIM800L
  SerialBT.begin("Smart Door");
  SD.begin(); //Uses pins 23,19,18 and 5
  SD_ReadFile(SD,"/pw.txt",sdPassword);
  Serial.print("password:");
  Serial.println(sdPassword);
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
    switch(key)
    {
      case '*':
        digitalWrite(LED_PASSWORD,HIGH);
        InputPassword();
        digitalWrite(LED_PASSWORD,LOW);
        break;
      case 'C':
        digitalWrite(LED_PASSWORD,HIGH);
        ChangePassword();
        digitalWrite(LED_PASSWORD,LOW);
        digitalWrite(LED_INPUT,LOW);
        break;
      case 'A':
        digitalWrite(LED_PASSWORD,HIGH);
        AddPhoneNumber();
        digitalWrite(LED_PASSWORD,LOW);
        digitalWrite(LED_INPUT,LOW);
        break;
    }
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
  GetBluetoothData(pswd,MAX_PASSWORD_LEN);
  if(strcmp(pswd,"\0") != 0)
  {
    Serial.print("Received data = ");
    Serial.println(pswd);
    if(strcmp(pswd,sdPassword) == 0)
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
        digitalWrite(LED_INPUT,HIGH);
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

void IndicatePasswordQuery(void)
{
  digitalWrite(LED_PASSWORD,LOW);
  delay(1000);
  digitalWrite(LED_PASSWORD,HIGH);
}

void InputPassword(void)
{
  char pswd[MAX_PASSWORD_LEN] = {0};
  Serial.println("Entering password mode");
  GetKeypadData(pswd);
  if(strcmp(pswd,sdPassword) == 0)
  {
    digitalWrite(LED_INTRUSION,LOW);
    ActuateOutput(LOCK,true);
    Serial.println("Password is correct");
    Serial.println("Door Open");
  }
  else
  {
    digitalWrite(LED_INTRUSION,HIGH);
    Serial.println("Incorrect password, 2 attempts left");
    pw_s pswdState = RetryPassword(pswd,sdPassword);
    switch(pswdState)
    {
      case PASSWORD_CORRECT:
        digitalWrite(LED_INTRUSION,LOW);
        ActuateOutput(LOCK,true);
        Serial.println("Password is now correct");
        Serial.println("Door Open");
        break;
      case PASSWORD_INCORRECT:
        System_SetState(FAILED_INPUT,true);
        intruderDetected = true;
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
    IndicatePasswordQuery();
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
  digitalWrite(LED_INPUT,HIGH);
  IndicatePasswordQuery();
  GetKeypadData(pswd);
  strcpy(newPassword,pswd);
  Serial.println("Reenter the new password");
  IndicatePasswordQuery();
  GetKeypadData(pswd);
  if(strcmp(pswd,newPassword) == 0)
  {
    Serial.println("New password successfully created");
    strcpy(sdPassword,pswd);
    SD_WriteFile(SD,"/pw.txt",sdPassword);
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
  Serial.println("Enter previous password");
  GetKeypadData(pswd);
  if(strcmp(pswd,sdPassword) == 0)
  {
    digitalWrite(LED_INTRUSION,LOW);
    Serial.println("Correct, now enter new password");
    InputNewPassword();
  }
  else
  {
    digitalWrite(LED_INTRUSION,HIGH);
    Serial.println("Incorrect password, 2 attempts left");
    pw_s pswdState = RetryPassword(pswd,sdPassword);
    switch(pswdState)
    {
      case PASSWORD_CORRECT:
        digitalWrite(LED_INTRUSION,LOW);
        Serial.println("Correct, now enter new password");
        InputNewPassword();
        break;
      case PASSWORD_INCORRECT:
        System_SetState(FAILED_INPUT,true);
        intruderDetected = true;
        break;
    }  
  }
}

void InputPhoneNumber(void)
{
  Serial.println("Enter phone number");
  char phoneNumber[12] = {0};
  char countryCodePhoneNo[15] = {0};
  digitalWrite(LED_INPUT,HIGH);
  GetKeypadData(phoneNumber);
  GetCountryCodePhoneNo(countryCodePhoneNo,phoneNumber);
  Serial.print("Phone number:");
  Serial.println(countryCodePhoneNo);
  SD_WriteFile(SD,"/pn.txt",countryCodePhoneNo);  
}

void AddPhoneNumber(void)
{
  char pswd[MAX_PASSWORD_LEN] = {0};
  Serial.println("Enter the password");
  GetKeypadData(pswd);
  if(strcmp(pswd,sdPassword) == 0)
  {
    digitalWrite(LED_INTRUSION,LOW);
    Serial.println("Password is correct");
    InputPhoneNumber();
  }
  else
  {
    digitalWrite(LED_INTRUSION,HIGH);
    Serial.println("Incorrect password, 2 attempts left");
    pw_s pswdState = RetryPassword(pswd,sdPassword);
    switch(pswdState)
    {
      case PASSWORD_CORRECT:
        digitalWrite(LED_INTRUSION,LOW);
        Serial.println("Password is correct");
        InputPhoneNumber();
        break;
      case PASSWORD_INCORRECT:
        System_SetState(FAILED_INPUT,true);
        intruderDetected = true;
        break;
    }    
  }  
}

