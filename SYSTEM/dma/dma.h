#ifndef __DMA_H
#define __DMA_H
#include "def.h"
#include "stm32f10x_dma.h"
#include "stm32f10x_rcc.h"

void MYDMA_Config(DMA_Channel_TypeDef *DMA_CHx, u32 cpar, u32 cmar, u16 cndtr);

void MYDMA_Config1(DMA_Channel_TypeDef *DMA_CHx, u32 cpar, u32 cmar, u16 cndtr);

// 开启一次DMA传输
void MYDMA_Enable(DMA_Channel_TypeDef *DMA_CHx);

#endif
