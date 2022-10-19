#include "stm32f10x.h"                  // Device header
#include <stdbool.h>
#include "timer.h"

void TIM_Init(TIM_TypeDef* TIMx,uint16_t prescale,uint16_t reload)
{
	TIMx->PSC = prescale;
	TIMx->ARR = reload;
	TIMx->DIER |= TIM_DIER_UIE;
}

void TIM_DelayMicros(TIM_TypeDef* TIMx)
{
	TIMx->CR1 |= TIM_CR1_CEN;
	while((TIMx->SR & TIM_SR_UIF) != TIM_SR_UIF)
	{
	}
	TIMx->SR &= ~TIM_SR_UIF; //clear UIF flag by writing 0.
	TIMx->CR1 &= ~TIM_CR1_CEN; //stop timer
}

void TIM_InputCaptureInit(TIM_TypeDef* TIMx,uint16_t prescale,uint16_t reload)
{
	TIMx->PSC = prescale;
	TIMx->ARR = reload; //load TIMx 
	// === Input Capture Configuration for PWM ===
	TIMx->CCMR1 |= TIM_CCMR1_CC1S_0; //CC1 = input, IC1 is mapped to TI1
	//NB: By default, when CC1 is configured as input, it is sensitive to rising edge signals
	TIMx->CCMR1 |= TIM_CCMR1_CC2S_1; //CC2 = input, IC2 is mapped to TI1
	TIMx->CCMR1 |= (TIM_CCMR1_IC1F_0 | TIM_CCMR1_IC1F_1 | TIM_CCMR1_IC2F_0 | TIM_CCMR1_IC2F_1);//enabling input filter 
  TIMx->CCER |= (TIM_CCER_CC2P); //enable sensitivity of CC2 to falling edge 
	TIMx->SMCR |= (TIM_SMCR_TS_0 | TIM_SMCR_TS_2); //Selecting filtered Timer input 1 as trigger input
	TIMx->SMCR |= TIM_SMCR_SMS_2; //configure slave mode controller in reset mode
	TIMx->CCER |= (TIM_CCER_CC1E | TIM_CCER_CC2E); //enable input capture
	TIMx->CR1 |= TIM_CR1_CEN; //enable TIMx counter.
}

void TIM_GetDutyCycle(TIM_TypeDef* TIMx,uint32_t* pDuty)
{
	if((TIMx->SR & TIM_SR_CC2IF) == TIM_SR_CC2IF)
	{
		*pDuty = TIMx->CCR2;
	}
}


