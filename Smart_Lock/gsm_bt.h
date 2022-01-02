#ifndef GSM_BT_H
#define GSM_BT_H

#include "BluetoothSerial.h"

extern BluetoothSerial SerialBT;
extern void SendSMS(char* phoneNumber,char* message);
extern void GetCountryCodePhoneNo(char* countryCodePhoneNo,char* phoneNumber);
extern void GetBluetoothData(char* bluetoothBuffer,int maxLen);

#endif //GSM_BT_H

