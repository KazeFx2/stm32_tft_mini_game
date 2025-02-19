#ifndef __OLED_H
#define __OLED_H

#include "gpio.h"
#include "spi.h"
#include "sys.h"

#define OLED_RES PD(9)
#define OLED_DC PD(11)
#define OLED_CS PD(13)

#define P_OLED_RES PDout(9)
#define P_OLED_DC PDout(11)
#define P_OLED_CS PDout(13)

#define OLED_CS_Set() P_OLED_CS = 1
#define OLED_CS_Clr() P_OLED_CS = 0
#define OLED_DC_Set() P_OLED_DC = 1
#define OLED_DC_Clr() P_OLED_DC = 0
#define OLED_RES_Set() P_OLED_RES = 1
#define OLED_RES_Clr() P_OLED_RES = 0

#define OLED_CMD 0
#define OLED_DATA 1

void OLED_ClearPoint(u8 x, u8 y);
void OLED_ColorTurn(u8 i);
void OLED_DisplayTurn(u8 i);
void OLED_WR_Byte(u8 dat, u8 cmd);
void OLED_DisPlay_On(void);
void OLED_DisPlay_Off(void);
void OLED_Refresh(void);
void OLED_Clear(void);
void OLED_DrawPoint(u8 x, u8 y);
void OLED_DrawLine(u8 x1, u8 y1, u8 x2, u8 y2);
void OLED_DrawCircle(u8 x, u8 y, u8 r);
void OLED_ShowChar(u8 x, u8 y, u8 chr, u8 size1);
void OLED_ShowString(u8 x, u8 y, char *chr, u8 size1);
void OLED_ShowNum(u8 x, u8 y, u32 num, u8 len, u8 size1);
void OLED_ShowChinese(u8 x, u8 y, u8 num, u8 size1);
void OLED_ScrollDisplay(u8 num, u8 space);
void OLED_WR_BP(u8 x, u8 y);
void OLED_ShowPicture(u8 x0, u8 y0, u8 x1, u8 y1, u8 BMP[]);
void oled_init(int spi);

#endif
