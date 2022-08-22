#include <Arduino.h>
#include "button.h"
#include "threads.h"

#define INDOOR_BUTTON    34
#define OUTDOOR_BUTTON   35
#define IR_SENSOR        36 //VP pin

const int tSystemHalt = 10000; //time taken for system to halt
const int tLockOn = 8000; //time taken for lock to be on
static hw_timer_t* timer0;
static hw_timer_t* timer1;
static Button indoorButton(INDOOR_BUTTON);
static Button outdoorButton(OUTDOOR_BUTTON);

static void HandleFailedInput(void)
{
  static int failedInputTimeout = 0;
  if(System_GetState(FAILED_INPUT))
  {
    failedInputTimeout++;
    if(failedInputTimeout == tSystemHalt)
    {
      //returns access to the keypad
      System_SetState(FAILED_INPUT,false);
      failedInputTimeout = 0;
    }
  }
}

static void HandleActiveBuzzer(void)
{
  static int buzzerTimeout = 0;
  if(System_GetState(BUZZER_ON))
  {
    buzzerTimeout++;
    if(buzzerTimeout == tSystemHalt)
    {
      ActuateOutput(BUZZER,false);
      buzzerTimeout = 0;
    }
  }  
}

static void HandleLock(void)
{
  static int lockTimeout = 0;
  //Turn lock off to avoid power drain
  if(System_GetState(DOOR_UNLOCKED))
  {
    lockTimeout++;
    if(lockTimeout == tLockOn)
    {
      ActuateOutput(LOCK,false);
      lockTimeout = 0;
    }
  }
  else
  {//If door is locked by other means (buttons)
    lockTimeout = 0;
  }  
}

static void HandleFingerprint(void)
{
  static int tInvalidPrint = 0;
  if(System_GetState(INVALID_FINGERPRINT))
  {
    tInvalidPrint++;
    if(tInvalidPrint == tSystemHalt)
    {
      //returns access to fingerprint module
      System_SetState(INVALID_FINGERPRINT,false);
      tInvalidPrint = 0;
    }
  }  
}

/*
 * @brief Periodically polls and de-bounces the buttons
*/
void IRAM_ATTR Timer0ISR(void)
{
  if(!System_GetState(DOOR_UNLOCKED))
  {
    if(indoorButton.IsPressedOnce())
    {
      ActuateOutput(LOCK,true);
    }
  }
  else
  {
    if(indoorButton.IsPressedOnce() || 
       outdoorButton.IsPressedOnce())
    {
      ActuateOutput(LOCK,false);
    }
  }
}

/*
 * @brief Handles timings asynchronously
*/
void IRAM_ATTR Timer1ISR(void)
{
  HandleFailedInput();
  HandleActiveBuzzer();
  HandleLock();
  HandleFingerprint();
}

/*
 * @brief For tamper detection
*/
void IRAM_ATTR GPIO36ISR(void)
{
  System_SetState(LOCK_TAMPERED,true);
}

void Threads_Init(void)
{
  pinMode(IR_SENSOR,INPUT);
  attachInterrupt(IR_SENSOR,GPIO36ISR,CHANGE);
  timer0 = timerBegin(0,80,true); //timer 0, prescaler = 80
  timerAttachInterrupt(timer0,Timer0ISR,true);
  timerAlarmWrite(timer0,15000,true); //15ms periodic timer interrupt
  timerAlarmEnable(timer0);
  timer1 = timerBegin(1,80,true); //timer 1, prescaler = 80
  timerAttachInterrupt(timer1,Timer1ISR,true);
  timerAlarmWrite(timer1,1000,true); //1ms periodic timer interrupt
  timerAlarmEnable(timer1);  
}

void DisableThreads(void)
{
  detachInterrupt(IR_SENSOR);
  timerAlarmDisable(timer0);
  timerAlarmDisable(timer1);
}

void EnableThreads(void)
{
  attachInterrupt(IR_SENSOR,GPIO36ISR,CHANGE);
  timerAlarmEnable(timer0);
  timerAlarmEnable(timer1);
}

