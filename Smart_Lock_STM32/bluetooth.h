#ifndef BLUETOOTH_H
#define BLUETOOTH_H

/**
* Bluetooth: HC05/6
*/
#define BT_BUFFERSIZE	20 //Max Rx buffer size
typedef enum
{
	NO_DATA = 0,
	BUFFER_FULL,
	IDLE_LINE
}btStatus_t;
	
extern void BT_Init(void);
extern void BT_RxBufferInit(uint8_t* pBuffer,uint8_t bufferSize);
extern void BT_Transmit(char* pData);
extern btStatus_t BT_Receive(void);

#endif //BLUETOOTH_H
