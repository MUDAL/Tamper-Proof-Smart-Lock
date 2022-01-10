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
 * -Fingerprint scanner --> UART --> [UART1 pins: 9,10]
 * Indoor button to open/close the door --> GPIO with external pullup + Timer Interrupt --> 34
 * Outdoor button to close the door --> GPIO with external pullup + Timer Interrupt --> 35
 * -Electromagnetic lock --> GPIO -->
 * IR sensor --> GPIO Interrupt --> 36
 * LED to signify the lock is awaiting an input --> GPIO --> 2
 * LED to signify an incorrect input --> GPIO --> 15
 * -Active Buzzer --> GPIO --> 
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

#define LED_AWAITING_INPUT    2
#define LED_INTRUSION         15
#define IR_SENSOR             36 //VP pin

//Password states
typedef enum 
{
  PASSWORD_INCORRECT = 0,
  PASSWORD_CORRECT
}pw_s;

//Button struct
typedef struct
{
  int pin;
  bool prevPressed;
}button_t;

//Variables
BluetoothSerial SerialBT; 
RTC_DS3231 rtc; 
char sdPassword[MAX_PASSWORD_LEN] = {0}; //Stored password
int rowPins[NUMBER_OF_ROWS] = {16,22,32,33};  
int columnPins[NUMBER_OF_COLUMNS] = {25,26,27,14};
Keypad keypad(rowPins,columnPins); 
button_t indoorButton = {34,false};
button_t outdoorButton = {35,false};
hw_timer_t* timer0 = NULL;
hw_timer_t* timer1 = NULL;
bool intruderDetected = false;
volatile bool failedInput = false; //set when password is incorrect after multiple trials, reset by a Timer1 ISR after a timeout
volatile bool buzzerOn = false; //set when password is incorrect after multiple trials/ tamper detection, reset by a Timer1 ISR after a timeout
volatile bool tampered = false;

//Functions
bool ReadButton(button_t* pButton);
void ProcessBluetoothData(void);
void GetKeypadData(char* keyBuffer);
void InputPassword(void);
pw_s RetryPassword(char* keyBuffer,char* password);
void InputNewPassword(void);
void ChangePassword(void);
void InputPhoneNumber(void);
void AddPhoneNumber(void);

//ISRs
/*
 * @brief Periodically polls and de-bounces the buttons
*/
void IRAM_ATTR Timer0ISR(void)
{
  static int toggle = 0;
  if(ReadButton(&indoorButton))
  {
    toggle ^= 1;
    if(toggle)
    {
      Serial.println("Open");
    }
    else
    {
      Serial.println("Close");
    }
  }
  if(ReadButton(&outdoorButton))
  {
    Serial.println("Close");
  }
}

/*
 * @brief Handles timings asynchronously
*/
void IRAM_ATTR Timer1ISR(void)
{
  static int failedInputTimeout = 0;
  static int buzzerTimeout = 0;
  if(failedInput)
  {
    failedInputTimeout++;
    if(failedInputTimeout == 10000)
    {
      //returns access to the lock 
      Serial.println("Access restored");
      failedInput = false;
      failedInputTimeout = 0;
    }
  }
  if(buzzerOn)
  {
    buzzerTimeout++;
    if(buzzerTimeout == 10000)
    {
      /*Place code to turn buzzer off*/
      Serial.println("Buzzer off");
      buzzerOn = false;
    }
  }
  /*Place code to turn solenoid lock off (after 8seconds) to avoid power drain*/
}

/*
 * @brief For tamper detection
*/
void IRAM_ATTR GPIO36ISR(void)
{
  tampered = true;
}

void setup() 
{
  setCpuFrequencyMhz(80);
  pinMode(indoorButton.pin,INPUT);
  pinMode(outdoorButton.pin,INPUT);
  pinMode(IR_SENSOR,INPUT);
  pinMode(LED_AWAITING_INPUT,OUTPUT);
  pinMode(LED_INTRUSION,OUTPUT);
  Wire.begin(21,4); //SDA pin, SCL pin
  Serial.begin(115200);
  Serial2.begin(9600,SERIAL_8N1,-1,17); //for SIM800L
  SerialBT.begin("Smart Door");
  SD.begin(); //Uses pins 23,19,18 and 5
  SD_ReadFile(SD,"/pw.txt",sdPassword);
  Serial.print("password:");
  Serial.println(sdPassword);
  attachInterrupt(IR_SENSOR,GPIO36ISR,CHANGE);
  timer0 = timerBegin(0,80,true);
  timerAttachInterrupt(timer0,Timer0ISR,true);
  timerAlarmWrite(timer0,10000,true);//10ms periodic timer interrupt
  timerAlarmEnable(timer0);
  timer1 = timerBegin(1,80,true);
  timerAttachInterrupt(timer1,Timer1ISR,true);
  timerAlarmWrite(timer1,1000,true);//1ms periodic timer interrupt
  timerAlarmEnable(timer1);
}

void loop() 
{
  //Bluetooth
  ProcessBluetoothData();
  //Keypad
  if(!failedInput)
  {
    char key = keypad.GetChar();
    switch(key)
    {
      case '*':
        digitalWrite(LED_AWAITING_INPUT,HIGH);
        InputPassword();
        digitalWrite(LED_AWAITING_INPUT,LOW);
        break;
      case 'C':
        digitalWrite(LED_AWAITING_INPUT,HIGH);
        ChangePassword();
        digitalWrite(LED_AWAITING_INPUT,LOW);
        break;
      case 'A':
        digitalWrite(LED_AWAITING_INPUT,HIGH);
        AddPhoneNumber();
        digitalWrite(LED_AWAITING_INPUT,LOW);
        break;
    }
  }
  //Intruder detection
  if(intruderDetected)
  {
    char countryCodePhoneNo[15] = {0};
    Serial.println("Intruder!!!!!");
    /*Place code to turn buzzer on*/
    buzzerOn = true;
    SD_ReadFile(SD,"/pn.txt",countryCodePhoneNo);  
    SendSMS(countryCodePhoneNo,"Intruder!!!!!");
    intruderDetected = false;
  }
  //Tamper detection
  if(tampered)
  {
    digitalWrite(LED_INTRUSION,HIGH);
    char countryCodePhoneNo[15] = {0};
    Serial.println("Tamper detected!!!!!"); 
    /*Place code to turn buzzer on*/
    buzzerOn = true;
    SD_ReadFile(SD,"/pn.txt",countryCodePhoneNo);  
    SendSMS(countryCodePhoneNo,"Tamper detected!!!!!");
    tampered = false;
  }
  //Fingerprint
  /*
   * Place code here
  */
}

/*
 * @brief Reads the logic state of a button
 * @param pButton: pointer to struct containing button data
 * @return true if button is pressed once and false if otherwise
*/
bool ReadButton(button_t* pButton)
{
  if(!digitalRead(pButton->pin) && !pButton->prevPressed)
  {
    pButton->prevPressed = true;
    return true;
  }
  else if(digitalRead(pButton->pin) && pButton->prevPressed)
  {
    pButton->prevPressed = false;
  }
  return false;    
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
  Serial.println("Entering password mode");
  GetKeypadData(pswd);
  if(strcmp(pswd,sdPassword) == 0)
  {
    digitalWrite(LED_INTRUSION,LOW);
    /*Place code to open the lock*/
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
        /*Place code to open the lock*/
        Serial.println("Password is now correct");
        Serial.println("Door Open");
        break;
      case PASSWORD_INCORRECT:
        failedInput = true;
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
        failedInput = true;
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
        failedInput = true;
        intruderDetected = true;
        break;
    }    
  }  
}
