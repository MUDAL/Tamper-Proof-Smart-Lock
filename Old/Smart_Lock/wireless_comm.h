#ifndef WIRELESS_COMM_H
#define WIRELESS_COMM_H

#include "BluetoothSerial.h"

extern BluetoothSerial SerialBT;
extern void SendSMS(char* phoneNumber,char* message);
extern void GetCountryCodePhoneNo(char* countryCodePhoneNo,char* phoneNumber);
extern void GetBluetoothData(char* bluetoothBuffer,int maxLen);

#endif //WIRELESS_COMM_H

