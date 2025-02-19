#ifndef __USART_H
#define __USART_H

#include "stm32f10x.h"
#include "stdio.h"
#include "gpio.h"

void usart1_init(u32 bound);
void send_string(char *str);
void send_char(char ch);
char read_char(void);

#endif
