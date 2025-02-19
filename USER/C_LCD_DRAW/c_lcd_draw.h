#ifndef __C_LCD_DRAW_H
#define __C_LCD_DRAW_H

#include "stm32f10x.h"

#define LCD_WIDTH 240
#define LCD_HEIGHT 240

typedef uint16_t color_t;

#define BIT_5 0x1f
#define BIT_6 0x3f

#define WIDTH_FA 2
#define HEIGHT_FA 2

#define WIDTH_BUF_FA WIDTH_FA
#define HEIGHT_BUF_FA HEIGHT_FA

#define SCREEN_START 0
#define SCREEN_TOP 50
#define SCREEN_W 240
#define SCREEN_H (240 - (2 * (SCREEN_TOP)))

#define WIDTH_BUF (LCD_WIDTH / WIDTH_BUF_FA)
#define HEIGHT_BUF (SCREEN_H / HEIGHT_BUF_FA)

// RGB 565

typedef struct layer_s
{
    uint8_t type;
    uint8_t start_x;
    uint8_t start_y;
    uint8_t alpha;
    union layer_data
    {
        struct layer_str
        {
            char *str;
            uint8_t font_size;
            color_t fc;
            int bc;
        } str_data;
        struct layer_pic
        {
            uint8_t *pic;
            uint8_t *mask;
            uint8_t real_width;
            uint8_t pic_start_x;
            uint8_t pic_start_y;
            uint8_t pic_width;
            uint8_t pic_height;
        } pic_data;
    } data;
} layer_t;

typedef struct palette_s
{
    color_t palette[256];
    uint16_t length;
} palette_t;

extern uint8_t bg[LCD_HEIGHT / HEIGHT_FA][LCD_WIDTH / WIDTH_FA];

extern color_t paint_buffer[HEIGHT_BUF][WIDTH_BUF];

extern float current_fps;
extern float speed;
extern u16 score;

extern u8 g_on_play;

void init_led(void);

void rand_init(void);

color_t alpha_mix(color_t color_a, color_t color_b, u8 alpha_b);

void set_fps(int val);

void c_lcd_refresher(int f);

#endif
