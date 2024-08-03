/*
 * @FilePath: lpm.c
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-07-31 13:45:56
 * @copyright : Copyright (c) 2024
 */
#include "lpm.h"

#define DBG_SECTION_NAME "LPM"
#define DBG_LEVEL DBG_LOG
#include <rtdbg.h>

#define SEN_PWR_WKUP7  GET_PIN(E, 8)
// #define BUTTON_PIN  GET_PIN(C, 13)
RTC_HandleTypeDef hrtc;
#define RTC_ASYNCH_PREDIV    0x7F
#define RTC_SYNCH_PREDIV     0x0F9
static void MX_RTC_Init(void);

void shut_down(void)
{
    /* Sensor wakeup pin irq enable. */
    rt_pin_mode(SEN_PWR_WKUP7, PIN_MODE_INPUT);
    rt_pin_irq_enable(SEN_PWR_WKUP7, PIN_IRQ_ENABLE);
    HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN7_HIGH_2);

    // // /* Enable WakeUp Pin PWR_WAKEUP_PIN2 connected to PC.13 */
    // rt_pin_mode(BUTTON_PIN, PIN_MODE_INPUT);
    // rt_pin_irq_enable(BUTTON_PIN, PIN_IRQ_ENABLE);
    // HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN2_HIGH_1);

    LOG_D("Start Shut Down.");

    /* Clear all related wakeup flags*/
    __HAL_PWR_CLEAR_FLAG(PWR_WAKEUP_FLAG2);

    /* Enter the Shutdown mode */
    HAL_PWREx_EnterSHUTDOWNMode();
}

static void alarm_callback(rt_alarm_t alarm, time_t timestamp)
{
    LOG_D("user alarm callback function.\n");
}

rt_err_t set_rtc_wakeup(time_t sleep_time)
{
    rt_device_t dev = rt_device_find("rtc");
    if (!dev)
    {
        LOG_E("find device rtc failed.");
        return RT_ERROR;
    }
    LOG_D("find device rtc success.");
    if (rt_device_open(dev, 0) != RT_EOK)
    {
        LOG_E("open device rtc failed.");
        return RT_ERROR;
    }
    LOG_D("open device rtc success.");
    if (set_date(2024, 8, 3) != RT_EOK)
    {
        LOG_E("set_date(2024, 8, 3) failed.");
        return RT_ERROR;
    }
    LOG_D("set_date(2024, 8, 3) success.");
    if (set_time(0, 0, 0) != RT_EOK)
    {
        LOG_E("set_time(0, 0, 0) failed.");
        return RT_ERROR;
    }
    LOG_D("set_time(0, 0, 0) success.");
    struct rt_alarm_setup setup;
    struct rt_alarm *alarm = RT_NULL;
    static time_t now;
    struct tm p_tm;

    now = time(NULL);
    LOG_D("now %s", ctime(&now));
    now += sleep_time;
    LOG_D("wakeup %s", ctime(&now));
    gmtime_r(&now, &p_tm);

    setup.flag = RT_ALARM_ONESHOT;            
    setup.wktime.tm_year = p_tm.tm_year;
    setup.wktime.tm_mon = p_tm.tm_mon;
    setup.wktime.tm_mday = p_tm.tm_mday;
    setup.wktime.tm_wday = p_tm.tm_wday;
    setup.wktime.tm_hour = p_tm.tm_hour;
    setup.wktime.tm_min = p_tm.tm_min;
    setup.wktime.tm_sec = p_tm.tm_sec;   

    alarm = rt_alarm_create(alarm_callback, &setup);    
    if(alarm != RT_NULL)
    {
        LOG_D("rt_alarm_create success.");
        if (rt_alarm_start(alarm) != RT_EOK)
        {
            LOG_E("rt_alarm_start failed.");
        }
        LOG_D("rt_alarm_start success.");
    }
}

static void test_rtc_wakeup(int argc, char **argv)
{
    set_rtc_wakeup(10);
    // shut_down();
    // MX_RTC_Init();
}
MSH_CMD_EXPORT(test_rtc_wakeup, test rtc wakeup);


/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN Init */
  /* Reset the RTC peripheral and the RTC clock source selection */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_BACKUPRESET_FORCE();
  __HAL_RCC_BACKUPRESET_RELEASE();

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_PrivilegeStateTypeDef privilegeState = {0};
  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};
  RTC_AlarmTypeDef sAlarm = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = RTC_ASYNCH_PREDIV;
  hrtc.Init.SynchPrediv = RTC_SYNCH_PREDIV;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  hrtc.Init.OutPutPullUp = RTC_OUTPUT_PULLUP_NONE;
  hrtc.Init.BinMode = RTC_BINARY_NONE;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }
  LOG_D("HAL_RTC_Init success.");
  privilegeState.rtcPrivilegeFull = RTC_PRIVILEGE_FULL_NO;
  privilegeState.backupRegisterPrivZone = RTC_PRIVILEGE_BKUP_ZONE_NONE;
  privilegeState.backupRegisterStartZone2 = RTC_BKP_DR0;
  privilegeState.backupRegisterStartZone3 = RTC_BKP_DR0;
  if (HAL_RTCEx_PrivilegeModeSet(&hrtc, &privilegeState) != HAL_OK)
  {
    Error_Handler();
  }
  LOG_D("HAL_RTCEx_PrivilegeModeSet success.");

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0x2;
  sTime.Minutes = 0x20;
  sTime.Seconds = 0x0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  LOG_D("HAL_RTC_SetTime success.");
  sDate.WeekDay = RTC_WEEKDAY_TUESDAY;
  sDate.Month = RTC_MONTH_OCTOBER;
  sDate.Date = 0x18;
  sDate.Year = 0x22;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  LOG_D("HAL_RTC_SetDate success.");

  /** Enable the Alarm A
  */
  sAlarm.AlarmTime.Hours = 0x2;
  sAlarm.AlarmTime.Minutes = 0x20;
  sAlarm.AlarmTime.Seconds = 0x30;
  sAlarm.AlarmTime.SubSeconds = 0x56;
  sAlarm.AlarmMask = RTC_ALARMMASK_SECONDS;
  sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
  sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_WEEKDAY;
  sAlarm.AlarmDateWeekDay = RTC_WEEKDAY_TUESDAY;
  sAlarm.Alarm = RTC_ALARM_A;
  if (HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  LOG_D("HAL_RTC_SetAlarm_IT success.");
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}
