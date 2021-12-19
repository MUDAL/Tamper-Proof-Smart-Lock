#ifndef SD_CARD_LIB_H
#define SD_CARD_LIB_H

#include "FS.h"
#include "SD.h"
#include "SPI.h"

#ifndef MAX_PASSWORD_LEN
#define MAX_PASSWORD_LEN  20
#endif

extern void ReadFile(fs::FS &fs, const char* path, char* readBuffer);
extern void WriteFile(fs::FS &fs, const char* path, const char* message);
extern void AppendFile(fs::FS &fs, const char* path, const char* message);

#endif //SD_CARD_LIB_H

