#ifndef BUTTON_H
#define BUTTON_H

typedef enum
{
	INDOOR = 0,
	OUTDOOR
}button_t;

extern void Button_Init(void);
extern bool Button_IsPressedOnce(button_t button);

#endif //BUTTON_H
