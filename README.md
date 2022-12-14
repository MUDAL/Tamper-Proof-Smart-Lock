# Tamper-proof smart door lock  

## Description  
A smart lock with the following features:  
1. Access control via:  
- Fingerprint sensor  
- Indoor and outdoor buttons  
- Keypad  
- Mobile app  
2. Tamper detection via ultrasonic sensor  
3. Intruder alert via SMS  
4. HMI (Keypad + OLED):    
a. To add/remove fingerprints  
b. To add phone number   
c. To change password   
5. Temporary timekeeping intruder and   
tamper detection occurrences.    

## Mobile app  
The app was developed using ``MIT App Inventor``. The ``aia`` file can be found in  
the ``App files`` folder of this repo. This file can be opened in MIT App Inventor  
to view the source code. The ``APK`` file is also present in the same folder as the  
``aia`` file.  

## Pinout
|  N   |  Component     | STM32                            |  
| :------: | :------: | :------: |  
1   |   HC05 Bluetooth Module     |  USART3 (PB10, PB11)                           |   
2   |   Indoor Button       |  PA0                            |   
3   |   Outdoor Button       |  PA1   |  
4   |   AT24C16 EEPROM       |  I2C1 (PB6, PB7)   |    
5   |   DY50 Fingerprint Sensor   | USART1 (PA9, PA10)   |   
6   |   SIM800L GSM Module      |   USART2 Tx (PA2)   |   
7   |   Ultrasonic Sensor           |   Trig pin: PA4, Echo pin: PA6   |     
8   |   4x4 Keypad       |  PB0, PB1, PB5, PB8, PB9, PB12, PB13, PB14  |     
9   |   SH1106  OLED    |   I2C1 (PB6, PB7)  |   
10  |   Solenoid Lock   |   PA3   |     
11  |   Buzzer      |   PA8   |     
12  |   DS3231  RTC   |   I2C1 (PB6, PB7)  |   

## Important SIM800L points  
1. The module should be powered with sufficient voltage (4.1 to 4.4v).  
2. It should also be powered with a supply that can source 2A or more.  
3. The wires connecting the power pins of the module to the supply should be as short as possible   
in order to minimize resistance. This helps in preventing unwanted resets as the resistance introduced    
by longer wires could limit the amount of current the module would draw.   

## Images  

![Finished work front view](https://user-images.githubusercontent.com/46250887/205495787-6a811678-a764-4d76-84f7-8808ee10ea26.jpg)  

![Finished work](https://user-images.githubusercontent.com/46250887/205495841-9cb31fff-6728-400c-9580-214edff1c8ab.jpg)  

![Finished work side view 2](https://user-images.githubusercontent.com/46250887/205496066-7b8d5151-9c68-45fd-b9ab-18db715630c9.jpg)  

![Finished work side view 1](https://user-images.githubusercontent.com/46250887/205496075-09c605d2-d992-4f6d-afba-44d772edaa56.jpg)  



