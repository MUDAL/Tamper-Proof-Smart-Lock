# Tamper-proof smart door lock  

## Pinout
|  N   |  Component     | STM32                            |  
| :------: | :------: | :------: |  
1   |   HC05 Bluetooth Module     |  USART3 (PB10, PB11)                           |   
2   |   Indoor Button       |  PA0                            |   
3   |   Outdoor Button       |  PA1   |  
4   |   AT24C16 EEPROM       |  I2C1 (PB6, PB7)   |    
5   |   DY50 Fingerprint Sensor   | USART1 (PA9, PA10)   |   
6   |   SIM800L GSM Module      |   USART2 Tx (PA2)   |   
7   |   IR Sensor           |   PA4   |     
8   |   4x4 Keypad       |  PB0, PB1, PB5, PB8, PB9, PB12, PB13, PB14  |     
9   |   SH1106  OLED    |   I2C1 (PB6, PB7)  |   
10  |   Solenoid Lock   |   PA3   |     
11  |   Buzzer      |   PA8   |     
12  |   DS3231  RTC   |   I2C1 (PB6, PB7)  |   

## Modifications to make  
1. Replace IR sensor with ultrasonic sensor.  
2. Setup a hardware timer for uS timing. (Timer interrupt with 100kHz repetition rate).  
For the sensor's library, the function to get distance can be implemented as a ``callback`` which would  
be called by the timer's interrupt handler.  

