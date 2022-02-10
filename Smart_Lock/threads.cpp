#include <Arduino.h>
#include "button.h"
#include "threads.h"

#define INDOOR_BUTTON    34
#define OUTDOOR_BUTTON   35
#define IR_SENSOR        36 //VP pin

namespace TimeoutMillis
{
  const int failedInput = 10000;
  const int buzzer = 10000;
  const int lock = 8000;
};

static Button indoorButton(INDOOR_BUTTON);
static Button outdoorButton(OUTDOOR_BUTTON);

/*
 * @brief Periodically polls and de-bounces the buttons
*/
void IRAM_ATTR Timer0ISR(void)
{
  if(!System_GetState(DOOR_UNLOCKED))
  {
    if(indoorButton.IsPressedOnce())
    {
      Serial.println("Door opened via indoor button");
      ActuateOutput(LOCK,true);
    }
  }
  else
  {
    if(indoorButton.IsPressedOnce() || 
       outdoorButton.IsPressedOnce())
    {
      Serial.println("Door closed via button");
      ActuateOutput(LOCK,false);
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
  if(System_GetState(FAILED_INPUT))
  {
    failedInputTimeout++;
    if(failedInputTimeout == TimeoutMillis::failedInput)
    {
      //returns access to the lock 
      Serial.println("Access restored");
      System_SetState(FAILED_INPUT,false);
      failedInputTimeout = 0;
    }
  }
  if(System_GetState(BUZZER_ON))
  {
    buzzerTimeout++;
    if(buzzerTimeout == TimeoutMillis::buzzer)
    {
      ActuateOutput(BUZZER,false);
      Serial.println("Buzzer off");
      buzzerTimeout = 0;
    }
  }
  //Turn lock off to avoid power drain
  if(System_GetState(DOOR_UNLOCKED))
  {
    lockTimeout++;
    if(lockTimeout == TimeoutMillis::lock)
    {
      ActuateOutput(LOCK,false);
      Serial.println("Door closed via timeout");
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
  System_SetState(LOCK_TAMPERED,true);
}

void Threads_Init(void)
{
  pinMode(IR_SENSOR,INPUT);
  attachInterrupt(IR_SENSOR,GPIO36ISR,CHANGE);
  hw_timer_t* timer0 = timerBegin(0,80,true); //timer 0, prescaler = 80
  timerAttachInterrupt(timer0,Timer0ISR,true);
  timerAlarmWrite(timer0,15000,true); //15ms periodic timer interrupt
  timerAlarmEnable(timer0);
  hw_timer_t* timer1 = timerBegin(1,80,true); //timer 1, prescaler = 80
  timerAttachInterrupt(timer1,Timer1ISR,true);
  timerAlarmWrite(timer1,1000,true); //1ms periodic timer interrupt
  timerAlarmEnable(timer1);  
}
