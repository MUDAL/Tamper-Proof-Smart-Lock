#ifndef BUTTON_H
#define BUTTON_H

typedef struct
{
	GPIO_TypeDef* GPIOx;
	uint8_t portLevel;
	uint16_t pin;
	uint32_t config;
	bool prevPressed;
}button_t;

extern void Button_Init(button_t* pButton);
extern bool Button_IsPressedOnce(button_t* pButton);

#endif //BUTTON_H
