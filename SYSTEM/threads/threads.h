#ifndef __THREADS_H
#define __THREADS_H

#include "def.h"
#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_tim.h"
#include "misc.h"

#define JOIN 0x0
#define NO_JOIN 0x1

#define STACK_SIZE 1024 * 1

#define MAX_THREADS 8

#define __ATOM_START no_schedule = 1;
#define __ATOM_END no_schedule = 0;

#define PCB_STATUS_CREATED 0x1
#define PCB_STATUS_TERMINATED 0x2
#define PCB_STATUS_RUNNING 0x4
#define PCB_STATUS_WAITING 0x8
#define PCB_STATUS_READY 0x10
#define PCB_STATUS_JOINED 0x20
#define PCB_STATUS_FROZEN 0x40

#define PCB_WAITING_TYPE_DELAY 0x1
#define PCB_WAITING_TYPE_JOIN 0x2
#define PCB_WAITING_TYPE_IT 0x4

#define PCB_IT_TYPE_USART 0x1

extern volatile uint8_t no_schedule;

typedef void * u_ptr;

typedef uint32_t ThreadHandlerType;

void set_it_type(uint64_t type);

void clr_it_type(uint64_t type);

void init_threads_manager(void);

ThreadHandlerType add_thread(u_ptr func, uint32_t n_args, ...);

void start_thread(ThreadHandlerType thread, uint8_t no_join);

uint64_t join_thread(ThreadHandlerType thread);

void frozen_thread(ThreadHandlerType thread);

void resume_thread(ThreadHandlerType thread);

void delay_us(uint32_t dur);

void delay_ms(uint32_t dur);

uint64_t time(void);

#endif
