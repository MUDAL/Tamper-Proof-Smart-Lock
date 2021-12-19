#include <Arduino.h>
#include "gsm.h"

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

