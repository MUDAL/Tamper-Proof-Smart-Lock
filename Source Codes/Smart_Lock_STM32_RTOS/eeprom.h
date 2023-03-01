#ifndef _EEPROM_H
#define _EEPROM_H
/**
* EEPROM: AT2C16
*/
#define PAGE_SIZE	16
/*Page ranges from 0 to 127*/
extern void EEPROM_StoreData(uint8_t* pData, uint32_t len, uint8_t initialPage);
extern void EEPROM_GetData(uint8_t* pData, uint32_t len, uint8_t initialPage);

#endif //_EEPROM_H
