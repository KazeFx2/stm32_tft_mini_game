#ifndef __KEYS_H
#define __KEYS_H

#include "stm32f10x.h"

#define KEY_0 0x1
#define KEY_1 0x2
#define KEY_2 0x4
#define KEY_3 0x8
#define KEY_4 0x10
#define KEY_5 0x20
#define KEY_6 0x40
#define KEY_7 0x80
#define KEY_8 0x100
#define KEY_9 0x200

extern uint32_t pressed_keys;

void init_keys(void);

#endif
