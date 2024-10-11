/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file         stm32u5xx_hal_msp.c
  * @brief        This file provides code for the MSP Initialization
  *               and de-Initialization codes.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
/* USER CODE BEGIN Includes */
#include <drv_common.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN Define */

/* USER CODE END Define */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN Macro */

/* USER CODE END Macro */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static void HAL_USART1_MspInit(GPIO_InitTypeDef *GPIO_InitStruct, RCC_PeriphCLKInitTypeDef *PeriphClkInit);
static void HAL_USART3_MspInit(GPIO_InitTypeDef *GPIO_InitStruct, RCC_PeriphCLKInitTypeDef *PeriphClkInit);
static void HAL_UART4_MspInit(GPIO_InitTypeDef *GPIO_InitStruct, RCC_PeriphCLKInitTypeDef *PeriphClkInit);
static void HAL_UART5_MspInit(GPIO_InitTypeDef *GPIO_InitStruct, RCC_PeriphCLKInitTypeDef *PeriphClkInit);
static void HAL_LPUART1_MspInit(GPIO_InitTypeDef *GPIO_InitStruct, RCC_PeriphCLKInitTypeDef *PeriphClkInit);

static void HAL_USART1_MspDeInit(void);
static void HAL_USART3_MspDeInit(void);
static void HAL_UART4_MspDeInit(void);
static void HAL_UART5_MspDeInit(void);
static void HAL_LPUART1_MspDeInit(void);

static void HAL_ADC1_MspInit(GPIO_InitTypeDef *GPIO_InitStruct, RCC_PeriphCLKInitTypeDef *PeriphClkInit);
static void HAL_ADC1_MspDeInit(void);

static void HAL_I2C1_MspInit(GPIO_InitTypeDef *GPIO_InitStruct, RCC_PeriphCLKInitTypeDef *PeriphClkInit);
static void HAL_I2C1_MspDeInit(void);
/* USER CODE END PFP */

/* External functions --------------------------------------------------------*/
/* USER CODE BEGIN ExternalFunctions */

/* USER CODE END ExternalFunctions */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */
/**
  * Initializes the Global MSP.
  */
void HAL_MspInit(void)
{

  /* USER CODE BEGIN MspInit 0 */

  /* USER CODE END MspInit 0 */

  __HAL_RCC_PWR_CLK_ENABLE();
  HAL_PWREx_EnableVddA();
  /* System interrupt init*/

  /* USER CODE BEGIN MspInit 1 */

  /* USER CODE END MspInit 1 */
}

/**
* @brief UART MSP Initialization
* This function configures the hardware resources used in this example
* @param huart: UART handle pointer
* @retval None
*/
void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  if(huart->Instance==USART1)
  {
    HAL_USART1_MspInit(&GPIO_InitStruct, &PeriphClkInit);
  }
  else if(huart->Instance==USART3)
  {
    HAL_USART3_MspInit(&GPIO_InitStruct, &PeriphClkInit);
  }
  else if(huart->Instance==UART4)
  {
    HAL_UART4_MspInit(&GPIO_InitStruct, &PeriphClkInit);
  }
  else if(huart->Instance==UART5)
  {
    HAL_UART5_MspInit(&GPIO_InitStruct, &PeriphClkInit);
  }
  else if(huart->Instance==LPUART1)
  {
    HAL_LPUART1_MspInit(&GPIO_InitStruct, &PeriphClkInit);
  }
}

static void HAL_USART1_MspInit(GPIO_InitTypeDef *GPIO_InitStruct, RCC_PeriphCLKInitTypeDef *PeriphClkInit)
{
  /* USER CODE BEGIN USART1_MspInit 0 */

  /* USER CODE END USART1_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInit->PeriphClockSelection = RCC_PERIPHCLK_USART1;
    PeriphClkInit->Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
    if (HAL_RCCEx_PeriphCLKConfig(PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    /* Peripheral clock enable */
    __HAL_RCC_USART1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    GPIO_InitStruct->Pin = PIN_STPIN(USART1_TX_PIN)|PIN_STPIN(USART1_RX_PIN);
    GPIO_InitStruct->Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct->Pull = GPIO_NOPULL;
    GPIO_InitStruct->Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct->Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(PIN_STPORT(USART1_TX_PIN), GPIO_InitStruct);

  /* USER CODE BEGIN USART1_MspInit 1 */

  /* USER CODE END USART1_MspInit 1 */
}

static void HAL_USART3_MspInit(GPIO_InitTypeDef *GPIO_InitStruct, RCC_PeriphCLKInitTypeDef *PeriphClkInit)
{
  /* USER CODE BEGIN USART3_MspInit 0 */

  /* USER CODE END USART3_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInit->PeriphClockSelection = RCC_PERIPHCLK_USART3;
    PeriphClkInit->Usart3ClockSelection = RCC_USART3CLKSOURCE_PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    /* Peripheral clock enable */
    __HAL_RCC_USART3_CLK_ENABLE();

    __HAL_RCC_GPIOC_CLK_ENABLE();
    /**USART3 GPIO Configuration
    PC4     ------> USART3_TX
    PC5     ------> USART3_RX
    */
    GPIO_InitStruct->Pin = PIN_STPIN(USART3_TX_PIN)|PIN_STPIN(USART3_RX_PIN);
    GPIO_InitStruct->Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct->Pull = GPIO_NOPULL;
    GPIO_InitStruct->Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct->Alternate = GPIO_AF7_USART3;
    HAL_GPIO_Init(PIN_STPORT(USART3_TX_PIN), GPIO_InitStruct);

  /* USER CODE BEGIN USART3_MspInit 1 */

  /* USER CODE END USART3_MspInit 1 */
}

static void HAL_UART4_MspInit(GPIO_InitTypeDef *GPIO_InitStruct, RCC_PeriphCLKInitTypeDef *PeriphClkInit)
{
  /* USER CODE BEGIN UART4_MspInit 0 */

  /* USER CODE END UART4_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInit->PeriphClockSelection = RCC_PERIPHCLK_UART4;
    PeriphClkInit->Uart4ClockSelection = RCC_UART4CLKSOURCE_PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    /* Peripheral clock enable */
    __HAL_RCC_UART4_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**UART4 GPIO Configuration
    PA0     ------> UART4_TX
    PA1     ------> UART4_RX
    */
    GPIO_InitStruct->Pin = PIN_STPIN(UART4_TX_PIN)|PIN_STPIN(UART4_RX_PIN);
    GPIO_InitStruct->Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct->Pull = GPIO_NOPULL;
    GPIO_InitStruct->Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct->Alternate = GPIO_AF8_UART4;
    HAL_GPIO_Init(PIN_STPORT(UART4_TX_PIN), GPIO_InitStruct);

  /* USER CODE BEGIN UART4_MspInit 1 */

  /* USER CODE END UART4_MspInit 1 */
}

static void HAL_UART5_MspInit(GPIO_InitTypeDef *GPIO_InitStruct, RCC_PeriphCLKInitTypeDef *PeriphClkInit)
{
  /* USER CODE BEGIN UART5_MspInit 0 */

  /* USER CODE END UART5_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInit->PeriphClockSelection = RCC_PERIPHCLK_UART5;
    PeriphClkInit->Uart5ClockSelection = RCC_UART5CLKSOURCE_PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    /* Peripheral clock enable */
    __HAL_RCC_UART5_CLK_ENABLE();

    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    /**UART5 GPIO Configuration
    PC12     ------> UART5_TX
    PD2      ------> UART5_RX
    */
    GPIO_InitStruct->Pin = PIN_STPIN(UART5_TX_PIN);
    GPIO_InitStruct->Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct->Pull = GPIO_NOPULL;
    GPIO_InitStruct->Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct->Alternate = GPIO_AF8_UART5;
    HAL_GPIO_Init(PIN_STPORT(UART5_TX_PIN), GPIO_InitStruct);

    GPIO_InitTypeDef RX_GPIO_InitStruct = {0};
    RX_GPIO_InitStruct.Pin = PIN_STPIN(UART5_RX_PIN);
    RX_GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    RX_GPIO_InitStruct.Pull = GPIO_NOPULL;
    RX_GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    RX_GPIO_InitStruct.Alternate = GPIO_AF8_UART5;
    HAL_GPIO_Init(PIN_STPORT(UART5_RX_PIN), &RX_GPIO_InitStruct);

  /* USER CODE BEGIN UART5_MspInit 1 */

  /* USER CODE END UART5_MspInit 1 */
}

static void HAL_LPUART1_MspInit(GPIO_InitTypeDef *GPIO_InitStruct, RCC_PeriphCLKInitTypeDef *PeriphClkInit)
{
  /* USER CODE BEGIN LPUART1_MspInit 0 */

  /* USER CODE END LPUART1_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInit->PeriphClockSelection = RCC_PERIPHCLK_LPUART1;
    PeriphClkInit->Lpuart1ClockSelection = RCC_LPUART1CLKSOURCE_PCLK3;
    if (HAL_RCCEx_PeriphCLKConfig(PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    /* Peripheral clock enable */
    __HAL_RCC_LPUART1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**LPUART1 GPIO Configuration
    PA2     ------> LPUART1_TX
    PA3     ------> LPUART1_RX
    */
    GPIO_InitStruct->Pin = PIN_STPIN(LPUART1_TX_PIN)|PIN_STPIN(LPUART1_RX_PIN);
    GPIO_InitStruct->Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct->Pull = GPIO_NOPULL;
    GPIO_InitStruct->Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct->Alternate = GPIO_AF8_LPUART1;
    HAL_GPIO_Init(PIN_STPORT(LPUART1_TX_PIN), GPIO_InitStruct);

  /* USER CODE BEGIN LPUART1_MspInit 1 */

  /* USER CODE END LPUART1_MspInit 1 */
}

/**
* @brief UART MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param huart: UART handle pointer
* @retval None
*/
void HAL_UART_MspDeInit(UART_HandleTypeDef* huart)
{
  if(huart->Instance==USART1)
  {
    HAL_USART1_MspDeInit();
  }
  else if(huart->Instance==USART3)
  {
    HAL_USART3_MspDeInit();
  }
  else if(huart->Instance==UART4)
  {
    HAL_UART4_MspDeInit();
  }
  else if(huart->Instance==UART5)
  {
    HAL_UART5_MspDeInit();
  }
  else if(huart->Instance==LPUART1)
  {
    HAL_LPUART1_MspDeInit();
  }
}

static void HAL_USART1_MspDeInit(void)
{
  /* USER CODE BEGIN USART1_MspDeInit 0 */

  /* USER CODE END USART1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART1_CLK_DISABLE();

    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    HAL_GPIO_DeInit(PIN_STPORT(USART1_TX_PIN), PIN_STPIN(USART1_TX_PIN)|PIN_STPIN(USART1_RX_PIN));

  /* USER CODE BEGIN USART1_MspDeInit 1 */

  /* USER CODE END USART1_MspDeInit 1 */
}

static void HAL_USART3_MspDeInit(void)
{
  /* USER CODE BEGIN USART3_MspDeInit 0 */

  /* USER CODE END USART3_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART3_CLK_DISABLE();

    /**USART3 GPIO Configuration
    PC4     ------> USART3_TX
    PC5     ------> USART3_RX
    */
    HAL_GPIO_DeInit(PIN_STPORT(USART3_TX_PIN), PIN_STPIN(USART3_TX_PIN)|PIN_STPIN(USART3_RX_PIN));

  /* USER CODE BEGIN USART3_MspDeInit 1 */

  /* USER CODE END USART3_MspDeInit 1 */
}

static void HAL_UART4_MspDeInit(void)
{
  /* USER CODE BEGIN UART4_MspDeInit 0 */

  /* USER CODE END UART4_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_UART4_CLK_DISABLE();

    /**UART4 GPIO Configuration
    PA0     ------> UART4_TX
    PA1     ------> UART4_RX
    */
    HAL_GPIO_DeInit(PIN_STPORT(UART4_TX_PIN), PIN_STPIN(UART4_TX_PIN)|PIN_STPIN(UART4_RX_PIN));

  /* USER CODE BEGIN UART4_MspDeInit 1 */

  /* USER CODE END UART4_MspDeInit 1 */
}

static void HAL_UART5_MspDeInit(void)
{
  /* USER CODE BEGIN UART5_MspDeInit 0 */

  /* USER CODE END UART5_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_UART5_CLK_DISABLE();

    /**UART5 GPIO Configuration
    PC12     ------> UART5_TX
    PD2      ------> UART5_RX
    */
    HAL_GPIO_DeInit(PIN_STPORT(UART5_TX_PIN), PIN_STPIN(UART5_TX_PIN));
    HAL_GPIO_DeInit(PIN_STPORT(UART5_RX_PIN), PIN_STPIN(UART5_RX_PIN));

  /* USER CODE BEGIN UART5_MspDeInit 1 */

  /* USER CODE END UART5_MspDeInit 1 */
}

static void HAL_LPUART1_MspDeInit(void)
{
  /* USER CODE BEGIN LPUART1_MspDeInit 0 */

  /* USER CODE END LPUART1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_LPUART1_CLK_DISABLE();

    /**LPUART1 GPIO Configuration
    PA2     ------> LPUART1_TX
    PA3     ------> LPUART1_RX
    */
    HAL_GPIO_DeInit(PIN_STPORT(LPUART1_TX_PIN), PIN_STPIN(LPUART1_TX_PIN)|PIN_STPIN(LPUART1_RX_PIN));

  /* USER CODE BEGIN LPUART1_MspDeInit 1 */

  /* USER CODE END LPUART1_MspDeInit 1 */
}

/**
* @brief ADC MSP Initialization
* This function configures the hardware resources used in this example
* @param hadc: ADC handle pointer
* @retval None
*/
void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  if(hadc->Instance==ADC1)
  {
    HAL_ADC1_MspInit(&GPIO_InitStruct, &PeriphClkInit);
  }

}

static void HAL_ADC1_MspInit(GPIO_InitTypeDef *GPIO_InitStruct, RCC_PeriphCLKInitTypeDef *PeriphClkInit)
{
  /* USER CODE BEGIN ADC1_MspInit 0 */

  /* USER CODE END ADC1_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInit->PeriphClockSelection = RCC_PERIPHCLK_ADCDAC;
    PeriphClkInit->AdcDacClockSelection = RCC_ADCDACCLKSOURCE_HSI;
    if (HAL_RCCEx_PeriphCLKConfig(PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    /* Peripheral clock enable */
    __HAL_RCC_ADC12_CLK_ENABLE();

    __HAL_RCC_GPIOC_CLK_ENABLE();
    /**ADC1 GPIO Configuration
    PC0     ------> ADC1_IN1
    PC1     ------> ADC1_IN2
    */
    GPIO_InitStruct->Pin = PIN_STPIN(ADC1_IN1_PIN)|PIN_STPIN(ADC1_IN2_PIN);
    GPIO_InitStruct->Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct->Pull = GPIO_NOPULL;
    HAL_GPIO_Init(PIN_STPORT(ADC1_IN1_PIN), GPIO_InitStruct);

  /* USER CODE BEGIN ADC1_MspInit 1 */

  /* USER CODE END ADC1_MspInit 1 */
}

/**
* @brief ADC MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param hadc: ADC handle pointer
* @retval None
*/
void HAL_ADC_MspDeInit(ADC_HandleTypeDef* hadc)
{
  if(hadc->Instance==ADC1)
  {
    HAL_ADC1_MspDeInit();
  }

}

static void HAL_ADC1_MspDeInit(void)
{
  /* USER CODE BEGIN ADC1_MspDeInit 0 */

  /* USER CODE END ADC1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_ADC12_CLK_DISABLE();

    /**ADC1 GPIO Configuration
    PC0     ------> ADC1_IN1
    PC1     ------> ADC1_IN2
    */
    HAL_GPIO_DeInit(PIN_STPORT(ADC1_IN1_PIN), PIN_STPIN(ADC1_IN1_PIN)|PIN_STPIN(ADC1_IN2_PIN));

  /* USER CODE BEGIN ADC1_MspDeInit 1 */

  /* USER CODE END ADC1_MspDeInit 1 */
}

/**
* @brief I2C MSP Initialization
* This function configures the hardware resources used in this example
* @param hi2c: I2C handle pointer
* @retval None
*/
void HAL_I2C_MspInit(I2C_HandleTypeDef* hi2c)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  if(hi2c->Instance==I2C1)
  {
    HAL_I2C1_MspInit(&GPIO_InitStruct, &PeriphClkInit);
  }

}

static void HAL_I2C1_MspInit(GPIO_InitTypeDef *GPIO_InitStruct, RCC_PeriphCLKInitTypeDef *PeriphClkInit)
{
  /* USER CODE BEGIN I2C1_MspInit 0 */
    
  /* USER CODE END I2C1_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInit->PeriphClockSelection = RCC_PERIPHCLK_I2C1;
    PeriphClkInit->I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**I2C1 GPIO Configuration
    PB8     ------> I2C1_SCL
    PB3     ------> I2C1_SDA
    */
    GPIO_InitStruct->Pin = PIN_STPIN(I2C1_SCL_PIN)|PIN_STPIN(I2C1_SDA_PIN);
    GPIO_InitStruct->Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct->Pull = GPIO_PULLUP;
    GPIO_InitStruct->Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct->Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(PIN_STPORT(I2C1_SCL_PIN), GPIO_InitStruct);

    /* Peripheral clock enable */
    __HAL_RCC_I2C1_CLK_ENABLE();
    /* I2C1 interrupt Init */
    HAL_NVIC_SetPriority(I2C1_EV_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
    HAL_NVIC_SetPriority(I2C1_ER_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(I2C1_ER_IRQn);
  /* USER CODE BEGIN I2C1_MspInit 1 */

  /* USER CODE END I2C1_MspInit 1 */
}

/**
* @brief I2C MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param hi2c: I2C handle pointer
* @retval None
*/
void HAL_I2C_MspDeInit(I2C_HandleTypeDef* hi2c)
{
  if(hi2c->Instance==I2C1)
  {
    HAL_I2C1_MspDeInit();
  }
}

static void HAL_I2C1_MspDeInit(void)
{
  /* USER CODE BEGIN I2C1_MspDeInit 0 */

  /* USER CODE END I2C1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_I2C1_CLK_DISABLE();

    /**I2C1 GPIO Configuration
    PB8     ------> I2C1_SCL
    PB3     ------> I2C1_SDA
    */
    HAL_GPIO_DeInit(PIN_STPORT(I2C1_SCL_PIN), PIN_STPIN(I2C1_SCL_PIN)|PIN_STPIN(I2C1_SDA_PIN));

    /* I2C1 interrupt DeInit */
    HAL_NVIC_DisableIRQ(I2C1_EV_IRQn);
    HAL_NVIC_DisableIRQ(I2C1_ER_IRQn);
  /* USER CODE BEGIN I2C1_MspDeInit 1 */

  /* USER CODE END I2C1_MspDeInit 1 */
}

/**
* @brief RTC MSP Initialization
* This function configures the hardware resources used in this example
* @param hrtc: RTC handle pointer
* @retval None
*/
void HAL_RTC_MspInit(RTC_HandleTypeDef* hrtc)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  if(hrtc->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspInit 0 */

  /* USER CODE END RTC_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
#ifdef BSP_RTC_USING_LSE
    PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
#else
    PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
#endif
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    /* Peripheral clock enable */
    __HAL_RCC_RTC_ENABLE();
    __HAL_RCC_RTCAPB_CLK_ENABLE();
    __HAL_RCC_RTCAPB_CLKAM_ENABLE();
  /* USER CODE BEGIN RTC_MspInit 1 */

  /* USER CODE END RTC_MspInit 1 */

  }

}

/**
* @brief RTC MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param hrtc: RTC handle pointer
* @retval None
*/
void HAL_RTC_MspDeInit(RTC_HandleTypeDef* hrtc)
{
  if(hrtc->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspDeInit 0 */

  /* USER CODE END RTC_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_RTC_DISABLE();
    __HAL_RCC_RTCAPB_CLK_DISABLE();
    __HAL_RCC_RTCAPB_CLKAM_DISABLE();
  /* USER CODE BEGIN RTC_MspDeInit 1 */

  /* USER CODE END RTC_MspDeInit 1 */
  }

}

void HAL_SPI_MspInit(SPI_HandleTypeDef* spiHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  if(spiHandle->Instance==SPI1)
  {
  /* USER CODE BEGIN SPI1_MspInit 0 */

  /* USER CODE END SPI1_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_SPI1;
    PeriphClkInit.Spi1ClockSelection = RCC_SPI1CLKSOURCE_SYSCLK;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    /* SPI1 clock enable */
    __HAL_RCC_SPI1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**SPI1 GPIO Configuration
    PA5     ------> SPI1_SCK
    PA6     ------> SPI1_MISO
    PA7     ------> SPI1_MOSI
    */
    GPIO_InitStruct.Pin = PIN_STPIN(SPI1_SCK_PIN)|PIN_STPIN(SPI1_MISO_PIN)|PIN_STPIN(SPI1_MOSI_PIN);
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init(PIN_STPORT(SPI1_SCK_PIN), &GPIO_InitStruct);

    // /* SPI1 interrupt Init */
    // HAL_NVIC_SetPriority(SPI1_IRQn, 5, 0);
    // HAL_NVIC_EnableIRQ(SPI1_IRQn);
  /* USER CODE BEGIN SPI1_MspInit 1 */

  /* USER CODE END SPI1_MspInit 1 */
  }
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef* spiHandle)
{

  if(spiHandle->Instance==SPI1)
  {
  /* USER CODE BEGIN SPI1_MspDeInit 0 */

  /* USER CODE END SPI1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_SPI1_CLK_DISABLE();

    /**SPI1 GPIO Configuration
    PA5     ------> SPI1_SCK
    PA6     ------> SPI1_MISO
    PA7     ------> SPI1_MOSI
    */
    HAL_GPIO_DeInit(PIN_STPORT(SPI1_SCK_PIN), PIN_STPIN(SPI1_SCK_PIN)|PIN_STPIN(SPI1_MISO_PIN)|PIN_STPIN(SPI1_MOSI_PIN));

    // /* SPI1 interrupt Deinit */
    // HAL_NVIC_DisableIRQ(SPI1_IRQn);
  /* USER CODE BEGIN SPI1_MspDeInit 1 */

  /* USER CODE END SPI1_MspDeInit 1 */
  }
}

/**
* @brief CRC MSP Initialization
* This function configures the hardware resources used in this example
* @param hcrc: CRC handle pointer
* @retval None
*/
void HAL_CRC_MspInit(CRC_HandleTypeDef* hcrc)
{
  if(hcrc->Instance==CRC)
  {
  /* USER CODE BEGIN CRC_MspInit 0 */

  /* USER CODE END CRC_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_CRC_CLK_ENABLE();
  /* USER CODE BEGIN CRC_MspInit 1 */

  /* USER CODE END CRC_MspInit 1 */
  }

}

/**
* @brief CRC MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param hcrc: CRC handle pointer
* @retval None
*/
void HAL_CRC_MspDeInit(CRC_HandleTypeDef* hcrc)
{
  if(hcrc->Instance==CRC)
  {
  /* USER CODE BEGIN CRC_MspDeInit 0 */

  /* USER CODE END CRC_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_CRC_CLK_DISABLE();
  /* USER CODE BEGIN CRC_MspDeInit 1 */

  /* USER CODE END CRC_MspDeInit 1 */
  }

}

/**
* @brief WWDG MSP Initialization
* This function configures the hardware resources used in this example
* @param hwwdg: WWDG handle pointer
* @retval None
*/
void HAL_WWDG_MspInit(WWDG_HandleTypeDef* hwwdg)
{
  if(hwwdg->Instance==WWDG)
  {
  /* USER CODE BEGIN WWDG_MspInit 0 */

  /* USER CODE END WWDG_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_WWDG_CLK_ENABLE();
  /* USER CODE BEGIN WWDG_MspInit 1 */

  /* USER CODE END WWDG_MspInit 1 */
  }

}

/**
* @brief HASH MSP Initialization
* This function configures the hardware resources used in this example
* @param hhash: HASH handle pointer
* @retval None
*/
void HAL_HASH_MspInit(HASH_HandleTypeDef* hhash)
{
  /* USER CODE BEGIN HASH_MspInit 0 */

  /* USER CODE END HASH_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_HASH_CLK_ENABLE();
  /* USER CODE BEGIN HASH_MspInit 1 */

  /* USER CODE END HASH_MspInit 1 */

}

/**
* @brief HASH MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param hhash: HASH handle pointer
* @retval None
*/
void HAL_HASH_MspDeInit(HASH_HandleTypeDef* hhash)
{
  /* USER CODE BEGIN HASH_MspDeInit 0 */

  /* USER CODE END HASH_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_HASH_CLK_DISABLE();
  /* USER CODE BEGIN HASH_MspDeInit 1 */

  /* USER CODE END HASH_MspDeInit 1 */

}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
