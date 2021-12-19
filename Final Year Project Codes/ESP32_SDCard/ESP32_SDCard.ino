#include "FS.h"
#include "SD.h"
#include "SPI.h"

#define MAX_BUFFER_LEN    20
char kpBuffer[20];
char btBuffer[20];

void ReadFile(fs::FS &fs, const char* path, char* readBuffer)
{
  int i = 0;
  File file = fs.open(path);
  while(file.available())
  {
    if(i < MAX_BUFFER_LEN)
    {
      readBuffer[i] = file.read();
      i++;
    }
  }
  readBuffer[i] = '\0';
  file.close();
}

void WriteFile(fs::FS &fs, const char* path, const char* message)
{
  File file = fs.open(path, FILE_WRITE);
  file.print(message);
  file.close();
}

void AppendFile(fs::FS &fs, const char* path, const char* message)
{
  File file = fs.open(path, FILE_APPEND);
  file.print(message);
  file.close();
}

void setup()
{
  Serial.begin(9600);
  SD.begin();
}

void loop()
{
  ReadFile(SD, "/kp.txt",kpBuffer);
  Serial.println(kpBuffer);
  ReadFile(SD, "/bt.txt",btBuffer);
  Serial.println(btBuffer);
  delay(4000);
}
