#include "stm32f10x.h"                  // Device header
#include "i2c.h"

void I2C_Init(I2C_TypeDef* I2Cx, 
							uint32_t i2cClock, 
							uint32_t i2cCCR, 
							uint32_t i2cTRISE)
{
	//I2C software reset to eliminate BUSY flag glitch
	I2Cx->CR1 |= I2C_CR1_SWRST;
	I2Cx->CR1 &= ~I2C_CR1_SWRST;
	I2Cx->CR2 |= i2cClock;
	I2Cx->CCR = i2cCCR;
	I2Cx->TRISE = i2cTRISE;
	I2Cx->CR1 |= I2C_CR1_PE; //Enable I2C peripheral
}

void I2C_Write(I2C_TypeDef* I2Cx,
							 uint8_t slaveAddr,
							 uint8_t data)
{
	volatile uint32_t read_I2C_SR2;
	
	while((I2Cx->SR2 & I2C_SR2_BUSY) == I2C_SR2_BUSY); //wait for I2C busy bit to be cleared	 
	I2Cx->CR1 |= I2C_CR1_START; //Generate start condition
	while((I2Cx->SR1 & I2C_SR1_SB) != I2C_SR1_SB);//wait for start bit to be set
	I2Cx->DR = slaveAddr << 1; //slave address
	while((I2Cx->SR1 & I2C_SR1_ADDR) != I2C_SR1_ADDR);//wait for ADDR bit to be set
	read_I2C_SR2 = I2Cx->SR2;
	while((I2Cx->SR1 & I2C_SR1_TXE) != I2C_SR1_TXE);//wait for TXE bit to be set
	I2Cx->DR = data;
	
	while(((I2Cx->SR1 & I2C_SR1_TXE) != I2C_SR1_TXE) || 
				 ((I2Cx->SR1 & I2C_SR1_BTF) != I2C_SR1_BTF)); 
	I2Cx->CR1 |= I2C_CR1_STOP; 	
}

void I2C_WriteByte(I2C_TypeDef* I2Cx, 
									 uint8_t slaveAddr,
									 uint8_t regAddr,
									 uint8_t data)
{
	volatile uint32_t read_I2C_SR2;
	
	while((I2Cx->SR2 & I2C_SR2_BUSY) == I2C_SR2_BUSY); //wait for I2C busy bit to be cleared	 
	I2Cx->CR1 |= I2C_CR1_START; //Generate start condition
	while((I2Cx->SR1 & I2C_SR1_SB) != I2C_SR1_SB);//wait for start bit to be set
	I2Cx->DR = slaveAddr << 1; //slave address
	while((I2Cx->SR1 & I2C_SR1_ADDR) != I2C_SR1_ADDR);//wait for ADDR bit to be set
	read_I2C_SR2 = I2Cx->SR2;
	while((I2Cx->SR1 & I2C_SR1_TXE) != I2C_SR1_TXE);//wait for TXE bit to be set
	I2Cx->DR = regAddr;
	while((I2Cx->SR1 & I2C_SR1_TXE) != I2C_SR1_TXE); 
	I2Cx->DR = data;
	while( ((I2Cx->SR1 & I2C_SR1_TXE) != I2C_SR1_TXE) || 
				 ((I2Cx->SR1 & I2C_SR1_BTF) != I2C_SR1_BTF) ); 
	I2Cx->CR1 |= I2C_CR1_STOP; 
		
}

void I2C_WriteMultiByte(I2C_TypeDef* I2Cx, 
												uint8_t slaveAddr,
												uint8_t regAddr,
												uint8_t* pData,
												uint32_t length)

{
	volatile uint32_t read_I2C_SR2;
	
	while((I2Cx->SR2 & I2C_SR2_BUSY) == I2C_SR2_BUSY); //wait for I2C busy bit to be cleared	 
	I2Cx->CR1 |= I2C_CR1_START; //Generate start condition
	while((I2Cx->SR1 & I2C_SR1_SB) != I2C_SR1_SB);//wait for start bit to be set
	I2Cx->DR = slaveAddr << 1; //slave address
	while((I2Cx->SR1 & I2C_SR1_ADDR) != I2C_SR1_ADDR);//wait for ADDR bit to be set
	read_I2C_SR2 = I2Cx->SR2;
	while((I2Cx->SR1 & I2C_SR1_TXE) != I2C_SR1_TXE);//wait for TXE bit to be set
	I2Cx->DR = regAddr;
	
	for(uint32_t i = 0; i < length; i++)
	{
		while((I2Cx->SR1 & I2C_SR1_TXE) != I2C_SR1_TXE); 
		I2Cx->DR = pData[i];
	}
	
	while( ((I2Cx->SR1 & I2C_SR1_TXE) != I2C_SR1_TXE) || 
				 ((I2Cx->SR1 & I2C_SR1_BTF) != I2C_SR1_BTF) ); 
	I2Cx->CR1 |= I2C_CR1_STOP; 
}

void I2C_ReadByte(I2C_TypeDef* I2Cx,
									uint8_t slaveAddr,
									uint8_t regAddr,
									uint8_t* pData)
{
	volatile uint32_t read_I2C_SR2;
	
	while((I2Cx->SR2 & I2C_SR2_BUSY) == I2C_SR2_BUSY); //wait for I2C busy bit to be cleared 
	I2Cx->CR1 |= I2C_CR1_START; //Generate start condition
	while((I2Cx->SR1 & I2C_SR1_SB) != I2C_SR1_SB);//wait for start bit to be set
	I2Cx->DR = slaveAddr << 1; //slave address
	while((I2Cx->SR1 & I2C_SR1_ADDR) != I2C_SR1_ADDR); //wait for ADDR bit to be set
	read_I2C_SR2 = I2Cx->SR2;
	while((I2Cx->SR1 & I2C_SR1_TXE) != I2C_SR1_TXE); //wait for TXE bit to be set
	I2Cx->DR = regAddr;
		
	while((I2Cx->SR1 & I2C_SR1_TXE) != I2C_SR1_TXE); 
	I2Cx->CR1 |= I2C_CR1_START; 
	while((I2Cx->SR1 & I2C_SR1_SB) != I2C_SR1_SB);
	I2Cx->DR = slaveAddr << 1 | 1;
	
	while((I2Cx->SR1 & I2C_SR1_ADDR) != I2C_SR1_ADDR); 
	I2Cx->CR1 &= ~I2C_CR1_ACK;//Send NACK
	read_I2C_SR2 = I2Cx->SR2;
		
	I2Cx->CR1 |= I2C_CR1_STOP; //Send STOP
	while((I2Cx->SR1 & I2C_SR1_RXNE) != I2C_SR1_RXNE); //Wait for RXNE bit to be set
	*pData = I2Cx->DR;
		
}

void I2C_Read2Bytes(I2C_TypeDef* I2Cx, 
										uint8_t slaveAddr,
										uint8_t regAddr,
										uint8_t* pData)
{
	volatile uint32_t read_I2C_SR2;
	
	while((I2Cx->SR2 & I2C_SR2_BUSY) == I2C_SR2_BUSY); //wait for I2C busy bit to be cleared 
	I2Cx->CR1 |= I2C_CR1_START; //Generate start condition
	while((I2Cx->SR1 & I2C_SR1_SB) != I2C_SR1_SB);//wait for start bit to be set
			
	I2Cx->DR = slaveAddr << 1; //slave address
	while((I2Cx->SR1 & I2C_SR1_ADDR) != I2C_SR1_ADDR); //wait for ADDR bit to be set
	read_I2C_SR2 = I2Cx->SR2;
	while((I2Cx->SR1 & I2C_SR1_TXE) != I2C_SR1_TXE);//wait for TXE bit to be set
	I2Cx->DR = regAddr;
		
	while((I2Cx->SR1 & I2C_SR1_TXE) != I2C_SR1_TXE); 
	I2Cx->CR1 |= I2C_CR1_START; 
	while((I2Cx->SR1 & I2C_SR1_SB) != I2C_SR1_SB);
	I2Cx->DR = slaveAddr << 1 | 1;
	
	while((I2Cx->SR1 & I2C_SR1_ADDR) != I2C_SR1_ADDR); 
	I2Cx->CR1 &= ~I2C_CR1_ACK;//Send NACK
	I2Cx->CR1 |= I2C_CR1_POS; 
	read_I2C_SR2 = I2Cx->SR2;
		
	while((I2Cx->SR1 & I2C_SR1_BTF) != I2C_SR1_BTF);//Wait for BTF bit to be set
	I2Cx->CR1 |= I2C_CR1_STOP; //Send STOP
	pData[0] = I2Cx->DR;
	pData[1] = I2Cx->DR;
	
}

void I2C_ReadMultiByte(I2C_TypeDef* I2Cx,
											 uint8_t slaveAddr,
											 uint8_t regAddr,
											 uint8_t* pData, 
											 uint32_t length)
{
	if(length == 0)
	{//Invalid input
		return;
	}
	
	else if(length == 1)
	{
		I2C_ReadByte(I2Cx,slaveAddr,regAddr,pData);
	}
	
	else if(length == 2)
	{
		I2C_Read2Bytes(I2Cx,slaveAddr,regAddr,pData);
	}
	
	else
	{
		volatile uint32_t read_I2C_SR2;
		while((I2Cx->SR2 & I2C_SR2_BUSY) == I2C_SR2_BUSY); //wait for I2C busy bit to be cleared
		I2Cx->CR1 |= I2C_CR1_START; //Generate start condition
		while((I2Cx->SR1 & I2C_SR1_SB) != I2C_SR1_SB); //wait for start bit to be set
				
		I2Cx->DR = slaveAddr << 1; //slave address
		while((I2Cx->SR1 & I2C_SR1_ADDR) != I2C_SR1_ADDR); //wait for ADDR bit to be set
		read_I2C_SR2 = I2Cx->SR2;
		while((I2Cx->SR1 & I2C_SR1_TXE) != I2C_SR1_TXE); //wait for TXE bit to be set
		I2Cx->DR = regAddr;
			
		while((I2Cx->SR1 & I2C_SR1_TXE) != I2C_SR1_TXE); 
		I2Cx->CR1 |= I2C_CR1_START; 
		while((I2Cx->SR1 & I2C_SR1_SB) != I2C_SR1_SB); 
		I2Cx->DR = slaveAddr << 1 | 1;
		
		while((I2Cx->SR1 & I2C_SR1_ADDR) != I2C_SR1_ADDR); 
		read_I2C_SR2 = I2Cx->SR2;
		I2Cx->CR1 |= I2C_CR1_ACK; //Send ACK
		
		//Read incoming data
		for(uint32_t i = 0; i < length - 3; i++)
		{
			while((I2Cx->SR1 & I2C_SR1_RXNE) != I2C_SR1_RXNE); //Wait for RXNE bit to be set
			pData[i] = I2Cx->DR;
		}
		
		while((I2Cx->SR1 & I2C_SR1_BTF) != I2C_SR1_BTF);//Wait for BTF bit to be set
		I2Cx->CR1 &= ~I2C_CR1_ACK;//Send NACK	
		pData[length - 3] = I2C1->DR;
			
		while((I2Cx->SR1 & I2C_SR1_BTF) != I2C_SR1_BTF);
		I2Cx->CR1 |= I2C_CR1_STOP; //Send STOP
		pData[length - 2] = I2Cx->DR;
		pData[length - 1] = I2Cx->DR;
	}
}
