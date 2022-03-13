#include "EEPROM.h" //To access ESP32 flash memory
#include "RTClib.h" //Version 1.3.3
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>
#include "sd_card.h"
#include "wireless_comm.h"
#include "keypad.h"
#include "threads.h"

/*
 * Components:
 * ESP32 inbuilt bluetooth --> Bluetooth serial
 * Keypad --> GPIO --> [(row pins: 16,4,32,33), (column pins: 25,26,27,14)]
 * SD card [+3.3v power] --> SPI --> [SPI pins: 23(MOSI),19(MISO),18(SCK),5(CS)]
 * OLED display [+3.3v power] --> I2C --> [pins: 21(SDA),22(SCL)]
 * RTC module [+3.3v power] --> I2C --> [pins: 21(SDA),22(SCL)]
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
*/
#define MAX_PASSWORD_LEN      20 

//Password states
typedef enum 
{
  PASSWORD_INCORRECT = 0,
  PASSWORD_CORRECT
}pw_s;

//Variables
BluetoothSerial SerialBT; 
Adafruit_SH1106 oled(21,22); //SDA = 21, SCL = 22
RTC_DS3231 rtc; 
int rowPins[NUMBER_OF_ROWS] = {16,4,32,33};  
int columnPins[NUMBER_OF_COLUMNS] = {25,26,27,14};
Keypad keypad(rowPins,columnPins); 
bool intruderDetected = false;

//Functions
void Display(char* msg1,int msg2 = -1,int row = 1,int col = 1);
void Display(char* msg1,char* msg2,int row = 1,int col = 1);
void ProcessBluetoothData(void);
void GetKeypadData(char* keyBuffer);
void StorePassword(char* password);
void GetPassword(char* pswdBuffer);
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
  EEPROM.begin(MAX_PASSWORD_LEN);
  oled.begin(SH1106_SWITCHCAPVCC,0x3C);
  oled.setTextSize(1);
  oled.setTextColor(WHITE);
  oled.clearDisplay();
  oled.display();
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
  //HMI (Keypad + OLED)
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
    intruderDetected = false;
  }
  //Tamper detection
  if(System_GetState(LOCK_TAMPERED))
  {
    char countryCodePhoneNo[15] = {0};
    ActuateOutput(BUZZER,true);
    SD_ReadFile(SD,"/pn.txt",countryCodePhoneNo);  
    SendSMS(countryCodePhoneNo,"Tamper detected!!!!!"); 
    System_SetState(LOCK_TAMPERED,false);
  }
  //Fingerprint
  /*
   * Place code here
  */
}

void Display(char* msg1,int msg2,int row,int col)
{
  oled.clearDisplay();
  oled.setCursor(row,col);
  oled.print(msg1);
  if(msg2 != -1)
  {
    oled.println(msg2);
  }
  oled.display();
}

void Display(char* msg1,char* msg2,int row,int col)
{
  oled.clearDisplay();
  oled.setCursor(row,col);
  oled.print(msg1);  
  oled.println(msg2);
  oled.display();
}

void ProcessBluetoothData(void)
{
  char pswd[MAX_PASSWORD_LEN] = {0};
  char eepromPswd[MAX_PASSWORD_LEN] = {0};
  GetPassword(eepromPswd);
  GetBluetoothData(pswd,MAX_PASSWORD_LEN);
  if(strcmp(pswd,"\0") != 0)
  {
    if(strcmp(pswd,eepromPswd) == 0)
    {
      SerialBT.print("\nSmart lock bluetooth codes:\n" 
                     "1. To open the door\n"
                     "2. To close the door\n"
                     "3. To set the time\n"
                     "4. To get security report\n");
      char btCode = '\0';
      while(1)
      {
        GetBluetoothData(&btCode,1);
        if(btCode != '\0') 
        {
          break;
        }
      }
      switch(btCode)
      {
        case '1':
          ActuateOutput(LOCK,true);
          break;
        case '2':
          ActuateOutput(LOCK,false);
          break;
      }
    }
    else
    {
      SerialBT.println("Incorrect password");
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
  //Critical section to avoid Guru Meditation Error 
  //This error occurs when an interrupt is fired while accessing the flash memory
  DisableThreads();
  EEPROM.writeString(0,password); //starting address --> 0
  EEPROM.commit();
  EnableThreads();
}

void GetPassword(char* pswdBuffer)
{
  //Critical section to avoid Guru Meditation Error
  DisableThreads();
  int i = 0;
  while(EEPROM.readChar(i) != '\0')
  {
    pswdBuffer[i] = EEPROM.readChar(i);
    i++;
  }
  EnableThreads();
}

pw_s RetryPassword(char* keyBuffer,char* password)
{
  pw_s passwordState = PASSWORD_INCORRECT;
  int retry = 1;
  while(retry <= 2)
  {
    Display("Incorrect\nRetry:",retry);
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
  Display("Reenter new password");
  GetKeypadData(pswd);
  if(strcmp(pswd,newPassword) == 0)
  {
    Display("New password created");
    StorePassword(pswd);
  }
  else
  {
    pw_s pswdState = RetryPassword(pswd,newPassword);
    switch(pswdState)
    {
      case PASSWORD_CORRECT:
        Display("New password created");
        StorePassword(pswd);
        break;
      case PASSWORD_INCORRECT:
        Display("Could not create");
        break;
    } 
  } 
}

void InputPhoneNumber(void)
{
  char phoneNumber[12] = {0};
  char countryCodePhoneNo[15] = {0};
  GetKeypadData(phoneNumber);
  Display("Phone number:\n",phoneNumber);
  GetCountryCodePhoneNo(countryCodePhoneNo,phoneNumber);
  SD_WriteFile(SD,"/pn.txt",countryCodePhoneNo);  
}

void CheckKey(char key)
{
  switch(key)
  {
    case '*':
      Display("Correct");
      ActuateOutput(LOCK,true);
      break;
    case 'C':
      Display("Correct\nEnter new password");
      InputNewPassword();
      break;
    case 'A':
      Display("Correct\nEnter phone number");
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
        System_SetState(FAILED_INPUT,true);
        intruderDetected = true;
        break;
    }    
  }
  delay(2000);
  oled.clearDisplay();
  oled.display();
}

