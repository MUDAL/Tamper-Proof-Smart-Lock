#include "stm32f10x.h"                  // Device header
#include <stdbool.h>
#include "gpio.h"
#include "i2c.h"
#include "rtc.h"

/*
NB: 
1.) BCDToHex() is used to convert time data (seconds,minutes and hours) from the RTC 
to a form usable by the main application.

2.) HexToBCD() is used to convert time data (seconds,minutes and hours) from a form used
by the application (i.e. binary or decimal or hexadecimal etc.) to a form usable by the
RTC (i.e. BCD)
*/

//Register addresses for the RTC module (DS3231)
enum RegAddr
{
	RTC_ADDR = 0x68,
	SEC_REG_ADDR = 0x00,
	MIN_REG_ADDR = 0x01,
	HOUR_REG_ADDR = 0x02,
	ALARM2_MIN_REG_ADDR = 0x0B,
	ALARM2_HOUR_REG_ADDR = 0x0C,
	ALARM2_DY_DT_REG_ADDR = 0x0D,
	CONTROL_REG_ADDR = 0x0E,
	STATUS_REG_ADDR = 0x0F
};

/**
	* @brief Converts BCD to Hexadecimal
  * @param bcd: BCD value to be converted to Hex
	* @return Hex equivalent of 'bcd'
*/
static uint8_t BCDToHex(uint8_t bcd)
{
	uint8_t hex;
	hex = (((bcd & 0xF0)>>4)*10) + (bcd&0x0F);
	return hex;
}

/**
@brief Converts Hexadecimal to BCD  

@param hex: this parameter is passed with a value to be
converted to BCD. The argument doesn't necessarily need
to be hexadecimal. It can be binary, decimal etc. If its
any number system other than hexadecimal, the compiler
would be able to interprete the number system as hexadecimal
and the algorithm would still behave the same.  

@return BCD equivalent of the argument passed to 'hex'  

*/
static uint8_t HexToBCD(uint8_t hex)
{
	uint8_t bcd;
	uint8_t multipleOfTen = 0;
	while(hex >= 10)
	{
		hex -= 10;
		multipleOfTen++;
	}
	bcd = ((multipleOfTen<<4) | hex);
	return bcd;
}

/**
	* @brief Gets current time from the RTC module
	* @param pTime: pointer to struct containing information on time.
	* After calling this function, the current time is stored in the...
  * struct 'pTime' points to.
	* @return None
*/
void RTC_GetTime(ds3231_t* pTime)
{
	
	uint8_t timeBCD[3]; //sec,min,hour
	uint8_t periodOfDay;
	I2C_ReadMultiByte(I2C1,RTC_ADDR,SEC_REG_ADDR,timeBCD,3);
	if((timeBCD[2] & (1<<6)) == (1<<6))
	{
		/*
		1.)read AM/PM status of 12 hour clock
		2.)if 12 hour format is the current clock format, 
		read only bits 4-0 of ds3231 hour register*/
		periodOfDay = ((timeBCD[2] & (1<<5)) >> 5);
		timeBCD[2] = (timeBCD[2] & 0x1F);
	}
	else
	{
		/*
		1.)no AM/PM status for 24 hour clock
		2.)if 24 hour format is the current clock format, 
		read only bits 5-0 of ds3231 hour register*/
		periodOfDay = RTC_NO_PERIOD;
		timeBCD[2] = (timeBCD[2] & 0x3F); 
	}
	pTime->seconds = BCDToHex(timeBCD[0]);
	pTime->minutes = BCDToHex(timeBCD[1]);
	pTime->hours = BCDToHex(timeBCD[2]);
	pTime->period = periodOfDay;
}

/**
	* @brief Sets the time
	* @param hour: hour
	* @param min: minute
	* @return None
*/
void RTC_SetTime(uint8_t hour,uint8_t min)
{
	uint8_t prevHoursBCD;
	uint8_t timeBCD[3] = {0,0,0}; //sec,min,hour
	timeBCD[1] = HexToBCD(min);
	timeBCD[2] = HexToBCD(hour);
	I2C_ReadByte(I2C1, RTC_ADDR, HOUR_REG_ADDR, &prevHoursBCD);
	/* 0xE0 preserves settings of the ds3231 hour register
	so that a write to the register doesn't clear the hour configurations.
	*/
	timeBCD[2] = (timeBCD[2] | (prevHoursBCD & 0xE0));
	I2C_WriteMultiByte(I2C1,RTC_ADDR,SEC_REG_ADDR,timeBCD,3);
}

/**
	* @brief Sets the RTC module to 12-hour format
  * @param periodOfDay: @arg RTC_PERIOD_PM (sets the period to PM)
	*											@arg RTC_PERIOD_AM (sets the period to AM)
	* An invalid argument to 'periodOfDay' will automatically..... 
	* set the period to AM.
	* @return None
*/
void RTC_12HourFormat(uint8_t periodOfDay)
{
	uint8_t hoursBCD;
	I2C_ReadByte(I2C1, RTC_ADDR, HOUR_REG_ADDR, &hoursBCD);
	if(periodOfDay == RTC_PERIOD_PM)
	{
		hoursBCD |= ((1<<6) | (1<<5));
	}
	else
	{
		hoursBCD &= ~(1<<5);
		hoursBCD |= (1<<6);
	}
	I2C_WriteByte(I2C1, RTC_ADDR, HOUR_REG_ADDR, hoursBCD);
}

/**
	* @brief Sets the RTC module to 24-hour format
	* @param None
	* @return None
*/
void RTC_24HourFormat(void)
{
	uint8_t hoursBCD;
	I2C_ReadByte(I2C1, RTC_ADDR, HOUR_REG_ADDR, &hoursBCD);
	hoursBCD &= ~((1<<6) | (1<<5));
	I2C_WriteByte(I2C1, RTC_ADDR, HOUR_REG_ADDR, hoursBCD);
}

