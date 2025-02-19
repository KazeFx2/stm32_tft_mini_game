#include "def.h"
#include "spi.h"
#include "gpio.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"

void spi1_init(void)
{
    SPI_InitTypeDef SPI_InitStructure;

    // PA4 NSS/CS
    // PA5 SCL/SCK
    // PA6 MISO
    // PA7 MOSI

    port_enable(PA(5), GPIO_Mode_AF_PP);
    port_enable(PA(7), GPIO_Mode_AF_PP);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

    SPI_InitStructure.SPI_Direction = SPI_Direction_1Line_Tx;          // 只发送模式
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;                      // 设置SPI工作模式：主机模式
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;                  // 设置SPI数据大小：8位帧结构
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;                        // 串行同步时钟空闲时SCLK位高电平
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;                       // 串行同步时钟空第二个时钟沿捕获
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;                          // NSS信号由软件管理
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2; // 波特率预分频值：波特率预分频值为2
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;                 // 数据传输高位先行
    SPI_InitStructure.SPI_CRCPolynomial = 7;                           // CRC值计算的多项式
    SPI_Init(SPI1, &SPI_InitStructure);                                // 初始化SPI
    SPI_Cmd(SPI1, ENABLE);                                             // 使能SPI
}

void spi2_init(void)
{
    SPI_InitTypeDef SPI_InitStructure;

    // PB12 NSS/CS
    // PB13 SCL/SCK
    // PB14 MISO
    // PB15 MOSI

    port_enable(PB(13), GPIO_Mode_AF_PP);
    port_enable(PB(15), GPIO_Mode_AF_PP);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);

    SPI_InitStructure.SPI_Direction = SPI_Direction_1Line_Tx;          // 只发送模式
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;                      // 设置SPI工作模式：主机模式
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;                  // 设置SPI数据大小：8位帧结构
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;                        // 串行同步时钟空闲时SCLK位高电平
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;                       // 串行同步时钟空第二个时钟沿捕获
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;                          // NSS信号由软件管理
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2; // 波特率预分频值：波特率预分频值为2
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;                 // 数据传输高位先行
    SPI_InitStructure.SPI_CRCPolynomial = 7;                           // CRC值计算的多项式
    SPI_Init(SPI2, &SPI_InitStructure);                                // 初始化SPI
    SPI_Cmd(SPI2, ENABLE);                                             // 使能SPI
}
