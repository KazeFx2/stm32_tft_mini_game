#include "def.h"
#include "usart.h"
#include "threads.h"
#include "lcd_def.h"

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_rcc.h"
#include "misc.h"

#define STDIN_SIZE 1024
char stdin_buf[STDIN_SIZE];
u16 stdin_pos = 0;
u16 stdin_len = 0;

USART_TypeDef *ACT_USART;

#pragma import(__use_no_semihosting)
struct __FILE
{
	int handle;
};
FILE __stdout, __stdin;
void _sys_exit(int x)
{
	x = x;
}
int fputc(int ch, FILE *f)
{
	send_char(ch);
	return ch;
}

int fgetc(FILE *f)
{
	return read_char();
}

uint8_t on_send_str = 0;

/*
typedef enum
{
	GPIO_Mode_AIN = 0x0,
	GPIO_Mode_IN_FLOATING = 0x04,
	GPIO_Mode_IPD = 0x28,
	GPIO_Mode_IPU = 0x48,
	GPIO_Mode_Out_OD = 0x14,
	GPIO_Mode_Out_PP = 0x10,
	GPIO_Mode_AF_OD = 0x1C,
	GPIO_Mode_AF_PP = 0x18
}
GPIOMode_TypeDef;
+--------------------------------+-----------------------------------+--------------------+----------------------------------------------------------------------------------------------------------------------------------+
 | 英语简写									| 英语全称										 | 中文名称				   | 功能描述																																												  |
+--------------------------------+-----------------------------------+--------------------+----------------------------------------------------------------------------------------------------------------------------------+
 | GPIO_Mode_AIN					| Analog Input								 | 模拟输入				   | 将 GPIO 端口配置为模拟输入模式，用于连接模拟传感器或接收模拟信号。																				  |
+--------------------------------+-----------------------------------+--------------------+----------------------------------------------------------------------------------------------------------------------------------+
 | GPIO_Mode_IN_FLOATING	| Input Floating							  | 浮空输入			    | 将 GPIO 端口配置为浮空输入模式，没有外部上拉或下拉电阻。适用于连接外部信号源，不需要外部电阻来提供稳定的电平。	 |
+--------------------------------+-----------------------------------+--------------------+----------------------------------------------------------------------------------------------------------------------------------+
 | GPIO_Mode_IPD					| Input Pull-Down						   | 上拉输入			 	 | 将 GPIO 端口配置为上拉输入模式，使得端口上的电平在未连接时被拉低，通过内部电阻连接到地。											 |
+--------------------------------+-----------------------------------+--------------------+----------------------------------------------------------------------------------------------------------------------------------+
 | GPIO_Mode_IPU					| Input Pull-Up								  | 下拉输入				| 将 GPIO 端口配置为下拉输入模式，使得端口上的电平在未连接时被拉高，通过内部电阻连接到 VDD。									 |
+--------------------------------+-----------------------------------+--------------------+----------------------------------------------------------------------------------------------------------------------------------+
 | GPIO_Mode_Out_OD				| Output Open-Drain						| 开漏输出				  | 将 GPIO 端口配置为开漏输出模式，可以连接到外部设备，并且可以由 GPIO 输出低电平，但输出高电平时会断开连接。			 |
+--------------------------------+-----------------------------------+--------------------+----------------------------------------------------------------------------------------------------------------------------------+
 | GPIO_Mode_Out_PP				 | Output Push-Pull							| 推挽输出		  		  | 将 GPIO 端口配置为推挽输出模式，可以输出高电平或低电平，适用于驱动外部设备。															  |
+--------------------------------+-----------------------------------+--------------------+----------------------------------------------------------------------------------------------------------------------------------+
 | GPIO_Mode_AF_OD				 | Alternate Function Open-Drain  | 替代功能开漏输出  | 将 GPIO 端口配置为替代功能开漏输出模式，用于连接到外设的备用功能，并且可以输出低电平，但输出高电平时会断开连接。  |
+--------------------------------+-----------------------------------+--------------------+----------------------------------------------------------------------------------------------------------------------------------+
 | GPIO_Mode_AF_PP				  | Alternate Function Push-Pull	  | 替代功能推挽输出  | 将 GPIO 端口配置为替代功能推挽输出模式，用于连接到外设的备用功能，并且可以输出高电平或低电平。								   |
+--------------------------------+-----------------------------------+--------------------+----------------------------------------------------------------------------------------------------------------------------------+
*/
void usart1_init(u32 bound)
{
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	// Enable Clock of GPIOA and AFIO (Alternate Function I/O) and USART1
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	// Init GPIOA9 (TXD - AF_PP) and GPIOA10 (RXD - IF)
	port_enable(PA(9), GPIO_Mode_AF_PP);
	port_enable(PA(10), GPIO_Mode_IN_FLOATING);

	// NVIC
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	// Init USART1
	USART_InitStructure.USART_BaudRate = bound;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART1, &USART_InitStructure);
	// USART Interrupt
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	// USART1 Enable
	USART_Cmd(USART1, ENABLE);

	// USART_GetFlagStatus(USART1, USART_FLAG_TC);

	// Set UASRT1
	ACT_USART = USART1;
}

void USART1_IRQHandler(void)
{
	if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{
		if (stdin_len == 0)
			clr_it_type(PCB_IT_TYPE_USART);
		stdin_buf[(stdin_pos + stdin_len) % STDIN_SIZE] = USART_ReceiveData(USART1);
		stdin_len++;
	}
}

void send_string(char *str)
{
	u8 ch = 0;
	while (on_send_str)
		;
	on_send_str = 1;
	while ((ch = *(str++)) != 0)
	{
		send_char(ch);
	}
	on_send_str = 0;
}

void send_char(char ch)
{
	while ((ACT_USART->SR & 0x40) == 0)
		;
	ACT_USART->DR = (u8)ch;
}

char read_char(void)
{
	__ATOM_START
	if (stdin_len == 0)
		set_it_type(PCB_IT_TYPE_USART);
	__ATOM_END
	while (stdin_len == 0)
		;
	stdin_len--;
	char ret = stdin_buf[stdin_pos];
	stdin_pos = (stdin_pos + 1) % STDIN_SIZE;
	return ret;
}
