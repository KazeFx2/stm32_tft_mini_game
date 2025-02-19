#include "music.h"
#include "threads.h"
#include "keys.h"
#include "stm32f10x_dac.h"
#include "stm32f10x_adc.h"
#include "oled.h"
#include <stm32f10x_gpio.h>

int sound_length_div = 5;
int base = 5;

u16 out_val = 0, duty = 0, frequency = 1;

u8 type = 0;

int frequencies[] = {
	262, // 0 -> C
	277,
	294, // 2 -> D
	311,
	330, // 4 -> E
	349,
	370, // 6 -> F
	392, // 7 -> G
	415,
	440, // 9 -> A
	466,
	494, // 11 -> B

	523,
	554,
	587,
	622,
	659,
	698,
	740,
	784,
	831,
	880,
	932,
	988,

	1046,
	1109,
	1175,
	1245,
	1318,
	1397,
	1480,
	1568,
	1661,
	1760,
	1865,
	1976};

int f_trans[] = {
	0, 0, 2, 4, 6, 7, 9, 11};

void set_base(char val)
{
	switch (val)
	{
	case 'C':
		base = 0;
		break;
	case 'D':
		base = 2;
		break;
	case 'E':
		base = 4;
		break;
	case 'F':
		base = 6;
		break;
	case 'G':
		base = 7;
		break;
	case 'A':
		base = 9;
		break;
	case 'B':
		base = 11;
		break;
	default:
		base = 0;
		break;
	}
}

void dac1_init(void)
{
	DAC_InitTypeDef DAC_InitType;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);
	set_ain(PA(4));
	set_bit(PA(4), 0);

	DAC_InitType.DAC_Trigger = DAC_Trigger_None;
	DAC_InitType.DAC_WaveGeneration = DAC_WaveGeneration_None;
	DAC_InitType.DAC_LFSRUnmask_TriangleAmplitude = DAC_LFSRUnmask_Bit0;
	DAC_InitType.DAC_OutputBuffer = DAC_OutputBuffer_Disable;
	DAC_Init(DAC_Channel_1, &DAC_InitType);
	DAC_Cmd(DAC_Channel_1, ENABLE);
	out_val = 0;
	// while (out_val != 4096)
	// 	DAC_SetChannel1Data(DAC_Align_12b_R, out_val++), delay_us(100);
	// while (1)
	// {
	// 	DAC_SetChannel1Data(DAC_Align_12b_R, --out_val), delay_us(100);
	// 	if (!out_val)
	// 		break;
	// }
	type = 0;
}

void pwm_init(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_OCInitTypeDef TIM_OCInitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_TIM4, ENABLE);

	set_afpp(PD(12));
	set_bit(PD(12), 0);

	// 10kHz
	TIM_TimeBaseStructure.TIM_Period = 600 - 1;
	TIM_TimeBaseStructure.TIM_Prescaler = 12 - 1;
	TIM_TimeBaseStructure.TIM_ClockDivision = 1 - 1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

	// TIM4 Channelx PWM
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

	TIM_OC1Init(TIM4, &TIM_OCInitStructure);
	TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);

	TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	TIM_Cmd(TIM4, ENABLE);
	TIM_SelectOCxM(TIM4, TIM_Channel_1, TIM_ForcedAction_InActive);
	TIM_CCxCmd(TIM4, TIM_Channel_1, TIM_CCx_Enable);
	type = 1;
}

void adc_init(void)
{
	ADC_InitTypeDef ADC_InitStructure;
	// 光敏 PF8
	set_ain(PA(1));
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
}

u32 get_avg(u32 adc, u32 ch, int times)
{
	u32 val = 0;
	int bk = times;
	while (times--)
	{
		ADC_RegularChannelConfig(adc, ch, 1, ADC_SampleTime_239Cycles5);
		ADC_SoftwareStartConvCmd(adc, ENABLE);
		while (!ADC_GetFlagStatus(adc, ADC_FLAG_EOC))
			;
		val += ADC_GetConversionValue(adc);
		delay_ms(5);
	}
	return val / bk;
}

// 0~127
// 0~63
u8 x = 0;
void get_volt(void)
{
	while (1)
	{
		u32 volt = get_avg(ADC1, ADC_Channel_1, 1);
		for (u8 i = 0; i < 64; i++)
			OLED_ClearPoint(x, i);
		// printf("%d\r\n", volt);
		OLED_DrawPoint(x++, volt * 63 / 4095);
		OLED_Refresh();
		delay_us(10);
	}
}

void TIM4_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
	}
}

#define DUR_DELAY 1000 * 1000

int key_check(int *flag, uint64_t *time0, uint32_t key, u8 no_time_out)
{
	if (!*flag && (pressed_keys & key))
	{
		*flag = 1;
		*time0 = time();
		return 1;
	}
	else if (*flag && !(pressed_keys & key))
	{
		*flag = 0;
		return 0;
	}
	else if (!no_time_out && (pressed_keys & key) && (time() - *time0) > DUR_DELAY)
	{
		return 1;
	}
	return 0;
}

void key_loop(void)
{
	int pressed_up = 0, pressed_down = 0, duty_up = 0, duty_down = 0, fre_up = 0, fre_down = 0;
	uint64_t time0_1, time0_2, time0_3, time0_4, time0_5, time0_6;
	uint64_t time_0, time_1;
	int tar_fps = 10;
	while (1)
	{
		time_0 = time();
		if (key_check(&pressed_up, &time0_1, KEY_3, 0))
		{
			out_val += 10;
			if (out_val > 4095)
				out_val = 0;
			// DAC_SetChannel1Data(DAC_Align_12b_R, out_val);
			printf("[DAC]%05d, [DUTY]%03d, [FREQENCY]%05d\r\n", out_val, duty, frequency, 0);
		}
		if (key_check(&pressed_down, &time0_2, KEY_4, 0))
		{
			out_val -= 10;
			if (out_val > 4095)
				out_val = 4095;
			// DAC_SetChannel1Data(DAC_Align_12b_R, out_val);
			printf("[DAC]%05d, [DUTY]%03d, [FREQENCY]%05d\r\n", out_val, duty, frequency, 0);
		}
		if (key_check(&duty_up, &time0_3, KEY_5, 0))
		{
			++duty;
			if (duty > 100)
				duty = 0;
			printf("[DAC]%05d, [DUTY]%03d, [FREQENCY]%05d\r\n", out_val, duty, frequency, 0);
		}
		if (key_check(&duty_down, &time0_4, KEY_6, 0))
		{
			--duty;
			if (duty == 65535)
				duty = 100;
			printf("[DAC]%05d, [DUTY]%03d, [FREQENCY]%05d\r\n", out_val, duty, frequency, 0);
		}
		if (key_check(&fre_up, &time0_5, KEY_7, 0))
		{
			frequency += 1;
			if (frequency > 10)
				frequency = 1;
			printf("[DAC]%05d, [DUTY]%03d, [FREQENCY]%05d\r\n", out_val, duty, frequency, 0);
		}
		if (key_check(&fre_down, &time0_6, KEY_8, 0))
		{
			frequency -= 1;
			if (frequency > 10 || frequency < 1)
				frequency = 10;
			printf("[DAC]%05d, [DUTY]%03d, [FREQENCY]%05d\r\n", out_val, duty, frequency, 0);
		}
		time_1 = time();
		if (tar_fps)
			delay_ms(1000 / tar_fps > (time_1 - time_0) / 1000 ? 1000 / tar_fps - (time_1 - time_0) / 1000 : 0);
	}
}

void play_loop(void)
{
	while (1)
	{
		// play_sound(duty, frequency, 100);
		// delay_ms(1000);
		uint64_t dur = 1000 * 1000 / frequency / 4096;
		u16 val = 0;
		while (val < 4096)
			DAC_SetChannel1Data(DAC_Align_12b_R, val++),
				delay_us(dur);
	}
}

void play_sound_sample(int val, int times)
{
	if (val == 0)
	{
		delay_ms(1000 / sound_length_div);
	}
	else if (val < 10)
	{
		PLAY_SOUND_F(frequencies[base + f_trans[val]], times);
	}
	else
	{
		PLAY_SOUND_F(frequencies[base + f_trans[val % 10] + 12], times);
	}
	delay_ms(10);
}

void set_sound_length_div(int n)
{
	sound_length_div = n;
}

void init_beep(void)
{
	set_ppout(BEEP);
	set_bit(BEEP, 0);
}

void play_sound(int duty, int frequency, int length_ms)
{
	if (!type)
	{
		frequency = frequency * length_ms / 1000;
		uint64_t dur = (uint64_t)length_ms * 1000 / frequency;
		uint64_t high = dur * duty / 100;
		uint64_t low = dur - high;
		while (frequency--)
		{
			DAC_SetChannel1Data(DAC_Align_12b_R, out_val);
			delay_us(high);
			DAC_SetChannel1Data(DAC_Align_12b_R, 0);
			delay_us(low);
		}
	}
	else
	{
		u16 arr = (u32)60000 * 100 / frequency;
		u16 cmp = (u32)arr * duty / 100;
		// printf("[ARR]%05d, [CMP]%05d, [LEN]%05d\r\n", arr, cmp, length_ms);
		TIM_SetCounter(TIM4, 0);
		TIM_SetAutoreload(TIM4, arr - 1);
		TIM_SetCompare1(TIM4, cmp - 1);
		if (cmp != 0)
		{
			TIM_SelectOCxM(TIM4, TIM_Channel_1, TIM_OCMode_PWM1);
			TIM_CCxCmd(TIM4, TIM_Channel_1, TIM_CCx_Enable);
		}
		// TIM_Cmd(TIM4, ENABLE);
		delay_ms(length_ms);
		// TIM_Cmd(TIM4, DISABLE);
		uint64_t us = 500 * 1000, min = 10, n = 0;
		uint64_t dur = us / (cmp ? cmp : 1), t0, t1;
		min = dur > min ? min : 0;
		float k = (dur - min) * 2.0 / (cmp ? cmp : 1);
		while (cmp)
		{
			t0 = time();
			TIM_SetCompare1(TIM4, cmp--);
			dur = min + k * n;
			dur > 500 ? dur - 500 : dur;
			n++;
			t1 = time() - t0;
			delay_us(dur > t1 ? dur - t1 : 0);
		}
		TIM_SelectOCxM(TIM4, TIM_Channel_1, TIM_ForcedAction_InActive);
		TIM_CCxCmd(TIM4, TIM_Channel_1, TIM_CCx_Enable);
	}
}
