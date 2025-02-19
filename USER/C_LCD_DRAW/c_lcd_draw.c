#include "c_lcd_draw.h"
#include "threads.h"
#include "c_lcd.h"
#include "c_lcdfont.h"
#include "pic.h"
#include "pics.h"
#include "keys.h"
#include "gpio.h"
#include "stm32f10x_adc.h"
#include <stdlib.h>
#include <stdio.h>

uint8_t bg[LCD_HEIGHT / HEIGHT_FA][LCD_WIDTH / WIDTH_FA];

color_t paint_buffer[HEIGHT_BUF][WIDTH_BUF];

layer_t *layers[10];
uint8_t n_layers = 0;
u32 frames = 0;
u8 g_on_play;

#define MAX_LAYERS 15
layer_t stone_layers[MAX_LAYERS];
float shifts[MAX_LAYERS];
u8 stone_layers_start = 0;
u8 stone_layers_end = 0;

float safe_dist = 0;

uint8_t fps = 30;
float current_fps = 0;

u8 run_count = 0;
u8 bg_count = 0;

u16 score = 0;

float speed = 1, f_n_layers = 0, f_jump_count = 0, f_bg_count = 0;

#define MAX_SPEED 2.2
#define SPEED_STEP 0.1

#define JUMP_LEN 12
#define CH_START 20
#define CH_END 30
const int8_t jump_offsets[JUMP_LEN] = {-5, -10, -20, -10, -5, -1, 0, 0, 1, 6, 12, 32};

u8 jump_count = 0;
u8 do_jump = 0;

float gen_p = 0.1, max_gen_p = 0.65, gen_step = 0.12;

float h_p = 0.2, max_h_p = 0.6, h_step = 0.08;

#define LAYER_TYPE_STR 0
#define LAYER_TYPE_PIC 1

#define BIT_SWAP(x) ((x) >> 8 | (x) << 8)

u8 play_led = 0;

#define _LED PB(7)

void init_led(void)
{
    set_ppout(_LED);
    set_bit(_LED, 0);
}

void rand_init(void)
{
    ADC_InitTypeDef ADC_InitStructure;
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);
    ADC_DeInit(ADC1);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC1, DISABLE);
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_Init(ADC1, &ADC_InitStructure);

    // PF8 CH6
    ADC_Cmd(ADC1, ENABLE);

    ADC_TempSensorVrefintCmd(ENABLE);

    ADC_ResetCalibration(ADC1);
    while (ADC_GetResetCalibrationStatus(ADC1))
        ;
    ADC_StartCalibration(ADC1);
    while (ADC_GetCalibrationStatus(ADC1))
        ;

    ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 1, ADC_SampleTime_239Cycles5);
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC))
        ;
    srand(ADC_GetConversionValue(ADC1));
}

void fn_play_led(void)
{
    set_bit(_LED, 1);
    delay_ms(1000);
    set_bit(_LED, 0);
    play_led = 0;
}

color_t alpha_mix(color_t color_a, color_t color_b, u8 alpha_b)
{
    u16 r_a = (color_a >> 11) & BIT_5, g_a = (color_a >> 5) & BIT_6, b_a = (color_a)&BIT_5;
    u16 r_b = (color_b >> 11) & BIT_5, g_b = (color_b >> 5) & BIT_6, b_b = (color_b)&BIT_5;
    u16 r_r = r_a * (255 - alpha_b) / 255 + r_b * alpha_b / 255;
    u16 g_r = g_a * (255 - alpha_b) / 255 + g_b * alpha_b / 255;
    u16 b_r = b_a * (255 - alpha_b) / 255 + b_b * alpha_b / 255;
    r_r = r_r > BIT_5 ? BIT_5 : r_r;
    g_r = g_r > BIT_6 ? BIT_6 : g_r;
    b_r = b_r > BIT_5 ? BIT_5 : b_r;
    return r_r << 11 | g_r << 5 | b_r;
}

void set_fps(int val)
{
    fps = val;
}

void paint_bg_to_buffer(u8 scale_factor_x, u8 scale_factor_y, u8 target_x, u8 target_y, u8 width, u8 height)
{
    for (u8 i = 0; i < height; i++)
        for (u8 j = 0; j < width; j++)
        {
            u8 ori_x = (target_x + j) / scale_factor_x, ori_y = (target_y + i) / scale_factor_y;
            paint_buffer[i][j] = BIT_SWAP(bg_palette.palette[bg[ori_y][ori_x]]);
        }
}

void add_buf_color(u8 x, u8 y, color_t col, u8 alpha)
{
    paint_buffer[y][x] = BIT_SWAP(alpha_mix(BIT_SWAP(paint_buffer[y][x]), col, alpha));
}

// 12 16 24 32
void paint_char_to_buffer(u8 x, u8 y, char num, u8 sizey, int bc, color_t fc, u8 alpha, u8 target_x, u8 target_y, u8 width, u8 height)
{
    u8 temp, sizex, current_num, shift, left;
    u16 i, j, x_byte;
    u16 x0 = x;
    sizex = sizey / 2;
    u8 startx, endx, starty, endy;
    startx = x > target_x ? x : target_x;
    starty = y > target_y ? y : target_y;
    endx = x + sizex < target_x + width ? x + sizex : target_x + width;
    endy = y + sizey < target_y + height ? y + sizey : target_y + height;
    if (endx <= startx || endy <= starty)
        return;
    x_byte = sizex / 8 + ((sizex % 8) ? 1 : 0);
    num = num - ' '; // 得到偏移后的值
    for (i = starty; i < endy; i++)
    {
        current_num = ((startx - x) / 8) + (i - y) * x_byte;
        shift = (startx - x) % 8;
        left = 8 - shift;
        if (sizey == 12)
            temp = ascii_1206[num][current_num]; // 调用6x12字体
        else if (sizey == 16)
            temp = ascii_1608[num][current_num]; // 调用8x16字体
        else if (sizey == 24)
            temp = ascii_2412[num][current_num]; // 调用12x24字体
        else if (sizey == 32)
            temp = ascii_3216[num][current_num]; // 调用16x32字体
        else
            return;
        for (j = startx; j < endx; j++)
        {
            if (alpha != 255)
            {
                if (temp & (0x01 << shift))
                    add_buf_color(j - target_x, i - target_y, fc, alpha);
                else if (bc != -1)
                    add_buf_color(j - target_x, i - target_y, bc, alpha);
            }
            else
            {
                if (temp & (0x01 << shift))
                    paint_buffer[i - target_y][j - target_x] = BIT_SWAP(fc);
                else if (bc != -1)
                    paint_buffer[i - target_y][j - target_x] = BIT_SWAP(bc);
            }
            shift++;
            left--;
            if (left == 0)
            {
                if (sizey == 12)
                    temp = ascii_1206[num][++current_num]; // 调用6x12字体
                else if (sizey == 16)
                    temp = ascii_1608[num][++current_num]; // 调用8x16字体
                else if (sizey == 24)
                    temp = ascii_2412[num][++current_num]; // 调用12x24字体
                else if (sizey == 32)
                    temp = ascii_3216[num][++current_num]; // 调用16x32字体
                else
                    return;
                shift = 0;
                left = 8;
            }
        }
    }
}

void paint_str_to_buffer(u8 x, u8 y, const char *str, u8 sizey, int bc, color_t fc, u8 alpha, u8 target_x, u8 target_y, u8 width, u8 height)
{
    u8 sizex = sizey / 2;
    for (; *str != 0; str++, x += sizex)
    {
        paint_char_to_buffer(x, y, *str, sizey, bc, fc, alpha, target_x, target_y, width, height);
    }
}

void paint_pic_to_buffer(u8 x, u8 y, const u8 *pic, const u8 *mask, u8 rw, u8 p_x, u8 p_y, u8 w, u8 h, u8 alpha, u8 target_x, u8 target_y, u8 width, u8 height)
{
    u8 startx = x > target_x ? x : target_x;
    u8 starty = y > target_y ? y : target_y;
    u8 endx = x + w < target_x + width ? x + w : target_x + width;
    u8 endy = y + h < target_y + height ? y + h : target_y + height;
    if (endx <= startx || endy <= starty)
        return;
    for (u8 i = starty; i < endy; i++)
        for (u8 j = startx; j < endx; j++)
        {
            u32 low = (i - y + p_y) * rw * 2 + (j - x + p_x) * 2;
            if (mask && !mask[low / 2])
                continue;
            color_t val = (color_t)pic[low] << 8 | pic[low + 1];
            if (alpha != 255)
                add_buf_color(j - target_x, i - target_y, val, alpha);
            else
                paint_buffer[i - target_y][j - target_x] = BIT_SWAP(val);
        }
}

void set_bk_(u8 shift)
{
    for (u8 i = 0; i < 120; i++)
        for (u8 j = 0; j < 120; j++)
            bg[i][(j + 120 - shift) % 120] = bg_pic[i][j];
}

void paint_bg_to_buffer_(u8 scale_factor_x, u8 scale_factor_y, u8 target_x, u8 target_y, u8 width, u8 height, u8 shift)
{
    for (u8 i = 0; i < height; i++)
        for (u8 j = 0; j < width; j++)
        {
            u8 ori_x = (target_x + j) / scale_factor_x, ori_y = (target_y + i) / scale_factor_y;
            paint_buffer[i][j] = BIT_SWAP(bg_palette.palette[bg_pic[ori_y][(ori_x + shift) % 120]]);
        }
}

void add_stone(u8 size)
{
    u8 inx = stone_layers_end++;
    stone_layers_end %= MAX_LAYERS;
    layer_t *tar = &stone_layers[inx];
    tar->start_x = SCREEN_START + SCREEN_W;
    tar->alpha = 255;
    tar->type = LAYER_TYPE_PIC;
    if (size == 0)
        STONE_L_DATA(tar->data.pic_data), safe_dist = 12;
    else
        STONE_H_DATA(tar->data.pic_data), safe_dist = 15;
    tar->start_y = 184 - tar->data.pic_data.pic_height;
    layers[n_layers++] = tar;
    shifts[inx] = 0;
}

void stone_move()
{
    u8 i = n_layers - 1;
    for (; i > 0; i--)
    {
        u8 start = (u8)shifts[i];
        shifts[i] += speed * 8;
        u8 end = (u8)shifts[i];
        u8 move = end - start;
        if (move >= layers[i]->start_x + layers[i]->data.pic_data.real_width - layers[i]->data.pic_data.pic_start_x)
            break;
        if (layers[i]->start_x >= move)
            layers[i]->start_x -= move;
        else
            layers[i]->data.pic_data.pic_start_x += (move - layers[i]->start_x), layers[i]->data.pic_data.pic_width -= (move - layers[i]->start_x), layers[i]->start_x = 0;
    }
    if (i == 0)
        return;
    u8 left = n_layers - i;
    for (u8 j = 1; j < left; j++)
    {
        layers[j] = layers[i + j];
    }
    n_layers -= i;
    stone_layers_start += i;
    stone_layers_start %= MAX_LAYERS;
}

u8 stone_check()
{
    u8 i = n_layers - 1;
    for (; i > 0; i--)
    {
        u8 s_start = layers[i]->start_x, s_end = layers[i]->start_x + layers[i]->data.pic_data.pic_width;
        if ((CH_START < s_start && CH_END > s_start) || (CH_START < s_end && CH_END > s_end) || (s_start < CH_START && s_end > CH_START) || (s_start < CH_END && s_end > CH_END))
        {
            if (layers[0]->start_y + layers[0]->data.pic_data.pic_height > layers[i]->start_y)
            {
                // HIT
                // if (!play_led)
                // {
                //     play_led = 1;
                //     start_thread(add_thread(fn_play_led, 0), NO_JOIN);
                // }
                set_bit(_LED, 1);
                return 1;
            }
        }
    }
    if (i == 0)
        set_bit(_LED, 0);
    return 0;
}

void init_play(void)
{

    n_layers = 1;
    frames = 0;

    stone_layers_start = 0;
    stone_layers_end = 0;

    safe_dist = 0;

    run_count = 0;
    bg_count = 0;

    score = 0;

    speed = 1, f_n_layers = 0, f_jump_count = 0, f_bg_count = 0;

    jump_count = 0;
    do_jump = 0;

    play_led = 0;
}

void c_lcd_refresher(int f)
{
    g_on_play = 1;
    LCD_Fill(0, 0, LCD_WIDTH, LCD_HEIGHT, BLACK);
    layer_t test_layer_pic;
    test_layer_pic.type = LAYER_TYPE_PIC;
    test_layer_pic.alpha = 255;
    test_layer_pic.start_x = 0;
    test_layer_pic.start_y = 120;
    test_layer_pic.data.pic_data.mask = &run_pic_masks[run_count];
    test_layer_pic.data.pic_data.pic = &run_pics[run_count];
    test_layer_pic.data.pic_data.real_width = 60;
    test_layer_pic.data.pic_data.pic_start_x = 0;
    test_layer_pic.data.pic_data.pic_start_y = 0;
    test_layer_pic.data.pic_data.pic_width = 60;
    test_layer_pic.data.pic_data.pic_height = 60;

    layers[n_layers++] = &test_layer_pic;

    set_fps(f);
    uint64_t time0 = time(), dur = 0, time_0, time_1;
    u16 c_x = 0, c_y = 0, i;
    while (1)
    {
        time_0 = time();
        // PAINT START
        for (c_y = SCREEN_TOP; c_y < SCREEN_TOP + SCREEN_H;)
        {
            for (c_x = SCREEN_START; c_x < SCREEN_START + SCREEN_W;)
            {
                u8 w = (SCREEN_START + SCREEN_W) - c_x >= WIDTH_BUF ? WIDTH_BUF : (SCREEN_START + SCREEN_W) - c_x;
                u8 h = (SCREEN_TOP + SCREEN_H) - c_y >= HEIGHT_BUF ? HEIGHT_BUF : (SCREEN_TOP + SCREEN_H) - c_y;
                paint_bg_to_buffer_(WIDTH_FA, HEIGHT_FA, c_x, c_y, w, h, bg_count);
                // ...
                for (i = 0; i < n_layers; i++)
                {
                    // break;
                    switch (layers[i]->type)
                    {
                    case LAYER_TYPE_PIC:
                        paint_pic_to_buffer(layers[i]->start_x, layers[i]->start_y, layers[i]->data.pic_data.pic, layers[i]->data.pic_data.mask, layers[i]->data.pic_data.real_width, layers[i]->data.pic_data.pic_start_x, layers[i]->data.pic_data.pic_start_y, layers[i]->data.pic_data.pic_width, layers[i]->data.pic_data.pic_height, layers[i]->alpha, c_x, c_y, w, h);
                        break;
                    case LAYER_TYPE_STR:
                        paint_str_to_buffer(layers[i]->start_x, layers[i]->start_y, layers[i]->data.str_data.str, layers[i]->data.str_data.font_size,
                                            layers[i]->data.str_data.bc, layers[i]->data.str_data.fc, layers[i]->alpha, c_x, c_y, w, h);
                        break;
                    default:
                        break;
                    }
                }
                // ...
                if (c_x != SCREEN_START || c_y != SCREEN_TOP)
                    wait_dma();
                LCD_ShowPicture(c_x, c_y, w, h, (u8 *)paint_buffer, 1);
                c_x += WIDTH_BUF;
            }
            c_y += HEIGHT_BUF;
        }
        // PAINT END
        // AFTER PAINT

        if (stone_check())
        {
            char tmp_str[20];
            wait_dma();
            sprintf(tmp_str, "Your Score: %05d", score);
            LCD_ShowString(5, 5, "Game Over", RED, WHITE, 24, 1);
            LCD_ShowString(5, 32, tmp_str, WHITE, WHITE, 16, 1);
            LCD_ShowString(5, 240 - 5 - 34, "Press KEY0 to retry, KEY1 to ", WHITE, WHITE, 16, 1);
            LCD_ShowString(5, 240 - 5 - 16, "quit.", WHITE, WHITE, 16, 1);
            delay_ms(1000);
            while (1)
            {
                if (pressed_keys & KEY_0)
                {
                    LCD_Fill(0, 0, LCD_WIDTH, LCD_HEIGHT, BLACK);
                    init_play();
                    g_on_play = 1;
                    break;
                }
                else if (pressed_keys & KEY_1)
                {
                    LCD_Fill(0, 0, LCD_WIDTH, LCD_HEIGHT, GRAY);
                    init_play();
                    n_layers = 0;
                    g_on_play = 0;
                    return;
                }
                delay_ms(100);
            }
        }

        f_bg_count += 4 * speed;
        while (f_bg_count >= 120)
            f_bg_count -= 120;
        bg_count = (int)f_bg_count;
        run_count = (run_count + 1) % RUN_LEN;
        test_layer_pic.data.pic_data.mask = &run_pic_masks[run_count];
        test_layer_pic.data.pic_data.pic = &run_pics[run_count];
        if (do_jump)
        {
            f_jump_count += speed;
            while (f_jump_count-- > 0 && jump_count < JUMP_LEN)
                test_layer_pic.start_y += jump_offsets[jump_count++];
            if (f_jump_count < 0)
                f_jump_count++;
            if (jump_count >= JUMP_LEN)
                do_jump = 0;
        }
        if (pressed_keys & KEY_0)
        {
            if (!do_jump)
            {
                do_jump = 1;
                jump_count = 0;
                f_jump_count = 0;
            }
        }
        stone_move();
        if (safe_dist < 0)
        {
            u8 v1 = rand() % 100;
            if (v1 < gen_p * 100)
            {
                v1 = rand() % 100;
                if (v1 < h_p * 100)
                    add_stone(1);
                else
                    add_stone(0);
            }
        }
        else
            safe_dist -= speed;
        // AFTER PAINT END
        frames++;
        score++;
        if (!(score % 120) && speed < MAX_SPEED)
        {
            speed += SPEED_STEP;
            gen_p += gen_step;
            if (gen_p > max_gen_p)
                gen_p = max_gen_p;
            h_p += h_step;
            if (h_p > max_h_p)
                h_p = max_h_p;
            if (speed > MAX_SPEED)
                speed = MAX_SPEED;
        }
        time_1 = time();
        // FPS Control
        if (fps)
            delay_us(1000000 / fps > (time_1 - time_0) ? 1000000 / fps - (time_1 - time_0) : 0);
        // FPS Calc
        if (time() - time0 > 1000000)
        {
            dur = time() - time0;
            time0 = time();
            current_fps = (float)frames * 1000000 / dur;
            frames = 0;
        }
    }
}
