#ifndef _RTC_H
#define _RTC_H
/*
 * RTC: DS3231
*/
enum Period
{
	RTC_PERIOD_AM = 0,
	RTC_PERIOD_PM,
	RTC_NO_PERIOD
};

typedef struct
{
	uint8_t seconds;
	uint8_t minutes;
	uint8_t hours;
	uint8_t period;
}ds3231_t;

extern void RTC_GetTime(ds3231_t* pTime);
extern void RTC_SetTime(uint8_t hour, uint8_t min);
extern void RTC_12HourFormat(uint8_t periodOfDay);
extern void RTC_24HourFormat(void);

#endif //_RTC_H
