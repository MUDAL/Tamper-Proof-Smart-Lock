#include <Arduino.h>
#include "communication.h"

void SendSMS(char* message)
{
  Serial2.println("AT+CMGF=1");
  delay(500);
  Serial2.println("AT+CMGS=\"+2349058041373\"");
  delay(500);
  Serial2.println(message);
  delay(500);
  Serial2.write(26); //Command termination
  delay(500);
}

void GetBluetoothData(char* bluetoothBuffer)
{
  int i = 0;
  bool bufferCleared = false;
  char clearSerialBT = '\0';
  while(SerialBT.available() > 0)
  {
    if(!bufferCleared)
    {
      memset(bluetoothBuffer,'\0',MAX_BT_SERIAL_LEN);
      bufferCleared = true;
    }
    if(i < MAX_BT_SERIAL_LEN)
    {
      bluetoothBuffer[i] = SerialBT.read();
    }
    else
    {
      clearSerialBT = SerialBT.read();
    }
    i++;
  }
}

