/*
 * @FilePath: adc_dma.h
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-10-28 15:15:10
 * @copyright : Copyright (c) 2024
 */

#ifndef __ADC_DMA_H__
#define __ADC_DMA_H__

#include "rtthread.h"
#include "board.h"
#include "stm32u5xx_hal.h"

void get_adc_handle(ADC_HandleTypeDef **hadc);
void get_dma_node(DMA_NodeTypeDef **dma_node);
void get_dma_qlist(DMA_QListTypeDef **dma_qlist);
void get_dma_handle(DMA_HandleTypeDef **dma_handle);

#endif