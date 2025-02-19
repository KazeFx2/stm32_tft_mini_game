#ifndef __PICS_H_
#define __PICS_H_
#endif

#include "c_lcd_draw.h"

extern const u8 bg_pic[120][120];

extern const palette_t bg_palette;

#define RUN_LEN 4

extern const u8 run_pics[4][60][60][2];

extern const u8 run_pic_masks[4][60][60];

#define STONE_L_DATA(x)      \
    (x).pic = stone_l_pic,   \
    (x).mask = stone_l_mask, \
    (x).real_width = 19,     \
    (x).pic_start_x = 0,     \
    (x).pic_start_y = 0,     \
    (x).pic_width = 19,      \
    (x).pic_height = 25

extern const u8 stone_l_pic[25][19][2];

extern const u8 stone_l_mask[25][19];

#define STONE_H_DATA(x)      \
    (x).pic = stone_h_pic,   \
    (x).mask = stone_h_mask, \
    (x).real_width = 28,     \
    (x).pic_start_x = 0,     \
    (x).pic_start_y = 0,     \
    (x).pic_width = 28,      \
    (x).pic_height = 36

extern const u8 stone_h_pic[36][28][2];

extern const u8 stone_h_mask[36][28];
