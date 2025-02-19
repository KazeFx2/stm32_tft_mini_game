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
#include "def.h"

#include "stm32f10x.h"
#include "usart.h"
#include "lcd_def.h"
#include "delay.h"
#include "spi.h"
#include "oled.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_rcc.h"
#include "core_cm3.h"
#include "gpio.h"
#include <string.h>

#include <stdarg.h>


#define Stack_Size 1024 * 4
#define Drawback 40

#define LED PE(5)
#define LED_2 PB(5)

char tmp[32];
int inx = 0;

typedef struct PCB_s
{
  uint32_t *sp_ptr;
  u8 created;
  void (*func)(void);
	uint32_t registers[8];
} PCB_S;

__asm void _MSR_MSP(u32 addr)
{
  MSR MSP, r0 // set Main Stack value
  BX r14
}
__asm void MSR_PSR(u32 addr)
{
  MSR PSR, r0 // set Main Stack value
  BX r14
}

__asm uint32_t get_MSP(void)
{
  MRS r0, MSP
  BX LR
}

__asm uint32_t get_LR(void)
{
  MOV r0, LR
  BX LR
}

__asm uint32_t get_PSR(void)
{
  MRS r0, PSR
  BX LR
}

__asm void set_PC(void (*func)(void), uint32_t addr)
{
  MRS r2, MSP
  STR r0, [r2, #32]
	MOV r0, #0x01000000
  STR r0, [r2, #36]
	MOV r0, #0xfffffff9
  STR r0, [r2, #4]
	MOV r0, r1
  STR r0, [r2, #8]
	MOV r0, #0x1
	STR r0, [r2, #12]
	MOV r0, #0x2
	STR r0, [r2, #16]
	MOV r0, #0x3
	STR r0, [r2, #20]
  MOV r0, r2
	BX r14
}

__asm void save_Context_r4_2_r11(uint32_t addr)
{
	STR r4, [r0, #0]
	STR r5, [r0, #4]
	STR r6, [r0, #8]
	STR r7, [r0, #12]
	STR r8, [r0, #16]
	STR r9, [r0, #20]
	STR r10, [r0, #24]
	STR r11, [r0, #28]
	BX r14
}

__asm void rec_Context_r4_2_r11(uint32_t addr)
{
	LDR r4, [r0, #0]
	LDR r5, [r0, #4]
	LDR r6, [r0, #8]
	LDR r7, [r0, #12]
	LDR r8, [r0, #16]
	LDR r9, [r0, #20]
	LDR r10, [r0, #24]
	LDR r11, [r0, #28]
	BX r14
}

void quit_Handler(void)
{
  uint32_t tmp = get_PSR() & 0xffffff00;
	MSR_PSR(tmp);
}

double var_func(int num, ...){
		uint32_t sp = get_MSP();
    va_list valist;
    double sum = 0.0;
    int i;
    va_start(valist, num);
 
    for (i = 0; i < num; i++)
    {
       sum += va_arg(valist, int);
    }
    va_end(valist);
		return sum;
}

void caller(void (*func)(void))
{
	func();
}

void task1(void)
{
  while (1)
  {
		inx++;
    // TIM_Cmd(TIM3, DISABLE);
		sprintf(tmp, "T1: %05d\r\n", inx);
		// sprintf(tmp);
    // sprintf(tmp, "Now Count: %03d", inx);
    display_list_char(0, 1, tmp);
    // OLED_ShowString(0, 20, tmp, 16);
    // OLED_Refresh();
    // TIM_Cmd(TIM3, ENABLE);
		delay_ms(1);
  }
}

void task2(void)
{
  while (1)
  {
		inx++;
    // TIM_Cmd(TIM3, DISABLE);
		sprintf(tmp, "T2: %05d\r\n", inx);
		// printf(tmp);
    // sprintf(tmp, "Now Count: %03d", inx);
    OLED_ShowString(0, 40, tmp, 16);
    OLED_Refresh();
    // TIM_Cmd(TIM3, ENABLE);
		delay_ms(1);
  }
}

u8 Stacks[2][Stack_Size];

PCB_S
tcb0 = {
    NULL,
    0, 
    NULL
    },
    tcb1 = {
      Stacks[0] + Stack_Size - Drawback,
      0,
      task1
    }, tcb2 = {
      Stacks[1] + Stack_Size - Drawback, 
      0,
      task2
    },
    *c_tcb = NULL;

int main(void)
{
	double a = var_func(10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
	
  c_tcb = &tcb0;
  c_tcb->created = 1;

	set_ppout(LED);
	set_ppout(LED_2);
  delay_init();
  usart1_init(115200);
  delay_ms(10000);
  printf("====START====\r\n");
  lcd_init(1, 0);
  send_string("Hello World!\r\n");
  display_list_char(0, 0, "Hello World!");
  spi1_init();
  oled_init();
  OLED_ColorTurn(1);   // 0正常显示，1 反色显示
  OLED_DisplayTurn(0); // 0正常显示 1 屏幕翻转显示
  OLED_Refresh();
  OLED_ShowString(0, 0, "Hello World!", 16);
  OLED_Refresh();
  sprintf(tmp, "Now count: %03d", inx);

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
  TIM_TimeBaseInitTypeDef TIM_InitStruct;
  TIM_InitStruct.TIM_Period = 100 - 1;    // 周期即ARR自动重装器的值，计1000个数
  TIM_InitStruct.TIM_Prescaler = 7200 - 1;  // PSC预分频器的值，差了个1原因见理论，对72MHz进行7200分频，即10KHz，计10000个数即1s
  TIM_InitStruct.TIM_RepetitionCounter = 0; // 重复计数器的值，高级定时器才有，赋0
  TIM_InitStruct.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_InitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseInit(TIM3, &TIM_InitStruct);
  TIM_ClearFlag(TIM3, TIM_FLAG_Update);
  TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
  NVIC_EnableIRQ(TIM3_IRQn);
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

  NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_Init(&NVIC_InitStructure);
  TIM_Cmd(TIM3, ENABLE);

	set_bit(LED, 1);
  while (1)
  {
		if (inx > 4090) inx = 0; 
		sprintf(tmp, "M: %05d， TIM3->CR1: %p\r\n", inx, TIM3->CR1);
		set_anti_bit(LED);
		if (TIM3->CR1 & TIM_CR1_CEN)
			set_bit(LED_2, 0);
		else
			set_bit(LED_2, 1);
		printf(tmp);
		delay_ms(100);
  }
}

void TIM3_IRQHandler(void)
{
  if (TIM_GetITStatus(TIM3, TIM_IT_Update) == RESET)
		return;
  c_tcb->sp_ptr = get_MSP();
	save_Context_r4_2_r11(c_tcb->registers);
  if (c_tcb == &tcb0)
  {
    c_tcb = &tcb1;
  }
  else if (c_tcb == &tcb1)
  {
    c_tcb = &tcb2;
  }
  else
  {
    c_tcb = &tcb0;
  }

  // 加载下一个任务的堆栈指针
  MSR_MSP(c_tcb->sp_ptr);
	// 如果当前任务尚未创建
  if (!c_tcb->created)
  {
    c_tcb->created = 1;
    set_PC(caller, c_tcb->func);
  } else {
		rec_Context_r4_2_r11(c_tcb->registers);
	}
	TIM_ClearITPendingBit(TIM3, TIM_FLAG_Update); // 清除TIMx的中断待处理位:TIM 中断源
}

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
