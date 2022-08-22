#include <Arduino.h>
#include "sd_card.h"

/*
 * @brief Reads data from a file stored in an SD card
 * @param fs: 
 * @param path: path to the file to be read
 * @param readBuffer: Buffer to store the contents of the file
 * @return None
*/
void SD_ReadFile(fs::FS &fs, const char* path, char* readBuffer)
{
  int i = 0;
  File file = fs.open(path);
  while(file.available())
  {
    if(i < SD_MAX_LEN)
    {
      readBuffer[i] = file.read();
      i++;
    }
  }
  readBuffer[i] = '\0';
  file.close();
}

/*
 * @brief Writes data to a file stored in an SD card
 * @param fs: 
 * @param path: path to the file to be written
 * @param message: data to be written to the file
 * @return None
*/
void SD_WriteFile(fs::FS &fs, const char* path, const char* message)
{
  File file = fs.open(path, FILE_WRITE);
  file.print(message);
  file.close();
}

/*
 * @brief Appends data to a file stored in an SD card
 * @param fs: 
 * @param path: path to the file to be written
 * @param message: data to be appended to the file
 * @return None
*/
void SD_AppendFile(fs::FS &fs, const char* path, const char* message)
{
  File file = fs.open(path, FILE_APPEND);
  file.print(message);
  file.close();
}

