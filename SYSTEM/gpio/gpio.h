#ifndef __GPIO_H
#define __GPIO_H

#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"

#define PA_BASE 0
#define PB_BASE 16
#define PC_BASE 32
#define PD_BASE 48
#define PE_BASE 64
#define PF_BASE 80
#define PG_BASE 96

typedef int PortType;

#define PA(x) ((PortType)(PA_BASE + x))
#define PB(x) ((PortType)(PB_BASE + x))
#define PC(x) ((PortType)(PC_BASE + x))
#define PD(x) ((PortType)(PD_BASE + x))
#define PE(x) ((PortType)(PE_BASE + x))
#define PF(x) ((PortType)(PF_BASE + x))
#define PG(x) ((PortType)(PG_BASE + x))

#define UNSET 0xff

void port_enable(PortType port, GPIOMode_TypeDef mode);

void set_din(PortType port);

void set_ain(PortType port);

void set_pdin(PortType port);

void set_puin(PortType port);

void set_ppout(PortType port);

void set_afpp(PortType port);

void set_bit(PortType port, u8 val);

u8 read_bit(PortType port);

void set_anti_bit(PortType port);

void set_exti_line(PortType port, uint32_t pp, uint32_t sub_p);

#endif
