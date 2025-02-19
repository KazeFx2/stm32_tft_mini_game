#include "threads.h"
#include <string.h>

#define PRESCALER 720
#define PERIOD 10

#define DUR_US ((uint64_t)PRESCALER * PERIOD / 72)

// 16 * 4 Bytes
#define STACK_DRAWBACK 4 * 10

uint8_t stack[MAX_THREADS][STACK_SIZE];

volatile uint8_t no_schedule = 0;

uint8_t joined_threads = 0;

uint64_t timestamp = 0;

static uint8_t fac_us = 0;
static uint16_t fac_ms = 0;

typedef struct PCB_context_s
{
  u_ptr sp;
  u_ptr registers[8];
} PCB_context_t;

typedef struct PCB_s
{
  uint8_t status;
  uint8_t status_resume;
  u_ptr task_entrance;
  uint8_t waiting_type;
	uint64_t it_type;
	uint8_t no_join;
  uint64_t delay_count_us;
  uint64_t return_val;
	int waiter;
  PCB_context_t context;
} PCB_t;

PCB_t pcb_array[MAX_THREADS];

uint8_t current_task = 0;

// asm
__asm uint32_t MRS_PSR(void)
{
  MRS r0, PSR
  BX r14
}
__asm uint32_t MRS_MSP(void)
{
  MRS r0, MSP
  BX r14
}
__asm void SUB_MSP(uint32_t sub)
{
	MRS r1, MSP
	SUB r1, r0
	MSR MSP, r1
  BX r14
}
__asm void ADD_MSP(uint32_t add)
{
	MRS r1, MSP
	ADD r1, r0
	MSR MSP, r1
  BX r14
}
__asm void LDR_0_3(uint32_t offest)
{
	MRS r1, MSP
	ADD r1, r0
	LDM r1, {r0-r3}
	BX r14
}
__asm uint32_t MOV_R2(void)
{
	MOV r0, r2
	BX r14
}
__asm uint32_t MOV_R3(void)
{
	MOV r0, r3
	BX r14
}
__asm uint32_t LDR_SP(uint32_t offest)
{
	MRS r1, MSP
	LDR r0, [r1, r0]
	BX r14
}
__asm STR_SP(uint32_t offest, uint32_t value)
{
	MRS r2, MSP
	STR r1, [r2, r0]
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
// asm end


void set_it_type(uint64_t type)
{
	pcb_array[current_task].it_type |= type;
	pcb_array[current_task].waiting_type = PCB_WAITING_TYPE_IT;
	pcb_array[current_task].status = PCB_STATUS_WAITING;
}
void clr_it_type(uint64_t type)
{
	for (uint8_t i = 0; i < MAX_THREADS; i++)
	{
		if (pcb_array[i].status == PCB_STATUS_WAITING && pcb_array[i].waiting_type == PCB_WAITING_TYPE_IT && pcb_array[i].it_type == type)
		{
			pcb_array[i].it_type ^= type;
			if (!pcb_array[i].it_type)
				pcb_array[i].status = PCB_STATUS_READY;
		}
	}
}
u_ptr four_bytes_align(u_ptr ptr)
{
	return (uint32_t)ptr & 0xfffffffc;
}

void caller(u_ptr func, u_ptr pcb, uint32_t n_args, u_ptr p0, ...)
{
	uint32_t ps[4] = {p0, p0};
	uint8_t i;
	uint64_t ret;
	PCB_t *pcb_p;
	if (n_args > 1)
	{
		for (i = 0; i < n_args - 1; i++)
			ps[i + 1] = LDR_SP(6 * 4 + i * 4);
		for (; i < n_args + 3; i++)
			ps[i + 1] = ps[i + 1 - n_args];
		for (i = 0; i < n_args; i++)
			ps[i] = ps[i + 4];
	}
	LDR_0_3(n_args * 4);
	ret = ((uint64_t (*)(void))func)();
	__ATOM_START
	pcb_p = pcb;
	pcb_p->return_val = ret;
	if (pcb_p->no_join) {
		pcb_p->status = PCB_STATUS_JOINED;
		joined_threads++;
	}
	else
		pcb_p->status = PCB_STATUS_TERMINATED;
	if (pcb_p->waiter != -1)
		pcb_array[pcb_p->waiter].status = PCB_STATUS_READY;
	__ATOM_END
	while(1)
	{
	}
}

void init_threads_manager(void)
{
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);
	fac_us = SystemCoreClock / 8000000;
	fac_ms = (uint16_t)fac_us * 1000;
	
  for (uint8_t i = 1; i < MAX_THREADS; i++)
  {
    pcb_array[i].status = PCB_STATUS_JOINED;
  }
  current_task = 0;
	pcb_array[current_task].waiter = -1;
  pcb_array[current_task].status = PCB_STATUS_RUNNING;
	joined_threads = MAX_THREADS - 1;

  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
  TIM_TimeBaseInitTypeDef TIM_InitStruct;
  TIM_InitStruct.TIM_Period = PERIOD - 1;
  TIM_InitStruct.TIM_Prescaler = PRESCALER - 1;
  TIM_InitStruct.TIM_RepetitionCounter = 0;
  TIM_InitStruct.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_InitStruct.TIM_ClockDivision = TIM_CKD_DIV4;
  TIM_TimeBaseInit(TIM3, &TIM_InitStruct);
  TIM_ClearFlag(TIM3, TIM_FLAG_Update);
  TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
  NVIC_EnableIRQ(TIM3_IRQn);

  NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_Init(&NVIC_InitStructure);
  TIM_Cmd(TIM3, ENABLE);
}

void TIM3_IRQHandler(void)
{
  if (TIM_GetITStatus(TIM3, TIM_IT_Update) == RESET)
    return;
	timestamp += DUR_US;
  uint8_t max = MAX_THREADS, new_task = current_task + 1;
  while (max--)
  {
    if (new_task == MAX_THREADS)
      new_task = 0;
		if (pcb_array[new_task].status == PCB_STATUS_WAITING && pcb_array[new_task].waiting_type == PCB_WAITING_TYPE_DELAY) 
		{
			if (pcb_array[new_task].delay_count_us < DUR_US) pcb_array[new_task].status = PCB_STATUS_READY;
			else pcb_array[new_task].delay_count_us -= DUR_US;
		}
    if (pcb_array[new_task].status == PCB_STATUS_READY)
		{
			max--;
      break;
		}
    new_task++;
  }
  if (new_task == MAX_THREADS)
    new_task = 0;
  if (pcb_array[new_task].status == PCB_STATUS_READY && !no_schedule)
  {
		u_ptr sp = MRS_MSP();
		if (((u_ptr *)sp)[3] == 0xfffffff9) {
			pcb_array[current_task].context.sp = sp;
			if (pcb_array[current_task].status == PCB_STATUS_RUNNING)
				pcb_array[current_task].status = PCB_STATUS_READY;
			save_Context_r4_2_r11(pcb_array[current_task].context.registers);

			current_task = new_task;
			pcb_array[current_task].status = PCB_STATUS_RUNNING;
			MSR_MSP(pcb_array[current_task].context.sp);
			rec_Context_r4_2_r11(pcb_array[current_task].context.registers);
		}
  }
	max++;
	while (max--)
	{
		new_task++;
    if (new_task == MAX_THREADS)
      new_task = 0;
		if (pcb_array[new_task].status == PCB_STATUS_WAITING && pcb_array[new_task].waiting_type == PCB_WAITING_TYPE_DELAY) 
		{
			if (pcb_array[new_task].delay_count_us < DUR_US) pcb_array[new_task].status = PCB_STATUS_READY;
			else pcb_array[new_task].delay_count_us -= DUR_US;
		}
	}
  TIM_ClearITPendingBit(TIM3, TIM_FLAG_Update);
}

ThreadHandlerType add_thread(u_ptr func, uint32_t n_args, ...)
{
  uint8_t i;
  PCB_t *target = NULL;
  uint32_t stack_args = n_args > 2 ? n_args - 1 : 0;
  u_ptr sp = MRS_MSP(), r2 = MOV_R2(), r3 = MOV_R3();
  u_ptr *stack_start = NULL;
  __ATOM_START
  if (joined_threads == 0 || joined_threads > MAX_THREADS)
    return 0;
	joined_threads--;
  for (i = 0; i < MAX_THREADS; i++)
  {
    if (pcb_array[i].status == PCB_STATUS_JOINED)
    {
      pcb_array[i].status = PCB_STATUS_CREATED;
      target = &pcb_array[i];
			break;
    }
  }
  __ATOM_END
	target->it_type = 0;
  target->task_entrance = func;
  target->context.sp = four_bytes_align(&stack[i][STACK_SIZE - STACK_DRAWBACK - 4 * 2 - stack_args * 4]);
	if(stack_args > 1)
		memcpy(target->context.sp + STACK_DRAWBACK + 4 * 3, sp + 4 * 10, (stack_args - 1) * 4);
  stack_start = target->context.sp;
  stack_start[3] = 0xfffffff9;

  stack_start[4] = func;
  stack_start[5] = target;
  stack_start[6] = n_args;
  stack_start[7] = r2;

  stack_start[10] = caller;
  stack_start[11] = 0x01000000;
	stack_start[12] = r3;
	return target;
}

void start_thread(ThreadHandlerType thread, uint8_t no_join)
{
	if(!thread) return;
  PCB_t *pcb = thread;
	pcb->no_join = no_join;
  pcb->status = PCB_STATUS_READY;
}

void frozen_thread(ThreadHandlerType thread)
{
	if (!thread) return;
	PCB_t *target = thread;
	__ATOM_START
	target->status_resume = target->status;
	target->status = PCB_STATUS_FROZEN;
	__ATOM_END
}

void resume_thread(ThreadHandlerType thread)
{
	if (!thread) return;
	PCB_t *target = thread;
	if (target->status != PCB_STATUS_FROZEN) return;
	__ATOM_START
	target->status = target->status_resume;
	__ATOM_END
}

void delay_us(uint32_t dur)
{
	if (!dur) return;
	uint32_t psr = MRS_PSR();
	if (psr & 0xff)
	{
    uint32_t temp;
    SysTick->LOAD = dur * fac_us;
    SysTick->VAL = 0x00;
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
    do
    {
        temp = SysTick->CTRL;
    } while ((temp & 0x01) && !(temp & (1 << 16)));
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
    SysTick->VAL = 0X00;
		return;
	}
	__ATOM_START
	pcb_array[current_task].delay_count_us = dur;
	pcb_array[current_task].waiting_type = PCB_WAITING_TYPE_DELAY;
	pcb_array[current_task].status = PCB_STATUS_WAITING;
	__ATOM_END
	while(pcb_array[current_task].status == PCB_STATUS_WAITING)
	{
	}
}

void delay_ms(uint32_t dur)
{
	if (!dur) return;
	uint32_t psr = MRS_PSR();
	if (psr & 0xff)
	{
		uint32_t temp;
    SysTick->LOAD = dur * fac_ms;
    SysTick->VAL = 0x00;
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
    do
    {
        temp = SysTick->CTRL;
    } while ((temp & 0x01) && !(temp & (1 << 16)));
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
    SysTick->VAL = 0X00;
		return;
	}
	__ATOM_START
	pcb_array[current_task].delay_count_us = dur * 1000;
	pcb_array[current_task].waiting_type = PCB_WAITING_TYPE_DELAY;
	pcb_array[current_task].status = PCB_STATUS_WAITING;
	__ATOM_END
	while(pcb_array[current_task].status == PCB_STATUS_WAITING)
	{
	}
}

uint64_t join_thread(ThreadHandlerType thread)
{
	PCB_t *target = thread;
	__ATOM_START
	target->waiter = current_task;
	__ATOM_END
	if (target->status != PCB_STATUS_TERMINATED)
	{	
		pcb_array[current_task].waiting_type = PCB_WAITING_TYPE_JOIN;
		pcb_array[current_task].status = PCB_STATUS_WAITING;
		while(target->status != PCB_STATUS_TERMINATED)
		{
		}
	}
	__ATOM_START
	uint64_t ret = target->return_val;
	target->waiter = -1;
	target->status = PCB_STATUS_JOINED;
	joined_threads++;
	__ATOM_END
	return ret;
}

uint64_t time(void)
{
	return timestamp;
}
