#ifndef __MUSIC_H
#define __MUSIC_H

#include "gpio.h"

#define BEEP PB(8)

extern int sound_length_div, base;

#define PLAY_SOUND_F(f, t) play_sound(80, (f), t * 1000 / sound_length_div)

#define PLAY_SOUND(f) PLAY_SOUND_F(f, 1)

#define PLAY_L_1 PLAY_SOUND(262) // C
#define PLAY_L_1S PLAY_SOUND(277)
#define PLAY_L_2 PLAY_SOUND(294) // D
#define PLAY_L_2S PLAY_SOUND(311)
#define PLAY_L_3 PLAY_SOUND(330) // E
#define PLAY_L_4 PLAY_SOUND(349) // F
#define PLAY_L_4S PLAY_SOUND(370)
#define PLAY_L_5 PLAY_SOUND(392) // G
#define PLAY_L_5S PLAY_SOUND(415)
#define PLAY_L_6 PLAY_SOUND(440) // A
#define PLAY_L_6S PLAY_SOUND(466)
#define PLAY_L_7 PLAY_SOUND(494) // B

#define PLAY_M_1 PLAY_SOUND(523)
#define PLAY_M_1S PLAY_SOUND(554)
#define PLAY_M_2 PLAY_SOUND(587)
#define PLAY_M_2S PLAY_SOUND(622)
#define PLAY_M_3 PLAY_SOUND(659)
#define PLAY_M_4 PLAY_SOUND(698)
#define PLAY_M_4S PLAY_SOUND(740)
#define PLAY_M_5 PLAY_SOUND(784)
#define PLAY_M_5S PLAY_SOUND(831)
#define PLAY_M_6 PLAY_SOUND(880)
#define PLAY_M_6S PLAY_SOUND(932)
#define PLAY_M_7 PLAY_SOUND(988)

#define PLAY_H_1 PLAY_SOUND(1046)
#define PLAY_H_1S PLAY_SOUND(1109)
#define PLAY_H_2 PLAY_SOUND(1175)
#define PLAY_H_2S PLAY_SOUND(1245)
#define PLAY_H_3 PLAY_SOUND(1318)
#define PLAY_H_4 PLAY_SOUND(1397)
#define PLAY_H_4S PLAY_SOUND(1480)
#define PLAY_H_5 PLAY_SOUND(1568)
#define PLAY_H_5S PLAY_SOUND(1661)
#define PLAY_H_6 PLAY_SOUND(1760)
#define PLAY_H_6S PLAY_SOUND(1865)
#define PLAY_H_7 PLAY_SOUND(1976)

extern int frequencies[36];

extern int f_trans[8];

void dac1_init(void);

void pwm_init(void);

void adc_init(void);

u32 get_avg(u32 adc, u32 ch, int times);

int key_check(int *flag, uint64_t *time0, uint32_t key, u8 no_time_out);

void get_volt(void);

void key_loop(void);

void play_loop(void);

void set_base(char val);

void play_sound_sample(int val, int times);

void set_sound_length_div(int n);

void init_beep(void);

void play_sound(int duty, int frequency, int length_ms);

#endif
