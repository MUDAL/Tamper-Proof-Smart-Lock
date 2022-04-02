#ifndef _SYSTEM_H
#define _SYSTEM_H

typedef struct
{
	bool isCounting; 
	uint32_t start; 
	uint32_t ticksToWait; 
}sysTimer_t;

extern void System_Init(void);
extern void System_Reset(void);
extern void System_TimerDelayMs(uint32_t delayTime);
extern void System_TimerInit(sysTimer_t* pSysTimer, uint32_t timerRepTime);
extern bool System_TimerDoneCounting(sysTimer_t* pSysTimer);

#endif //_SYSTEM_H
