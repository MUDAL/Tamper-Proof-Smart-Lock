#include <Arduino.h>
#include "button.h"
#include "threads.h"

#define INDOOR_BUTTON    34
#define OUTDOOR_BUTTON   35
#define IR_SENSOR        36 //VP pin

static Button indoorButton(INDOOR_BUTTON);
static Button outdoorButton(OUTDOOR_BUTTON);
static volatile bool stateArr[4] = {false};

/*
 * @brief Periodically polls and de-bounces the buttons
*/
void IRAM_ATTR Timer0ISR(void)
{
  if(!GetState(DOOR_UNLOCKED))
  {
    if(indoorButton.IsPressedOnce())
    {
      Serial.println("Open");
      /*Place code to open the door*/
      SetState(DOOR_UNLOCKED,true);
    }
  }
  else
  {
    if(indoorButton.IsPressedOnce() || 
       outdoorButton.IsPressedOnce())
    {
      Serial.println("Close");
      /*Place code to close the door*/
      SetState(DOOR_UNLOCKED,false);
    }
  }
}

/*
 * @brief Handles timings asynchronously
*/
void IRAM_ATTR Timer1ISR(void)
{
  static int failedInputTimeout = 0;
  static int buzzerTimeout = 0;
  static int lockTimeout = 0;
  if(GetState(FAILED_INPUT))
  {
    failedInputTimeout++;
    if(failedInputTimeout == 10000)
    {
      //returns access to the lock 
      Serial.println("Access restored");
      SetState(FAILED_INPUT,false);
      failedInputTimeout = 0;
    }
  }
  if(GetState(BUZZER_ON))
  {
    buzzerTimeout++;
    if(buzzerTimeout == 10000)
    {
      /*Place code to turn buzzer off*/
      Serial.println("Buzzer off");
      SetState(BUZZER_ON,false);
      buzzerTimeout = 0;
    }
  }
  /*Place code to turn solenoid lock off (after 8seconds) to avoid power drain*/
  if(GetState(DOOR_UNLOCKED))
  {
    lockTimeout++;
    if(lockTimeout == 8000)
    {
      /*Place code to close the door*/
      Serial.println("Door closed");
      SetState(DOOR_UNLOCKED,false);
      lockTimeout = 0;
    }
  }
  else
  {
    lockTimeout = 0;
  }
}

/*
 * @brief For tamper detection
*/
void IRAM_ATTR GPIO36ISR(void)
{
  SetState(LOCK_TAMPERED,true);
}

//Extern functions
void SetState(int state,bool val)
{
  stateArr[state] = val;
}

bool GetState(int state)
{
  return stateArr[state];
}

void Threads_Init(void)
{
  pinMode(IR_SENSOR,INPUT);
  attachInterrupt(IR_SENSOR,GPIO36ISR,CHANGE);
  hw_timer_t* timer0 = timerBegin(0,80,true);
  timerAttachInterrupt(timer0,Timer0ISR,true);
  timerAlarmWrite(timer0,15000,true);//15ms periodic timer interrupt
  timerAlarmEnable(timer0);
  hw_timer_t* timer1 = timerBegin(1,80,true);
  timerAttachInterrupt(timer1,Timer1ISR,true);
  timerAlarmWrite(timer1,1000,true);//1ms periodic timer interrupt
  timerAlarmEnable(timer1);  
}
