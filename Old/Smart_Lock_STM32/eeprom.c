#include "stm32f10x.h"                  // Device header
#include <stdbool.h>
#include "i2c.h"
#include "gpio.h"
#include "systick.h"
#include "eeprom.h"

static void EEPROM_WritePage(uint8_t pageAddr, uint8_t* pData, uint8_t len)
{
	if(len > PAGE_SIZE)
	{//Page size must not be exceeded.
		return;
	}
	//optional but ensures the user's input doesn't exceed 127
	pageAddr = pageAddr % 128; 
	//extract 3 MSB of EEPROM page address and store in EEPROM slave address
	uint8_t deviceAddr = ((((pageAddr >> 3) & 0x0E) | 0xA0)>>1);
	//extract 4 LSB of EEPROM page address and store in EEPROM word address
	uint8_t wordAddr = ((pageAddr & 0x0F) << 4);
	//I2C communication involving EEPROM slave and word address
	I2C_WriteMultiByte(I2C1,deviceAddr ,wordAddr, pData, len);
	//write cycle time
	SysTick_DelayMs(5);
}

static void EEPROM_ReadPage(uint8_t pageAddr, uint8_t* pReceiveBuffer, uint8_t len)
{
	if(len > PAGE_SIZE)
	{//Page size must not be exceeded.
		return;
	}
	//optional but ensures the user's input doesn't exceed 127
	pageAddr = pageAddr % 128; 
	//extract 3 MSB of EEPROM page address and store in EEPROM slave address
	uint8_t deviceAddr = ((((pageAddr >> 3) & 0x0E) | 0xA0)>>1);
	//extract 4 LSB of EEPROM page address and store in EEPROM word address
	uint8_t wordAddr = ((pageAddr & 0x0F) << 4);
	//I2C communication involving EEPROM slave and word address
	I2C_ReadMultiByte(I2C1,deviceAddr, wordAddr, pReceiveBuffer, len);
}

void EEPROM_StoreData(uint8_t* pData, uint32_t len, uint8_t initialPage)
{	
	uint8_t i = 0;
	uint8_t page = initialPage;
	//number of pages to allocate for storing contents of pBuffer
	uint8_t numberOfPages = len / PAGE_SIZE; 
	//remaining bytes of pBuffer (which is less than a page) to be stored
	uint8_t numberOfBytesLeft = len % PAGE_SIZE; 
	
	//A page stores 16 bytes.
	//If data > 16 bytes, the data is split into blocks of 16 bytes...
	//with each block stored in a page. The storage of blocks in pages....
	//is sequential.
	while(i < numberOfPages)
	{
		EEPROM_WritePage(page+i,&pData[PAGE_SIZE*i],PAGE_SIZE);
		i++;
	}
	EEPROM_WritePage(page+i,&pData[PAGE_SIZE*i],numberOfBytesLeft);	
}

void EEPROM_GetData(uint8_t* pData, uint32_t len, uint8_t initialPage)
{
	uint8_t i = 0;
	uint8_t page = initialPage;
	//number of completely filled pages EEPROM pages to read 
	uint8_t numberOfPages = len / PAGE_SIZE; 
	//unfilled EEPROM page
	uint8_t numberOfBytesLeft = len % PAGE_SIZE; 
	
	//Reading from the EEPROM sequentially from all filled pages...
	//to the unfilled page(if any).
	while(i < numberOfPages)
	{
		EEPROM_ReadPage(page+i,&pData[PAGE_SIZE*i],PAGE_SIZE);
		i++;
	}
	EEPROM_ReadPage(page+i,&pData[PAGE_SIZE*i],numberOfBytesLeft);	
}
