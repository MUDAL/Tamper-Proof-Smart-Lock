#ifndef _SPI_H
#define _SPI_H

#define SPI_CLOCK_DIVIDER_32				SPI_CR1_BR_2
#define SPI_SOFTWARE_SLAVE_MGMNT		SPI_CR1_SSM
#define SPI_INTERNAL_SLAVE_SEL			SPI_CR1_SSI
#define SPI_MASTER_MODE							SPI_CR1_MSTR
#define SPI_ENABLE									SPI_CR1_SPE

extern void SPI_Init(SPI_TypeDef* SPIx, uint32_t config);
extern void SPI_WriteByte(SPI_TypeDef* SPIx, uint8_t* pByte);
extern void SPI_WriteBytes(SPI_TypeDef* SPIx, uint8_t* pData, uint16_t length);
extern uint8_t SPI_ReceiveByte(SPI_TypeDef* SPIx);
extern void SPI_Transceive(SPI_TypeDef* SPIx, uint8_t* txData, uint8_t* rxData);
extern void SPI_ChipSelect(GPIO_TypeDef* nssGpioPort, uint8_t nssGpioPin);
extern void SPI_ChipDeselect(GPIO_TypeDef* nssGpioPort, uint8_t nssGpioPin);

#endif //_SPI_H
