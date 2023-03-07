/**
 * This Library was originally written by Olivier Van den Eede (4ilo) in 2016.
 * Some refactoring was done and SPI support was added by Aleksander Alekseev (afiskon) in 2018.
 * https://github.com/afiskon/stm32-OLED
 * 
 * I modified and optimized the library to suit my application
 * OLED: SH1106
 */

#ifndef OLED_H_
#define OLED_H_

#define OLED_HEIGHT          64 // OLED height in pixels
#define OLED_WIDTH           132 // OLED width in pixels
#define OLED_BUFFER_SIZE   OLED_HEIGHT * OLED_WIDTH / 8

// Enumeration for screen colors
typedef enum 
{
	BLACK = 0x00, // Black color, no pixel
	WHITE = 0x01  // Pixel is set. Color depends on OLED
} OLED_COLOR;

extern void OLED_Init(void);
extern void OLED_UpdateScreen(void);
extern void OLED_DrawPixel(uint8_t x, uint8_t y, OLED_COLOR color);
extern char OLED_WriteChar(char ch, OLED_COLOR color);
extern char OLED_WriteString(char* str, OLED_COLOR color);
extern void OLED_SetCursor(uint8_t x, uint8_t y);
extern void OLED_ClearScreen(void);

#endif //OLED_H_
