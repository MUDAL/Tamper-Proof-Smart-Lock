#ifndef TIMER_H
#define TIMER_H

extern void TIM_Init(TIM_TypeDef* TIMx,uint16_t prescale,uint16_t reload);
extern void TIM_DelayMicros(TIM_TypeDef* TIMx);
extern void TIM_InputCaptureInit(TIM_TypeDef* TIMx,uint16_t prescale,uint16_t reload);
extern void TIM_GetDutyCycle(TIM_TypeDef* TIMx,uint32_t* pDuty);

#endif //TIMER_H
