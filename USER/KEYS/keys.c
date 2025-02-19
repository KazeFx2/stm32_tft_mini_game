#include "keys.h"
#include "gpio.h"
#include "threads.h"
#include "stm32f10x_exti.h"
#include "music.h"

#define KEY0 PF(0)
#define KEY1 PF(1)
#define KEY2 PF(2)
#define KEY3 PF(3)
#define KEY4 PF(4)
#define KEY5 PF(5)
#define KEY6 PF(6)
#define KEY7 PF(7)
#define KEY8 PF(8)
#define KEY9 PF(9)

uint32_t pressed_keys = 0x0;

void init_keys(void)
{
    set_exti_line(KEY0, 2, 0);
    set_exti_line(KEY1, 2, 0);
    set_exti_line(KEY2, 2, 0);
    set_exti_line(KEY3, 2, 0);
    set_exti_line(KEY4, 2, 0);
    set_exti_line(KEY5, 2, 0);
    set_exti_line(KEY6, 2, 0);
    set_exti_line(KEY7, 2, 0);
    set_exti_line(KEY8, 2, 0);
    set_exti_line(KEY9, 2, 0);
}

// KEY 0
void EXTI0_IRQHandler(void)
{
		delay_ms(10);
    if (!read_bit(KEY0))
    {
        pressed_keys |= KEY_0;
        // ThreadHandlerType t = add_thread(play_sound_sample, 2, 1, 1);
        // start_thread(t, NO_JOIN);
    }
    else
    {
        pressed_keys &= ~(uint32_t)KEY_0;
    }
    EXTI_ClearITPendingBit(EXTI_Line0);
}

// KEY 1
void EXTI1_IRQHandler(void)
{
		delay_ms(10);
    if (!read_bit(KEY1))
    {
        pressed_keys |= KEY_1;
        // ThreadHandlerType t = add_thread(play_sound_sample, 2, 2, 1);
        // start_thread(t, NO_JOIN);
    }
    else
    {
        pressed_keys &= ~(uint32_t)KEY_1;
    }
    EXTI_ClearITPendingBit(EXTI_Line1);
}

// KEY 2
void EXTI2_IRQHandler(void)
{
		delay_ms(10);
    if (!read_bit(KEY2))
    {
        pressed_keys |= KEY_2;
        // ThreadHandlerType t = add_thread(play_sound_sample, 2, 3, 1);
        // start_thread(t, NO_JOIN);
    }
    else
    {
        pressed_keys &= ~(uint32_t)KEY_2;
    }
    EXTI_ClearITPendingBit(EXTI_Line2);
}

// KEY 3
void EXTI3_IRQHandler(void)
{
		delay_ms(10);
    if (!read_bit(KEY3))
    {
        pressed_keys |= KEY_3;
        // ThreadHandlerType t = add_thread(play_sound_sample, 2, 4, 1);
        // start_thread(t, NO_JOIN);
    }
    else
    {
        pressed_keys &= ~(uint32_t)KEY_3;
    }
    EXTI_ClearITPendingBit(EXTI_Line3);
}

// KEY 4
void EXTI4_IRQHandler(void)
{
		delay_ms(10);
    if (!read_bit(KEY4))
    {
        pressed_keys |= KEY_4;
        // ThreadHandlerType t = add_thread(play_sound_sample, 2, 5, 1);
        // start_thread(t, NO_JOIN);
    }
    else
    {
        pressed_keys &= ~(uint32_t)KEY_4;
    }
    EXTI_ClearITPendingBit(EXTI_Line4);
}

// KEY 5-9
void EXTI9_5_IRQHandler(void)
{
		delay_ms(10);
    if (EXTI_GetITStatus(EXTI_Line5) != RESET)
    {
        if (!read_bit(KEY5))
        {
            pressed_keys |= KEY_5;
            // ThreadHandlerType t = add_thread(play_sound_sample, 2, 11, 1);
            // start_thread(t, NO_JOIN);
        }
        else
        {
            pressed_keys &= ~(uint32_t)KEY_5;
        }
        EXTI_ClearITPendingBit(EXTI_Line5);
    }
    else if (EXTI_GetITStatus(EXTI_Line6) != RESET)
    {
        if (!read_bit(KEY6))
        {
            pressed_keys |= KEY_6;
            // ThreadHandlerType t = add_thread(play_sound_sample, 2, 22, 1);
            // start_thread(t, NO_JOIN);
        }
        else
        {
            pressed_keys &= ~(uint32_t)KEY_6;
        }
        EXTI_ClearITPendingBit(EXTI_Line6);
    }
    else if (EXTI_GetITStatus(EXTI_Line7) != RESET)
    {
        if (!read_bit(KEY7))
        {
            pressed_keys |= KEY_7;
            // ThreadHandlerType t = add_thread(play_sound_sample, 2, 33, 1);
            // start_thread(t, NO_JOIN);
        }
        else
        {
            pressed_keys &= ~(uint32_t)KEY_7;
        }
        EXTI_ClearITPendingBit(EXTI_Line7);
    }
    else if (EXTI_GetITStatus(EXTI_Line8) != RESET)
    {
        if (!read_bit(KEY8))
        {
            pressed_keys |= KEY_8;
            // ThreadHandlerType t = add_thread(play_sound_sample, 2, 44, 1);
            // start_thread(t, NO_JOIN);
        }
        else
        {
            pressed_keys &= ~(uint32_t)KEY_8;
        }
        EXTI_ClearITPendingBit(EXTI_Line8);
    }
    else if (EXTI_GetITStatus(EXTI_Line9) != RESET)
    {
        if (!read_bit(KEY9))
        {
            pressed_keys |= KEY_9;
            // ThreadHandlerType t = add_thread(play_sound_sample, 2, 55, 1);
            // start_thread(t, NO_JOIN);
        }
        else
        {
            pressed_keys &= ~(uint32_t)KEY_9;
        }
        EXTI_ClearITPendingBit(EXTI_Line9);
    }
}