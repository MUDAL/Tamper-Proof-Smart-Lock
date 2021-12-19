#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

void setup() 
{
  SerialBT.begin("Smart Door");
}

void loop() 
{
  SerialBT.println("What's up");
  SerialBT.println("It's a good day to be alive");
  delay(2000);
}
