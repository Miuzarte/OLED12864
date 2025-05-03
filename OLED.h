#ifndef __OLED_H
#define __OLED_H

// 像素分辨率
#define OLED_RES_HORIZONTAL 128
#define OLED_RES_VERTICAL   64

// 字符长宽
#define OLED_CHAR_WIDTH  8
#define OLED_CHAR_HEIGHT 16

// 字符分辨率
#define OLED_CHARS_HORIZONTAL (OLED_RES_HORIZONTAL / OLED_CHAR_WIDTH) // 16
#define OLED_CHARS_VERTICAL   (OLED_RES_VERTICAL / OLED_CHAR_HEIGHT)  // 4

extern uint8_t OLED_DISPLAY_LINES; // 低于 4 的值会在下方留出保护区

void OLED_Init();
void OLED_WriteCommand(char command);
void OLED_WriteData(char data);
void OLED_SetCursor(uint8_t y, uint8_t x);
void OLED_Clear();
void OLED_ClearLine(uint8_t line, uint8_t columnStart, uint8_t columnEnd);
void OLED_ClearColumn(uint8_t column, uint8_t lineStart, uint8_t lineEnd);
uint8_t OLED_ShowChar(uint8_t line, uint8_t column, char Char);
uint8_t OLED_ShowString(uint8_t line, uint8_t column, char *str);
uint8_t OLED_ShowBool(uint8_t line, uint8_t column, uint8_t b);
uint8_t OLED_ShowNumNoZero(uint8_t line, uint8_t column, uint32_t number, uint8_t length);
uint8_t OLED_ShowNum(uint8_t line, uint8_t column, uint32_t number, uint8_t length);
uint8_t OLED_ShowSignedNum(uint8_t line, uint8_t column, int32_t number, uint8_t length);
uint8_t OLED_ShowHexNum(uint8_t line, uint8_t column, uint32_t number, uint8_t length);
void OLED_ShowBinNum(uint8_t line, uint8_t column, uint32_t number, uint8_t length);

#endif
