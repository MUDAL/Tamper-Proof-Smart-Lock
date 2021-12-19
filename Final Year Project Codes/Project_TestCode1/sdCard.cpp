#include <Arduino.h>
#include "sdCard.h"

void ReadFile(fs::FS &fs, const char* path, char* readBuffer)
{
  int i = 0;
  File file = fs.open(path);
  while(file.available())
  {
    if(i < MAX_PASSWORD_LEN)
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

