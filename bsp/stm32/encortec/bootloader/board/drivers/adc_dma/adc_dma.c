/*
 * @FilePath: adc_dma.c
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-10-28 15:15:03
 * @copyright : Copyright (c) 2024
 */

#include "adc_dma.h"

// #define DBG_SECTION_NAME "ADC_DMA"
// #define DBG_LEVEL DBG_LOG
// #include <rtdbg.h>

ADC_HandleTypeDef hadc1;
DMA_NodeTypeDef Node_GPDMA1_Channel1;
DMA_QListTypeDef List_GPDMA1_Channel1;
DMA_HandleTypeDef handle_GPDMA1_Channel1;

void get_adc_handle(ADC_HandleTypeDef **hadc)
{
    *hadc = &hadc1;
}

void get_dma_node(DMA_NodeTypeDef **dma_node)
{
    *dma_node = &Node_GPDMA1_Channel1;
}

void get_dma_qlist(DMA_QListTypeDef **dma_qlist)
{
    *dma_qlist = &List_GPDMA1_Channel1;
}

void get_dma_handle(DMA_HandleTypeDef **dma_handle)
{
    *dma_handle = &handle_GPDMA1_Channel1;
}

/**
  * @brief This function handles GPDMA1 Channel 1 global interrupt.
  */
void GPDMA1_Channel1_IRQHandler(void)
{
    /* USER CODE BEGIN GPDMA1_Channel1_IRQn 0 */

    /* USER CODE END GPDMA1_Channel1_IRQn 0 */
    HAL_DMA_IRQHandler(&handle_GPDMA1_Channel1);
    /* USER CODE BEGIN GPDMA1_Channel1_IRQn 1 */

    /* USER CODE END GPDMA1_Channel1_IRQn 1 */
}

#if 0
/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV16;
  hadc1.Init.Resolution = ADC_RESOLUTION_14B;
  hadc1.Init.GainCompensation = 0;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.TriggerFrequencyMode = ADC_TRIGGER_FREQ_LOW;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE;
  hadc1.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DMA_CIRCULAR;
  hadc1.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  // sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_814CYCLES;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  // if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  // {
  //   Error_Handler();
  // }

  /** Configure Regular Channel
  */
  // sConfig.Channel = ADC_CHANNEL_2;
  // sConfig.Rank = ADC_REGULAR_RANK_2;
  // if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  // {
  //   Error_Handler();
  // }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_VBAT;
  // sConfig.Rank = ADC_REGULAR_RANK_3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

  if (HAL_ADCEx_Calibration_Start(&hadc1, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief GPDMA1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPDMA1_Init(void)
{

  /* USER CODE BEGIN GPDMA1_Init 0 */

  /* USER CODE END GPDMA1_Init 0 */

  /* Peripheral clock enable */
  __HAL_RCC_GPDMA1_CLK_ENABLE();

  /* GPDMA1 interrupt Init */
    HAL_NVIC_SetPriority(GPDMA1_Channel1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(GPDMA1_Channel1_IRQn);

  /* USER CODE BEGIN GPDMA1_Init 1 */

  /* USER CODE END GPDMA1_Init 1 */
  /* USER CODE BEGIN GPDMA1_Init 2 */

  /* USER CODE END GPDMA1_Init 2 */

}

/**
  * @brief This function handles GPDMA1 Channel 1 global interrupt.
  */
void GPDMA1_Channel1_IRQHandler(void)
{
  /* USER CODE BEGIN GPDMA1_Channel1_IRQn 0 */

  /* USER CODE END GPDMA1_Channel1_IRQn 0 */
  HAL_DMA_IRQHandler(&handle_GPDMA1_Channel1);
  /* USER CODE BEGIN GPDMA1_Channel1_IRQn 1 */

  /* USER CODE END GPDMA1_Channel1_IRQn 1 */
}

#define ADC_DMA_BUFF_SIZE 0x2000
static uint32_t ADC_DMA_BUFF[ADC_DMA_BUFF_SIZE] = {0};

void test_read_adc_dma(void)
{
    LOG_D("test_read_adc_dma start");
    MX_GPDMA1_Init();
    MX_ADC1_Init();
    LOG_D("MX_ADC1_Init over.");

    int res;
    rt_uint32_t less_num = __HAL_DMA_GET_COUNTER(&handle_GPDMA1_Channel1);
    LOG_D("less num %ld", less_num);
    rt_uint16_t empty_cnt = 0;
    rt_memset(ADC_DMA_BUFF, 0, ADC_DMA_BUFF_SIZE);
    res = HAL_ADC_Start_DMA(&hadc1, (uint32_t *)ADC_DMA_BUFF, ADC_DMA_BUFF_SIZE);
    LOG_D("HAL_ADC_Start_DMA %s", res == HAL_OK ? "success" : "failed");

    if (res == HAL_OK)
    {
        rt_thread_mdelay(1000);
        res = HAL_ADC_Stop_DMA(&hadc1);
        LOG_D("HAL_ADC_Stop_DMA %s", res == HAL_OK ? "success" : "failed");
        for (rt_uint16_t i = 0; i < ADC_DMA_BUFF_SIZE; i++)
        {
            if (ADC_DMA_BUFF[i] != 0)
            {
                LOG_D("ADC_DMA_BUFF[%ld]=%ld", i, ADC_DMA_BUFF[i]);
            }
            else
            {
                empty_cnt++;
            }
        }
        LOG_D("Empty cnt %ld", empty_cnt);
    }
    less_num = __HAL_DMA_GET_COUNTER(&handle_GPDMA1_Channel1);
    LOG_D("less num %ld", less_num);
}
#endif