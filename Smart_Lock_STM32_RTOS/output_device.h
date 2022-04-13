#ifndef OUTPUT_DEVICE_H
#define OUTPUT_DEVICE_H

typedef enum
{
	LOCK = 0,
	BUZZER
}outputDev_t;

//Timeouts (in seconds)
#define TIMEOUT_LOCK			 8
#define TIMEOUT_BUZZER		10

extern void OutputDev_Init(void);
extern void OutputDev_Write(outputDev_t dev,bool state);
extern bool OutputDev_Read(outputDev_t dev);
extern void OutputDev_Timeout(outputDev_t dev,uint8_t* tCount,uint8_t timeout);

#endif //OUTPUT_DEVICE_H
