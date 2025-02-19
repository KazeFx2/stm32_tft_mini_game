/**
 ******************************************************************************
 * @file    USART/Printf/main.c
 * @author  MCD Application Team
 * @version V3.5.0
 * @date    08-April-2011
 * @brief   Main program body
 ******************************************************************************
 * @attention
 *
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
 * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
 * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
 * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
 * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include <stdio.h>

#include "stm32f10x.h"
#include "def.h"
#include "threads.h"
#include "gpio.h"
#include "music.h"
#include "oled.h"
#include "lcd_def.h"
#include "c_lcd.h"
#include "keys.h"
#include "misc.h"
#include "c_lcd_draw.h"
#include "usart.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_rcc.h"

#define LED0 PB(5)
#define LED1 PE(5)

int32_t led(int a)
{
	int ret = a;
	while (a--)
	{
		set_bit(LED1, 0);
		delay_ms(500);
		set_bit(LED1, 1);
		delay_ms(500);
	}
	return ret;
}

ThreadHandlerType lcd;

void oled_show(void)
{
	int a = 0;
	char tmp[20];
	uint64_t time0 = time(), dur = 0, time_0, time_1;
	int a0 = a;
	int frames = 0;
	float FPS = 0.0;
	int tar_fps = 10;
	while (1)
	{
		time_0 = time();
		if (time_0 - time0 > 1000000)
		{
			dur = time_0 - time0;
			time0 = time_0;
			frames = a - a0;
			a0 = a;
			FPS = (float)frames * 1000000 / dur;
			sprintf(tmp, "FPS: %.2f", FPS);
			OLED_ShowString(0, 20, tmp, 16);
		}
		sprintf(tmp, "Count: %07d", a);
		OLED_ShowString(0, 0, tmp, 16);
		sprintf(tmp, "KEYS: %03X", pressed_keys);
		OLED_ShowString(0, 40, tmp, 16);
		OLED_Refresh();
		a++;
		time_1 = time();
		if (tar_fps)
			delay_us(1000000 / tar_fps > (time_1 - time_0) ? 1000000 / tar_fps - (time_1 - time_0) : 0);
	}
}

void lcd_show(void)
{
	char tmp[20];
	uint64_t time_0, time_1;
	int tar_fps = 1;
	while (1)
	{
		time_0 = time();
		sprintf(tmp, "LCD_FPS: %.2f", current_fps);
		display_list_char(0, 0, tmp);
		sprintf(tmp, "V: %.1f", speed);
		display_list_char(0, 1, tmp);
		sprintf(tmp, "S: %05d", score);
		display_list_char(7, 1, tmp);
		time_1 = time();
		if (tar_fps)
			delay_ms(1000 / tar_fps > (time_1 - time_0) / 1000 ? 1000 / tar_fps - (time_1 - time_0) / 1000 : 0);
	}
}

void usart_msg(void)
{
	char ch, tmp[20], first_send = 0;
	while (1)
	{
		ch = read_char();
		if (first_send == 0)
		{
			send_string("REPLY: ");
			first_send = 1;
		}
		sprintf(tmp, "Input: %02x", ch);
		display_list_char(0, 1, tmp);
		if (ch == 0x0d)
		{
			send_char('\r');
			send_char('\n');
			first_send = 0;
		}
		else
			send_char(ch);
	}
}

void usart_send_msg(void)
{
	int pressed = 0;
	while (1)
	{
		if (!pressed && (pressed_keys & KEY_4))
		{
			send_string("YOU PRESSED KEY 4\r\n");
			pressed = 1;
		}
		else if (pressed && !(pressed_keys & KEY_4))
			pressed = 0;
	}
}

// exp
void init_adc()
{
	ADC_InitTypeDef ADC_InitStructure;
	// 光敏 PF8
	set_ain(PF(8));
	set_ain(PA(1));
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);
	ADC_DeInit(ADC1);
	ADC_DeInit(ADC3);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3 | RCC_APB2Periph_ADC1, ENABLE);
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC3 | RCC_APB2Periph_ADC1, ENABLE);
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC3 | RCC_APB2Periph_ADC1, DISABLE);
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_NbrOfChannel = 1;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_Init(ADC3, &ADC_InitStructure);
	ADC_Init(ADC1, &ADC_InitStructure);

	// PF8 CH6
	ADC_Cmd(ADC1, ENABLE);
	ADC_Cmd(ADC3, ENABLE);

	ADC_TempSensorVrefintCmd(ENABLE);

	ADC_ResetCalibration(ADC1);
	while (ADC_GetResetCalibrationStatus(ADC1))
		;
	ADC_StartCalibration(ADC1);
	while (ADC_GetCalibrationStatus(ADC1))
		;
	ADC_ResetCalibration(ADC3);
	while (ADC_GetResetCalibrationStatus(ADC3))
		;
	ADC_StartCalibration(ADC3);
	while (ADC_GetCalibrationStatus(ADC3))
		;
}

void get_adc_info()
{
	char str[20];
	while (1)
	{
		u16 temp = get_avg(ADC1, ADC_Channel_16, 20);
		u16 lsens = get_avg(ADC3, ADC_Channel_6, 1);
		u16 volt = get_avg(ADC1, ADC_Channel_1, 1);

		// temp
		double temp_f = (float)temp * (3.3 / 4096);
		// conv
		temp_f = (1.43 - temp_f) / 0.0043 + 25;
		temp_f *= 10;
		sprintf(str, "Temp: %.5lf, v: %d\r\n", temp_f, volt);
		send_string(str);

		// lsens
		if (lsens > 4000)
			lsens = 4000;
		lsens = (100 - (lsens / 40));
		sprintf(str, "Lsens: %d\r\n", lsens);
		send_string(str);
		delay_ms(500);
	}
}

// end exp
int main(void)
{
	// set_up
	init_threads_manager();
	// init_beep();
	dac1_init();
	// pwm_init();
	lcd_init(1, 0);
	oled_init(2);
	c_lcd_init(1);
	init_keys();
	init_led();
	rand_init();
	usart1_init(115200);
	// init_adc();

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

	set_ppout(LED0);
	set_ppout(LED1);
	set_bit(LED0, 1);
	set_bit(LED1, 1);

	// get_adc_info();
	// ThreadHandlerType t1 = add_thread(get_adc_info, 0);
	// start_thread(t1, NO_JOIN);

	LCD_Fill(0, 0, LCD_W, LCD_H, WHITE);
	LCD_Fill(0, 0, LCD_W, LCD_H, GRAY);

	ThreadHandlerType t1 = add_thread(led, 1, 3);
	start_thread(t1, NO_JOIN);

	ThreadHandlerType oled = add_thread(oled_show, 0);
	start_thread(oled, NO_JOIN);

	lcd = add_thread(lcd_show, 0);
	start_thread(lcd, NO_JOIN);

	// ThreadHandlerType c_lcd = add_thread(c_lcd_refresher, 1, 0);
	// start_thread(c_lcd, NO_JOIN);
	ThreadHandlerType c_lcd;

	ThreadHandlerType dac = add_thread(key_loop, 0);
	start_thread(dac, NO_JOIN);

	ThreadHandlerType play = add_thread(play_loop, 0);
	start_thread(play, NO_JOIN);

	ThreadHandlerType volt = add_thread(get_volt, 0);
	start_thread(volt, NO_JOIN);

	u8 func_b = 0;

	frozen_thread(volt);
	frozen_thread(play);

	// ThreadHandlerType usart = add_thread(usart_msg, 0);
	// start_thread(usart, NO_JOIN);

	// ThreadHandlerType usart_send = add_thread(usart_send_msg, 0);
	// start_thread(usart_send, NO_JOIN);

	int i = 0;

	u8 key_1_flag;
	uint64_t key_1_time;

	while (1)
	{
		i++;
		if (!(i % 100))
		{
			t1 = add_thread(led, 1, 3);
			start_thread(t1, NO_JOIN);
		}
		set_anti_bit(LED0);
		if (key_check(&key_1_flag, &key_1_time, KEY_1, 1))
		{
			if (!g_on_play && !func_b)
			{
				frozen_thread(oled);
				OLED_Clear();
				resume_thread(volt);
				resume_thread(play);
				func_b = 1;
			}
			else if (func_b)
			{
				frozen_thread(volt);
				frozen_thread(play);
				OLED_Clear();
				resume_thread(oled);
				func_b = 0;
				c_lcd = add_thread(c_lcd_refresher, 1, 0);
				start_thread(c_lcd, NO_JOIN);
			}
		}
		else
		{
			if (!g_on_play && !func_b)
			{
				frozen_thread(oled);
				OLED_Clear();
				resume_thread(volt);
				resume_thread(play);
				func_b = 1;
			}
		}
		delay_ms(50);
	}
}

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
