/*
 * @FilePath: voltage_adc.c
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-10-25 14:34:39
 * @copyright : Copyright (c) 2024
 */

#include "voltage_adc.h"
#include "stm32u5xx_hal.h"
#include "adc_dma.h"
#include "tools.h"
#include "logging.h"


ADC_HandleTypeDef *hadc1;
DMA_NodeTypeDef *Node_GPDMA1_Channel1;
DMA_QListTypeDef *List_GPDMA1_Channel1;
DMA_HandleTypeDef *handle_GPDMA1_Channel1;

static void ADC_DMA_Handle_Init(void)
{
    get_adc_handle(&hadc1);
    get_dma_node(&Node_GPDMA1_Channel1);
    get_dma_qlist(&List_GPDMA1_Channel1);
    get_dma_handle(&handle_GPDMA1_Channel1);
}

/**
  * @brief GPDMA1 Initialization Function
  * @param None
  * @retval None
  */
static rt_err_t MX_GPDMA1_Init(void)
{
    rt_err_t res = RT_ERROR;
    /* USER CODE BEGIN GPDMA1_Init 0 */

    /* USER CODE END GPDMA1_Init 0 */

    /* Peripheral clock enable */
    __HAL_RCC_GPDMA1_CLK_ENABLE();

    /* GPDMA1 interrupt Init */
    HAL_NVIC_SetPriority(GPDMA1_Channel1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(GPDMA1_Channel1_IRQn);

    /* USER CODE BEGIN GPDMA1_Init 1 */

    /* ADC1 DMA Init */
    /* GPDMA1_REQUEST_ADC1 Init */
    DMA_NodeConfTypeDef NodeConfig;
    NodeConfig.NodeType = DMA_GPDMA_LINEAR_NODE;
    NodeConfig.Init.Request = GPDMA1_REQUEST_ADC1;
    NodeConfig.Init.BlkHWRequest = DMA_BREQ_SINGLE_BURST;
    NodeConfig.Init.Direction = DMA_PERIPH_TO_MEMORY;
    NodeConfig.Init.SrcInc = DMA_SINC_FIXED;
    NodeConfig.Init.DestInc = DMA_DINC_INCREMENTED;
    NodeConfig.Init.SrcDataWidth = DMA_SRC_DATAWIDTH_HALFWORD;
    NodeConfig.Init.DestDataWidth = DMA_DEST_DATAWIDTH_HALFWORD;
    NodeConfig.Init.SrcBurstLength = 1;
    NodeConfig.Init.DestBurstLength = 1;
    NodeConfig.Init.TransferAllocatedPort = DMA_SRC_ALLOCATED_PORT0|DMA_DEST_ALLOCATED_PORT0;
    NodeConfig.Init.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
    NodeConfig.Init.Mode = DMA_NORMAL;
    NodeConfig.TriggerConfig.TriggerPolarity = DMA_TRIG_POLARITY_MASKED;
    NodeConfig.DataHandlingConfig.DataExchange = DMA_EXCHANGE_NONE;
    NodeConfig.DataHandlingConfig.DataAlignment = DMA_DATA_RIGHTALIGN_ZEROPADDED;
    if (HAL_DMAEx_List_BuildNode(&NodeConfig, Node_GPDMA1_Channel1) != HAL_OK)
    {
        log_error("HAL_DMAEx_List_BuildNode failed.");
        return res;
    }

    if (HAL_DMAEx_List_InsertNode(List_GPDMA1_Channel1, NULL, Node_GPDMA1_Channel1) != HAL_OK)
    {
        log_error("HAL_DMAEx_List_InsertNode failed.");
        return res;
    }

    if (HAL_DMAEx_List_SetCircularMode(List_GPDMA1_Channel1) != HAL_OK)
    {
        log_error("HAL_DMAEx_List_SetCircularMode failed.");
        return res;
    }

    /* USER CODE END GPDMA1_Init 1 */
    /* USER CODE BEGIN GPDMA1_Init 2 */

    /* USER CODE END GPDMA1_Init 2 */
    res = RT_EOK;
    return res;
}

void hal_dma_xfer_cplt_cb(DMA_HandleTypeDef *const _hdma)
{
    rt_kprintf("hal_dma_xfer_cplt_cb\n");
    // HAL_ADC_Stop_DMA(hadc1);
}

void hal_dma_xfer_halfcplt_cb(DMA_HandleTypeDef *const _hdma)
{
    rt_kprintf("hal_dma_xfer_halfcplt_cb\n");
}

void hal_dma_xfer_error_cb(DMA_HandleTypeDef *const _hdma)
{
    rt_kprintf("hal_dma_xfer_error_cb\n");
}

void hal_dma_xfer_abort_cb(DMA_HandleTypeDef *const _hdma)
{
    rt_kprintf("hal_dma_xfer_abort_cb\n");
}

void hal_dma_xfer_suspend_cb(DMA_HandleTypeDef *const _hdma)
{
    rt_kprintf("hal_dma_xfer_suspend_cb\n");
}

static rt_err_t MX_GPDMA1_Channel1(void)
{
    rt_err_t res = RT_ERROR;

    handle_GPDMA1_Channel1->Instance = GPDMA1_Channel1;
    handle_GPDMA1_Channel1->InitLinkedList.Priority = DMA_LOW_PRIORITY_LOW_WEIGHT;
    handle_GPDMA1_Channel1->InitLinkedList.LinkStepMode = DMA_LSM_FULL_EXECUTION;
    handle_GPDMA1_Channel1->InitLinkedList.LinkAllocatedPort = DMA_LINK_ALLOCATED_PORT0;
    handle_GPDMA1_Channel1->InitLinkedList.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
    handle_GPDMA1_Channel1->InitLinkedList.LinkedListMode = DMA_LINKEDLIST_CIRCULAR;
    if (HAL_DMAEx_List_Init(handle_GPDMA1_Channel1) != HAL_OK)
    {
        log_error("HAL_DMAEx_List_Init failed.");
        return res;
    }

    if (HAL_DMAEx_List_LinkQ(handle_GPDMA1_Channel1, List_GPDMA1_Channel1) != HAL_OK)
    {
        log_error("HAL_DMAEx_List_LinkQ failed.");
        return res;
    }

    __HAL_LINKDMA(hadc1, DMA_Handle, *handle_GPDMA1_Channel1);

    if (HAL_DMA_ConfigChannelAttributes(handle_GPDMA1_Channel1, DMA_CHANNEL_NPRIV) != HAL_OK)
    {
        log_error("HAL_DMA_ConfigChannelAttributes failed.");
        return res;
    }

#if 0
    if (HAL_DMA_RegisterCallback(handle_GPDMA1_Channel1, HAL_DMA_XFER_CPLT_CB_ID, hal_dma_xfer_cplt_cb) != HAL_OK)
    {
        log_error("HAL_DMA_RegisterCallback failed.");
        return res;
    }

    if (HAL_DMA_RegisterCallback(handle_GPDMA1_Channel1, HAL_DMA_XFER_HALFCPLT_CB_ID, hal_dma_xfer_halfcplt_cb) != HAL_OK)
    {
        log_error("HAL_DMA_RegisterCallback failed.");
        return res;
    }

    if (HAL_DMA_RegisterCallback(handle_GPDMA1_Channel1, HAL_DMA_XFER_ERROR_CB_ID, hal_dma_xfer_error_cb) != HAL_OK)
    {
        log_error("HAL_DMA_RegisterCallback failed.");
        return res;
    }

    if (HAL_DMA_RegisterCallback(handle_GPDMA1_Channel1, HAL_DMA_XFER_ABORT_CB_ID, hal_dma_xfer_abort_cb) != HAL_OK)
    {
        log_error("HAL_DMA_RegisterCallback failed.");
        return res;
    }

    if (HAL_DMA_RegisterCallback(handle_GPDMA1_Channel1, HAL_DMA_XFER_SUSPEND_CB_ID, hal_dma_xfer_suspend_cb) != HAL_OK)
    {
        log_error("HAL_DMA_RegisterCallback failed.");
        return res;
    }
#endif

    res = RT_EOK;
    return res;
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static rt_err_t MX_ADC1_Init(void)
{

    /* USER CODE BEGIN ADC1_Init 0 */

    /* USER CODE END ADC1_Init 0 */

    rt_err_t res = RT_ERROR;

    /* USER CODE BEGIN ADC1_Init 1 */

    /* USER CODE END ADC1_Init 1 */

    /** Common config
     */
    hadc1->Instance = ADC1;
    hadc1->Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV16;
    hadc1->Init.Resolution = ADC_RESOLUTION_14B;
    hadc1->Init.GainCompensation = 0;
    hadc1->Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1->Init.ScanConvMode = ADC_SCAN_DISABLE;
    hadc1->Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    hadc1->Init.LowPowerAutoWait = DISABLE;
    hadc1->Init.ContinuousConvMode = ENABLE;
    hadc1->Init.NbrOfConversion = 1;
    hadc1->Init.DiscontinuousConvMode = DISABLE;
    hadc1->Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1->Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc1->Init.DMAContinuousRequests = ENABLE;
    hadc1->Init.TriggerFrequencyMode = ADC_TRIGGER_FREQ_LOW;
    hadc1->Init.Overrun = ADC_OVR_DATA_PRESERVED;
    hadc1->Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE;
    hadc1->Init.ConversionDataManagement = ADC_CONVERSIONDATA_DMA_CIRCULAR;
    hadc1->Init.OversamplingMode = DISABLE;
    if (HAL_ADC_Init(hadc1) != HAL_OK)
    {
        log_error("HAL_ADC_Init failed.");
        return res;
    }
    /* USER CODE BEGIN ADC1_Init 2 */

    /* USER CODE END ADC1_Init 2 */
    res = RT_EOK;
    return res;
}

static rt_err_t MX_ADC1_IN_Enable(uint32_t adc_channel)
{
    rt_err_t res = RT_ERROR;
    ADC_ChannelConfTypeDef sConfig = {0};

    /** Configure Regular Channel
     */
    sConfig.Channel = adc_channel;  // ADC_CHANNEL_1, ADC_CHANNEL_2, ADC_CHANNEL_VBAT
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_814CYCLES;
    sConfig.SingleDiff = ADC_SINGLE_ENDED;
    sConfig.OffsetNumber = ADC_OFFSET_NONE;
    sConfig.Offset = 0;
    if (HAL_ADC_ConfigChannel(hadc1, &sConfig) != HAL_OK)
    {
        log_error("HAL_ADC_ConfigChannel failed.");
        return res;
    }
    if (HAL_ADCEx_Calibration_Start(hadc1, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED) != HAL_OK)
    {
        log_error("HAL_ADCEx_Calibration_Start failed.");
        return res;
    }
    res = RT_EOK;
    return res;
}

#define VOL_COLLECTION_MAX_TIME         20
#define VOL_COLLECTION_LIMIT_TIME       23
#define VOL_COLLECTION_RATE             1210
#define CUR_VOL_BUFF_SIZE               (VOL_COLLECTION_LIMIT_TIME * VOL_COLLECTION_RATE)
#define VOL_BUFF_SIZE                   10
#define ADC_VERF                        3300

static uint16_t cur_vol_buff[CUR_VOL_BUFF_SIZE] = {0};
static uint16_t cur_remain_size = 0;
static rt_tick_t cur_sticks, cur_eticks;
static uint16_t vol_buff[VOL_BUFF_SIZE] = {0};
static rt_uint8_t adc_dma_init_tag = 0;

rt_err_t adc_dma_init(void)
{
    rt_err_t res = adc_dma_init_tag == 1 ? RT_EOK : RT_ERROR;
    if (res == RT_EOK)
    {
        return res;
    }

    ADC_DMA_Handle_Init();

    res = MX_GPDMA1_Init();
    if (res != RT_EOK)
    {
        return res;
    }

    res = MX_ADC1_Init();
    if (res != RT_EOK)
    {
        return res;
    }

    res = MX_GPDMA1_Channel1();
    if (res != RT_EOK)
    {
        return res;
    }

    res = RT_EOK;
    adc_dma_init_tag = 1;
    return res;
}

static rt_err_t adc_vol_read(rt_uint32_t adc_channel, rt_uint16_t *value)
{
    rt_err_t res = adc_dma_init_tag == 0 ? RT_ERROR : RT_EOK;
    if (res == RT_ERROR)
    {
        return res;
    }

    res = MX_ADC1_IN_Enable(adc_channel);
    log_debug("MX_ADC1_IN_Enable %s", res_msg(res == RT_EOK));
    if (res != RT_EOK)
    {
        return res;
    }

    int hal_res;
    hal_res = HAL_ADC_Start_DMA(hadc1, (uint32_t *)vol_buff, VOL_BUFF_SIZE);
    res = hal_res == 0 ? RT_EOK : RT_ERROR;
    log_debug("HAL_ADC_Start_DMA %s", res_msg(res == RT_EOK));
    if (res != RT_EOK)
    {
        return res;
    }
    rt_thread_mdelay(10);
    rt_uint16_t remain_size = (rt_uint16_t)__HAL_DMA_GET_COUNTER(handle_GPDMA1_Channel1) / 2;
    hal_res = HAL_ADC_Stop_DMA(hadc1);
    log_debug("HAL_ADC_Stop_DMA %s, hal_res=%d, remain_size=%ld", hal_res == HAL_OK ? "success" : "failed", hal_res, remain_size);

    rt_uint16_t coll_size = VOL_BUFF_SIZE > remain_size ? VOL_BUFF_SIZE - remain_size : VOL_BUFF_SIZE;

    rt_uint16_t sum_val = 0;
    rt_uint16_t sum_cnt = 0;
    for (rt_uint16_t i = 0; i < VOL_BUFF_SIZE; i ++){
        // log_debug("vol_buff[%d]=%d", i, vol_buff[i]);
        sum_val += vol_buff[i];
        sum_cnt++;
    }
    log_debug("sum_val=%d, sum_cnt=%d, avg_val=%d", sum_val, sum_cnt, sum_val / sum_cnt);
    *value = (sum_val / sum_cnt) * ADC_VERF / ((1 << 14) - 1);

    return res;
}

rt_err_t vcap_vol_read(rt_uint16_t *value)
{
    rt_uint16_t vol_val;
    rt_err_t res = adc_vol_read(ADC_CHANNEL_2, &vol_val);
    if (res == RT_EOK)
    {
        *value = vol_val;
    }
    return res;
}

rt_err_t vbat_vol_read(rt_uint16_t *value)
{
    rt_uint16_t vol_val;
    rt_err_t res = adc_vol_read(ADC_CHANNEL_VBAT, &vol_val);
    if (res == RT_EOK)
    {
        *value = vol_val * 4;
    }
    return res;
}

rt_err_t cur_vol_read_start(void)
{
    rt_err_t res = adc_dma_init_tag == 0 ? RT_ERROR : RT_EOK;
    if (res == RT_ERROR)
    {
        return res;
    }

    res = MX_ADC1_IN_Enable(ADC_CHANNEL_1);
    if (res != RT_EOK)
    {
        return res;
    }
    
    int hal_res;
    cur_remain_size = 0;
    cur_sticks = rt_tick_get_millisecond();
    hal_res = HAL_ADC_Start_DMA(hadc1, (uint32_t *)cur_vol_buff, CUR_VOL_BUFF_SIZE);
    res = hal_res == 0 ? RT_EOK : RT_ERROR;
    if (res != RT_EOK)
    {
        return res;
    }

    return res;
}

rt_err_t cur_vol_read_stop(void)
{
    rt_err_t res = adc_dma_init_tag == 0 ? RT_ERROR : RT_EOK;
    if (res == RT_ERROR)
    {
        return res;
    }
    cur_remain_size = (uint16_t)__HAL_DMA_GET_COUNTER(handle_GPDMA1_Channel1) / 2;
    int hal_res = HAL_ADC_Stop_DMA(hadc1);
    cur_eticks = rt_tick_get_millisecond();
    res = hal_res == HAL_OK ? RT_EOK : RT_ERROR;

    return res;
}

rt_err_t cur_vol_read(rt_uint16_t **cur_buff, rt_uint16_t *buff_size)
{
    rt_err_t res = RT_EOK;
    log_debug(
        "cur_vol_read cur_remain_size=%ld, cur_sticks=%ld, cur_eticks=%ld, run_ticks=%ld",
        cur_remain_size, cur_sticks, cur_eticks, rt_tick_diff(cur_sticks, cur_eticks)
    );

    rt_uint16_t coll_size = 0;
    if (cur_remain_size < ((VOL_COLLECTION_LIMIT_TIME - VOL_COLLECTION_MAX_TIME) * VOL_COLLECTION_RATE))
    {
        coll_size = VOL_COLLECTION_MAX_TIME *  VOL_COLLECTION_RATE;
    }
    else if (rt_tick_diff(cur_sticks, cur_eticks) > 19 * 1000 && (CUR_VOL_BUFF_SIZE - cur_remain_size < rt_tick_diff(cur_sticks, cur_eticks)))
    {
        coll_size = VOL_COLLECTION_MAX_TIME *  VOL_COLLECTION_RATE;
    }
    else
    {
        coll_size = CUR_VOL_BUFF_SIZE - cur_remain_size;
    }

    for (rt_uint16_t i = 0; i < coll_size; i++)
    {
        cur_vol_buff[i] = cur_vol_buff[i] * ADC_VERF / ((1 << 14) - 1);
    }

    *cur_buff = cur_vol_buff;
    *buff_size = coll_size;

    return res;
}

void test_vol_read(void)
{
    rt_err_t res;

    res = adc_dma_init();
    log_debug("adc_dma_init %s", res_msg(res == RT_EOK));
    if (res != RT_EOK)
    {
        return;
    }

    rt_uint16_t vcap_vol, vbat_vol;
    vcap_vol = vbat_vol = 0;

    res = vcap_vol_read(&vcap_vol);
    log_debug("vcap_vol_read %s, vcap_vol=%d", res_msg(res == RT_EOK), vcap_vol);

    res = vbat_vol_read(&vbat_vol);
    log_debug("vbat_vol_read %s, vbat_vol=%d", res_msg(res == RT_EOK), vbat_vol);

    res = cur_vol_read_start();
    log_debug("cur_vol_read_start %s", res_msg(res == RT_EOK));

    rt_thread_mdelay(100);

    res = cur_vol_read_stop();
    log_debug("cur_vol_read_stop %s", res_msg(res == RT_EOK));

    rt_uint16_t *cur_buff;
    rt_uint16_t cur_buff_size = 0;
    res = cur_vol_read(&cur_buff, &cur_buff_size);
    log_debug(
        "cur_vol_read %s, cur_buff_size=%d, cur_buff=0x%08X, cur_vol_buff=0x%08X",
        res_msg(res == RT_EOK), cur_buff_size, cur_buff, cur_vol_buff
    );

    for (rt_uint16_t i = 0; i < 10; i++)
    {
        log_debug("cur_vol cur_buff[%d]=%d", i, cur_buff[i]);
    }
}