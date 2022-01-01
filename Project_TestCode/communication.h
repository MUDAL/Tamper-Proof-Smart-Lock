#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include "BluetoothSerial.h"
#define MAX_BT_SERIAL_LEN   20

extern BluetoothSerial SerialBT;
extern void SendSMS(char* phoneNumber,char* message);
extern void GetCountryCodePhoneNo(char* countryCodePhoneNo,char* phoneNumber);
extern void GetBluetoothData(char* bluetoothBuffer,int maxLen);

#endif //COMMUNICATION_H

