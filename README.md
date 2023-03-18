# Tamper-proof smart door lock  

## Description  
The Tamper-Proof Smart Lock is a functional prototype designed and developed for a school project.   
It is a secure, tamper-proof locking system that can be accessed using multiple options for access control,   
including fingerprint, keypad, mobile application, and tactile buttons. The system also features tamper   
detection using an ultrasonic sensor, real-time responses to security breaches via SMS alerts, and a user   
interface (keypad + OLED) to aid man-machine communication.  

## Features  
1. Access control via:  
- Fingerprint sensor  
- Indoor and outdoor buttons  
- Keypad  
- Mobile app  
2. Tamper detection via ultrasonic sensor  

3. Intruder alert via SMS  

4. HMI (Keypad + OLED):    
- To add/remove fingerprints  
- To add phone number   
- To change password   

5. Timekeeping occurrences of intruder and tamper detection using a Real-Time Clock.         

## Block diagram  
![block_diagram drawio](https://user-images.githubusercontent.com/46250887/219887347-b687955b-d9e1-4335-9081-6a45ee540765.png)   

## Software  
1. Keil uVision 5 for programming the STM32 microcontroller.  
2. MIT App Inventor for mobile application development.   
3. FreeRTOS (for concurrent execution of tasks).   

## Software architecture  
![Layered architecture](https://user-images.githubusercontent.com/46250887/224175922-1c03e6f9-ef71-46db-a4cd-f25f12cb3bbb.png)  

The software for the Tamper-Proof Smart Lock was designed using a layered architecture consisting of:   

1. ``Device drivers`` (using CMSIS): UART, I2C, GPIO, DMA, SysTick, Timer, RCC.  
2. ``Libraries``: These abstract the device drivers and provide an interface (function calls) for the main application  
to control devices like the Fingerprint module, OLED, GSM module etc. without the need to know about their hardware configurations.  
3. ``Main application``: A mix of ``FreeRTOS`` and function calls from the ``Libraries`` layer.      

## Algorithm for the main application  
The main application is divided into functions, FreeRTOS tasks and a software timer. All the tasks are given equal priority. The embedded application is split into the following:  

1. ``Setup`` task: This is the first task that is created and executed by the kernel. Before this task is created, clock configurations are made for the microcontroller and its peripherals. The setup task is responsible for initializing the components of the smart lock (e.g. keypad, fingerprint scanner, GSM module, OLED) as well as creating other tasks. It is executed just once. After performing its duties, it will be deleted from the memory utilized by the kernel.  
2. ``HMI and fingerprint`` task: This task manages access control and security operations that are related to the HMI (keypad and OLED) and the fingerprint scanner. It handles the validation of passcodes and fingerprints. If three consecutive incorrect passcodes or fingerprints are detected, the following will occur:  
  a. Triggering of the buzzer and sending of SMS  
  b. Timekeeping of the intrusion event  
  c. Keypad and fingerprint scanner become temporarily disabled  
3. ``Buttons and tamper detection`` task: This task handles inputs from the two tactile buttons and drives the solenoid lock accordingly. It also detects and responds to an event of device tampering. In response to a tampering event, an SMS alert will be sent, the buzzer will be activated, and the time of occurrence will be recorded.    
4. ``Bluetooth`` task: This task is responsible for controlling the solenoid lock if the appropriate command is received via Bluetooth from a smartphone application. It also receives the passcode for the smart lock from the app. If the wrong passcode is sent 3 times, this task will trigger the buzzer, send an SMS alert to the authorized user, and temporarily disable communication with the mobile application. The Bluetooth task also sends records of intrusion and tampering attempts to the mobile application for security analysis.  
5. ``Software timer``: The software timer is provided by the FreeRTOS kernel. It executes a piece of code (known as the software timer’s callback function [33]) periodically. In this project, a software timer was created in order to handle timeouts. For example, if intrusion is detected by the HMI task, the buzzer will be triggered and the keypad and fingerprint scanner will be disabled for a specific time (8 seconds was used). The software timer’s callback function restores the system to its default state once this time expires i.e. buzzer will be turned off and the keypad and fingerprint module will be enabled in this case. The software timer was configured to trigger the callback every second.  

The algorithm for the main application (functions, FreeRTOS tasks and software timer) is represented with flowcharts. All flowcharts can be found in the ``Flowcharts`` folder in this repo.   

## Mobile application  
The app was developed using ``MIT App Inventor``. The ``aia`` file can be found in  
the ``App files`` folder of this repo. This file can be opened in MIT App Inventor  
to view the source code. The ``APK`` file is also present in the same folder as the  
``aia`` file. The app communicates with the smart lock via ``Bluetooth``.  
![smart_lock_app](https://user-images.githubusercontent.com/46250887/223219502-15954848-4aee-4515-b2ed-a8cfea6cb61c.jpg)   

## Hardware / Pinout
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

## Images of prototype  

![20230214_100150](https://user-images.githubusercontent.com/46250887/218695341-854c8ac2-5f41-40c8-8c51-136155ccb4ab.jpg)

![20230214_100155](https://user-images.githubusercontent.com/46250887/218695788-5e97ac57-4694-493b-84dd-bd5a98e1836b.jpg)

![20230214_100201](https://user-images.githubusercontent.com/46250887/218695384-c2354e53-0bb2-4330-9c62-f33aa4c8f0eb.jpg)

![20230214_100224](https://user-images.githubusercontent.com/46250887/218695462-d884f2eb-eaef-4798-b935-7daa5b8e202b.jpg)

![20230103_102348](https://user-images.githubusercontent.com/46250887/210335882-975661a0-fde1-4a5d-8249-8ec0525b2334.jpg)  

![20230103_102401](https://user-images.githubusercontent.com/46250887/210335912-574271c0-df18-410e-85a2-cb47c12eabd1.jpg)  

![20230103_102541](https://user-images.githubusercontent.com/46250887/210335947-0569a6c0-065f-4610-aa5f-1f1278c975b3.jpg)  

![20230103_102533](https://user-images.githubusercontent.com/46250887/210335978-96057d95-0fb2-41ab-ae00-ca529f5605fd.jpg)  

## Credits  
Acknowledgement of existing libraries that were ported and modified to satisfy the smart lock's requirements.   
1. Adafruit Fingerprint Sensor Library: https://github.com/adafruit/Adafruit-Fingerprint-Sensor-Library  
2. STM32 Library for OLEDs: https://github.com/afiskon/stm32-ssd1306  

## Recommendations  
1. Power optimization e.g. use of BLE instead of Bluetooth 
2. Provision of an outlet for backup DC supply  
3. Provision of a port to enable AC power supply (alternate power source)  
4. Design and develop a mobile application with better user interface  

