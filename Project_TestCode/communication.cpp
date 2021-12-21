#include <Arduino.h>
#include "communication.h"

void SendSMS(char* phoneNumber,char* message)
{
  char AT_CMGS_Command[27] = "AT+CMGS=\"";
  Serial2.println("AT+CMGF=1");
  delay(500);
  strcat(AT_CMGS_Command,phoneNumber);
  strcat(AT_CMGS_Command,"\"\r\n");
  Serial2.print(AT_CMGS_Command);
  delay(500);
  Serial2.println(message);
  delay(500);
  Serial2.write(26); //Command termination
  delay(500);
}

void GetCountryCodePhoneNo(char* countryCodePhoneNo,char* phoneNumber)
{
  const char countryCode[5] = "+234";
  for(int i = 0; i < 4; i++)
  {
    countryCodePhoneNo[i] = countryCode[i];
  }
  for(int i = 4; i < 14; i++)
  {
    countryCodePhoneNo[i] = phoneNumber[i-3];
  }
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

