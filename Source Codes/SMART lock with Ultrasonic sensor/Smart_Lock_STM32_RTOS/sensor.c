#include "stm32f10x.h"                  // Device header
#include <stdbool.h>
#include <FreeRTOS.h>
#include <task.h>
#include "gpio.h"
#include "timer.h"
#include "sensor.h"

#define SYS_CLK_FREQ_MHZ		8
#define IC_TIMER_PRESCALER	200 //prescaler for input capture timer

static void TrigTask(void* pvParameters)
{
	while(1)
	{
		vTaskSuspendAll();
		GPIO_OutputWrite(GPIOA,GPIO_PIN4,true);
		TIM_DelayMicros(TIM2);
		GPIO_OutputWrite(GPIOA,GPIO_PIN4,false);
		xTaskResumeAll();
		vTaskDelay(pdMS_TO_TICKS(50));
	}
}

void Sensor_Init(void)
{
	//HCSR04 Echo pin init (PA6 -> Input) 
	//[PA6 and PA7 capture rising and falling edges respectively]
	GPIO_InputInit(GPIOA,GPIO_PORT_REG_LOW,GPIO_PIN6,GPIO_PIN6_INPUT_FLOATING,false);
	GPIO_InputInit(GPIOA,GPIO_PORT_REG_LOW,GPIO_PIN7,GPIO_PIN7_INPUT_FLOATING,false);
	TIM_InputCaptureInit(TIM3,(IC_TIMER_PRESCALER - 1),8000-1);
	//HCSR04 Trig pin init (PA4 -> Output)
	GPIO_OutputInit(GPIOA,
									GPIO_PORT_REG_LOW,
									GPIO_PIN4_OUTPUT_MODE_2MHZ,
									GPIO_GEN_PUR_OUTPUT_PUSH_PULL);	
	xTaskCreate(TrigTask,"",100,NULL,1,NULL);
	TIM_Init(TIM2,0,159);//Timer (20uS timebase for Trig pin)
}

uint32_t Sensor_GetDistance(void)
{
	uint32_t dist = (float)TIM_GetDutyCycle(TIM3) * IC_TIMER_PRESCALER / (58 * SYS_CLK_FREQ_MHZ);
	return dist;
}
