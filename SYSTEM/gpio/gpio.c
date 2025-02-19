#include "def.h"
#include "gpio.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_exti.h"
#include "misc.h"

GPIO_TypeDef *GPIOx[] = {GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, GPIOF, GPIOG};
const uint16_t GPIOpin[] = {
    GPIO_Pin_0,
    GPIO_Pin_1,
    GPIO_Pin_2,
    GPIO_Pin_3,
    GPIO_Pin_4,
    GPIO_Pin_5,
    GPIO_Pin_6,
    GPIO_Pin_7,
    GPIO_Pin_8,
    GPIO_Pin_9,
    GPIO_Pin_10,
    GPIO_Pin_11,
    GPIO_Pin_12,
    GPIO_Pin_13,
    GPIO_Pin_14,
    GPIO_Pin_15};
const uint32_t APB_GPIO[] = {
    RCC_APB2Periph_GPIOA,
    RCC_APB2Periph_GPIOB,
    RCC_APB2Periph_GPIOC,
    RCC_APB2Periph_GPIOD,
    RCC_APB2Periph_GPIOE,
    RCC_APB2Periph_GPIOF,
    RCC_APB2Periph_GPIOG};

const uint32_t EXTIx[] = {
    EXTI_Line0,
    EXTI_Line1,
    EXTI_Line2,
    EXTI_Line3,
    EXTI_Line4,
    EXTI_Line5,
    EXTI_Line6,
    EXTI_Line7,
    EXTI_Line8,
    EXTI_Line9,
    EXTI_Line10,
    EXTI_Line11,
    EXTI_Line12,
    EXTI_Line13,
    EXTI_Line14,
    EXTI_Line15};

const IRQn_Type EXTIx_IRQn[] = {EXTI0_IRQn, EXTI1_IRQn, EXTI2_IRQn, EXTI3_IRQn, EXTI4_IRQn, EXTI9_5_IRQn, EXTI9_5_IRQn, EXTI9_5_IRQn, EXTI9_5_IRQn, EXTI9_5_IRQn, EXTI15_10_IRQn, EXTI15_10_IRQn, EXTI15_10_IRQn, EXTI15_10_IRQn, EXTI15_10_IRQn, EXTI15_10_IRQn};

const uint8_t GPIO_PortSourceGPIOx[] = {GPIO_PortSourceGPIOA, GPIO_PortSourceGPIOB, GPIO_PortSourceGPIOC, GPIO_PortSourceGPIOD, GPIO_PortSourceGPIOE, GPIO_PortSourceGPIOF, GPIO_PortSourceGPIOG};

const uint8_t GPIO_PinSourcex[] = {
    GPIO_PinSource0,
    GPIO_PinSource1,
    GPIO_PinSource2,
    GPIO_PinSource3,
    GPIO_PinSource4,
    GPIO_PinSource5,
    GPIO_PinSource6,
    GPIO_PinSource7,
    GPIO_PinSource8,
    GPIO_PinSource9,
    GPIO_PinSource10,
    GPIO_PinSource11,
    GPIO_PinSource12,
    GPIO_PinSource13,
    GPIO_PinSource14,
    GPIO_PinSource15};

#define GPIO_x(port) (GPIOx[port / 16])
#define GPIO_pin(port) (GPIOpin[port % 16])
#define APB_gpio(port) (APB_GPIO[port / 16])

GPIO_InitTypeDef GPIOStructure = {GPIO_Pin_0, GPIO_Speed_50MHz, GPIO_Mode_AF_PP};

EXTI_InitTypeDef EXTI_InitStructure = {EXTI_Line0, EXTI_Mode_Interrupt, EXTI_Trigger_Rising_Falling, ENABLE};

NVIC_InitTypeDef NVIC_InitStructure = {EXTI0_IRQn, 0x02, 0x0, ENABLE};

void port_enable(PortType port, GPIOMode_TypeDef mode)
{
    GPIOStructure.GPIO_Pin = GPIO_pin(port);
    GPIOStructure.GPIO_Mode = mode;
    RCC_APB2PeriphClockCmd(APB_gpio(port), ENABLE);
    GPIO_Init(GPIO_x(port), &GPIOStructure);
}

void set_ain(PortType port)
{
    port_enable(port, GPIO_Mode_AIN);
}

void set_din(PortType port)
{
    port_enable(port, GPIO_Mode_IN_FLOATING);
}

void set_pdin(PortType port)
{
    port_enable(port, GPIO_Mode_IPD);
}

void set_puin(PortType port)
{
    port_enable(port, GPIO_Mode_IPU);
}

void set_ppout(PortType port)
{
    port_enable(port, GPIO_Mode_Out_PP);
}

void set_afpp(PortType port)
{
    port_enable(port, GPIO_Mode_AF_PP);
}

void set_bit(PortType port, u8 val)
{
    if (val)
        GPIO_SetBits(GPIO_x(port), GPIO_pin(port));
    else
        GPIO_ResetBits(GPIO_x(port), GPIO_pin(port));
    // GPIO_x(port)->ODR |= (val << (port % 16));
}

u8 read_bit(PortType port)
{
    return GPIO_ReadInputDataBit(GPIO_x(port), GPIO_pin(port));
    // GPIO_x(port)->ODR |= (0x1 << (port % 16));
    // return GPIO_x(port)->IDR & (0x1 << (port % 16));
}

void set_anti_bit(PortType port)
{
    set_bit(port, !read_bit(port));
}

void set_exti_line(PortType port, uint32_t pp, uint32_t sub_p)
{
    set_puin(port);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOx[port / 16], GPIO_PinSourcex[port % 16]);
    EXTI_InitStructure.EXTI_Line = EXTIx[port % 16];
    EXTI_Init(&EXTI_InitStructure);
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = pp;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = sub_p;
    NVIC_InitStructure.NVIC_IRQChannel = EXTIx_IRQn[port % 16];
    NVIC_Init(&NVIC_InitStructure);
}
