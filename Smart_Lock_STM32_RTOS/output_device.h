#ifndef OUTPUT_DEVICE_H
#define OUTPUT_DEVICE_H

typedef enum
{
	LOCK = 0,
	BUZZER
}outputDev_t;

extern void OutputDev_Init(void);
extern void OutputDev_Write(outputDev_t dev,bool state);
extern bool OutputDev_Read(outputDev_t dev);

#endif //OUTPUT_DEVICE_H
