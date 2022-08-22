#include "stm32f10x.h"                  // Device header
#include <stdbool.h>
#include "clock.h"
#include "gpio.h"
#include "i2c.h"
#include "systick.h"
#include "system.h"

void System_Config(void)
{
	Clock_HSI_8MHz_Init();
	SysTick_Init();
	GPIO_Reset();
	//GPIO configuration for common I2C pins
	GPIO_OutputInit(GPIOB,
								GPIO_PORT_REG_LOW,
							 (GPIO_PIN6_OUTPUT_MODE_2MHZ | GPIO_PIN7_OUTPUT_MODE_2MHZ),
							 (GPIO_PIN6_ALT_FUNC_OPEN_DRAIN | GPIO_PIN7_ALT_FUNC_OPEN_DRAIN));
	//I2C configuration
	I2C_Init(I2C1,
					 I2C_PERIPH_FREQ_8MHZ,
					 I2C_STANDARD_MODE_8MHZ_CCR, 
					 I2C_STANDARD_MODE_8MHZ_TRISE);
}

void System_Reset(void)
{
	NVIC_SystemReset();
}

void System_DelayMs(uint32_t delayTime)
{
	SysTick_DelayMs(delayTime);
}

void System_TimerInit(sysTimer_t* pSysTimer, uint32_t timerRepTime)
{
	pSysTimer->start = 0;
	pSysTimer->isCounting = false;
	pSysTimer->ticksToWait = timerRepTime;
}

bool System_TimerDoneCounting(sysTimer_t* pSysTimer)
{
	bool countingComplete = false;
	
	if(!pSysTimer->isCounting)
	{
		pSysTimer->start = SysTick_GetTick();
		pSysTimer->isCounting = true;
	}
	
	else
	{
		if((SysTick_GetTick() - pSysTimer->start) >= pSysTimer->ticksToWait)
		{
			countingComplete = true;
			pSysTimer->start = 0;
			pSysTimer->isCounting = false;
		}
	}
	return countingComplete;
}

