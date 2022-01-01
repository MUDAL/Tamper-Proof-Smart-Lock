#include <Arduino.h>
#include "communication.h"

/*
 * @brief Sends an SMS to a specified phone number.
 * @param phoneNumber: number the SMS will be sent to.
 * @param message: SMS to be sent.
 * @return None
*/
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

/*
 * @brief Gets country code representation of a phone number.
 * @example 08012312311(normal form) --> +2348012312311(country code)
 * @param countryCodePhoneNo: country code phone number
 * @param phoneNumber: phone number in normal form
 * @return None
*/
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

/*
 * @brief Gets data from a bluetooth device and stores in a buffer.
 * @param bluetoothBuffer: Buffer to store the data received via bluetooth.
 * @param maxLen: maximum permissible length of data to be received.
 * @return None
*/
void GetBluetoothData(char* bluetoothBuffer,int maxLen)
{
  int i = 0;
  while(SerialBT.available() > 0)
  {
    if(i < maxLen)
    {
      bluetoothBuffer[i] = SerialBT.read();
    }
    else
    {//removes leftover bytes from SerialBT buffer
      char clearSerialBT = SerialBT.read(); 
    }
    i++;
  }
}

