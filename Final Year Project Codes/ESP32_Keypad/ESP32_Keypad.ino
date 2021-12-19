#include "keypad.h"
#include "BluetoothSerial.h"

int rowPins[NUMBER_OF_ROWS] = {16,22,32,33};
int columnPins[NUMBER_OF_COLUMNS] = {25,26,27,14};
Keypad keypad(rowPins,columnPins);
BluetoothSerial SerialBT;

void setup() 
{
  // put your setup code here, to run once:
  Serial.begin(9600);
  SerialBT.begin("Smart Door");
}

void loop() 
{
  // put your main code here, to run repeatedly:
  char key = keypad.GetChar();
  if(key != '\0')
  {
    Serial.println(key);
    SerialBT.println(key);
  }
}
