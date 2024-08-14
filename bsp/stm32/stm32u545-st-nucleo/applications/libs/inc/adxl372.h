/*
 * @FilePath: adxl372.h
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-08-05 19:11:03
 * @copyright : Copyright (c) 2024
 */
#ifndef __ADXL372_H__
#define __ADXL372_H__

#include "rtthread.h"
#include "rtdevice.h"
#include "board.h"
#include "board_pin.h"

static const rt_uint8_t ADI_ADXL372_ADI_DEVID         = 0x00;   // Analog Devices, Inc., accelerometer ID 
static const rt_uint8_t ADI_ADXL372_MST_DEVID         = 0x01;   // Analog Devices MEMS device ID
static const rt_uint8_t ADI_ADXL372_DEVID             = 0x02;   // Device ID
static const rt_uint8_t ADI_ADXL372_REVID             = 0x03;   // product revision ID
static const rt_uint8_t ADI_ADXL372_STATUS_1          = 0x04;   // Status register 1
static const rt_uint8_t ADI_ADXL372_STATUS_2          = 0x05;   // Status register 2
static const rt_uint8_t ADI_ADXL372_FIFO_ENTRIES_2    = 0x06;   // Valid data samples in the FIFO
static const rt_uint8_t ADI_ADXL372_FIFO_ENTRIES_1    = 0x07;   // Valid data samples in the FIFO
static const rt_uint8_t ADI_ADXL372_X_DATA_H          = 0x08;   // X-axis acceleration data [11:4]
static const rt_uint8_t ADI_ADXL372_X_DATA_L          = 0x09;   // X-axis acceleration data [3:0] | dummy LSBs
static const rt_uint8_t ADI_ADXL372_Y_DATA_H          = 0x0A;   // Y-axis acceleration data [11:4]
static const rt_uint8_t ADI_ADXL372_Y_DATA_L          = 0x0B;   // Y-axis acceleration data [3:0] | dummy LSBs
static const rt_uint8_t ADI_ADXL372_Z_DATA_H          = 0x0C;   // Z-axis acceleration data [11:4]
static const rt_uint8_t ADI_ADXL372_Z_DATA_L          = 0x0D;   // Z-axis acceleration data [3:0] | dummy LSBs
static const rt_uint8_t ADI_ADXL372_X_MAXPEAK_H       = 0x15;   // X-axis MaxPeak acceleration data [15:8]
static const rt_uint8_t ADI_ADXL372_X_MAXPEAK_L       = 0x16;   // X-axis MaxPeak acceleration data [7:0]
static const rt_uint8_t ADI_ADXL372_Y_MAXPEAK_H       = 0x17;   // X-axis MaxPeak acceleration data [15:8]
static const rt_uint8_t ADI_ADXL372_Y_MAXPEAK_L       = 0x18;   // X-axis MaxPeak acceleration data [7:0]
static const rt_uint8_t ADI_ADXL372_Z_MAXPEAK_H       = 0x19;   // X-axis MaxPeak acceleration data [15:8]
static const rt_uint8_t ADI_ADXL372_Z_MAXPEAK_L       = 0x1A;   // X-axis MaxPeak acceleration data [7:0]
static const rt_uint8_t ADI_ADXL372_OFFSET_X          = 0x20;   // X axis offset
static const rt_uint8_t ADI_ADXL372_OFFSET_Y          = 0x21;   // Y axis offset
static const rt_uint8_t ADI_ADXL372_OFFSET_Z          = 0x22;   // Z axis offset
static const rt_uint8_t ADI_ADXL372_X_THRESH_ACT_H    = 0x23;   // X axis Activity Threshold [15:8]
static const rt_uint8_t ADI_ADXL372_X_THRESH_ACT_L    = 0x24;   // X axis Activity Threshold [7:0]
static const rt_uint8_t ADI_ADXL372_Y_THRESH_ACT_H    = 0x25;   // Y axis Activity Threshold [15:8]
static const rt_uint8_t ADI_ADXL372_Y_THRESH_ACT_L    = 0x26;   // Y axis Activity Threshold [7:0]
static const rt_uint8_t ADI_ADXL372_Z_THRESH_ACT_H    = 0x27;   // Z axis Activity Threshold [15:8]
static const rt_uint8_t ADI_ADXL372_Z_THRESH_ACT_L    = 0x28;   // Z axis Activity Threshold [7:0]
static const rt_uint8_t ADI_ADXL372_TIME_ACT          = 0x29;   // Activity Time
static const rt_uint8_t ADI_ADXL372_X_THRESH_INACT_H  = 0x2A;   // X axis Inactivity Threshold [15:8]
static const rt_uint8_t ADI_ADXL372_X_THRESH_INACT_L  = 0x2B;   // X axis Inactivity Threshold [7:0]
static const rt_uint8_t ADI_ADXL372_Y_THRESH_INACT_H  = 0x2C;   // Y axis Inactivity Threshold [15:8]
static const rt_uint8_t ADI_ADXL372_Y_THRESH_INACT_L  = 0x2D;   // Y axis Inactivity Threshold [7:0]
static const rt_uint8_t ADI_ADXL372_Z_THRESH_INACT_H  = 0x2E;   // Z axis Inactivity Threshold [15:8]
static const rt_uint8_t ADI_ADXL372_Z_THRESH_INACT_L  = 0x2F;   // Z axis Inactivity Threshold [7:0]
static const rt_uint8_t ADI_ADXL372_TIME_INACT_H      = 0x30;   // Inactivity Time [15:8]
static const rt_uint8_t ADI_ADXL372_TIME_INACT_L      = 0x31;   // Inactivity Time [7:0]
static const rt_uint8_t ADI_ADXL372_X_THRESH_ACT2_H   = 0x32;   // X axis Activity2 Threshold [15:8]
static const rt_uint8_t ADI_ADXL372_X_THRESH_ACT2_L   = 0x33;   // X axis Activity2 Threshold [7:0]
static const rt_uint8_t ADI_ADXL372_Y_THRESH_ACT2_H   = 0x34;   // Y axis Activity2 Threshold [15:8]
static const rt_uint8_t ADI_ADXL372_Y_THRESH_ACT2_L   = 0x35;   // Y axis Activity2 Threshold [7:0]
static const rt_uint8_t ADI_ADXL372_Z_THRESH_ACT2_H   = 0x36;   // Z axis Activity2 Threshold [15:8]
static const rt_uint8_t ADI_ADXL372_Z_THRESH_ACT2_L   = 0x37;   // Z axis Activity2 Threshold [7:0]
static const rt_uint8_t ADI_ADXL372_HPF               = 0x38;   // High Pass Filter
static const rt_uint8_t ADI_ADXL372_FIFO_SAMPLES      = 0x39;   // FIFO Samples
static const rt_uint8_t ADI_ADXL372_FIFO_CTL          = 0x3A;   // FIFO Control
static const rt_uint8_t ADI_ADXL372_INT1_MAP          = 0x3B;   // Interrupt 1 mapping control
static const rt_uint8_t ADI_ADXL372_INT2_MAP          = 0x3C;   // Interrupt 2 mapping control
static const rt_uint8_t ADI_ADXL372_TIMING            = 0x3D;   // Timing
static const rt_uint8_t ADI_ADXL372_MEASURE           = 0x3E;   // Measure
static const rt_uint8_t ADI_ADXL372_POWER_CTL         = 0x3F;   // Power control
static const rt_uint8_t ADI_ADXL372_SELF_TEST         = 0x40;   // Self Test
static const rt_uint8_t ADI_ADXL372_SRESET            = 0x41;   // Reset
static const rt_uint8_t ADI_ADXL372_FIFO_DATA         = 0x42;   // FIFO Data

static const rt_uint8_t ADI_ADXL372_ADI_DEVID_VAL     = 0xAD;   // Analog Devices, Inc., accelerometer ID
static const rt_uint8_t ADI_ADXL372_MST_DEVID_VAL     = 0x1D;   // Analog Devices MEMS device ID
static const rt_uint8_t ADI_ADXL372_DEVID_VAL         = 0xFA;   // Device ID
static const rt_uint8_t ADI_ADXL372_REVID_VAL         = 0x02;   // product revision ID


static const rt_uint8_t MEASURE_AUTOSLEEP_MASK        = 0xBF;
static const rt_uint8_t MEASURE_BANDWIDTH_MASK        = 0xF8;
static const rt_uint8_t MEASURE_ACTPROC_MASK          = 0xCF;
static const rt_uint8_t TIMING_ODR_MASK               = 0x1F;
static const rt_uint8_t TIMING_WUR_MASK               = 0xE3;
static const rt_uint8_t PWRCTRL_OPMODE_MASK           = 0xFC;
static const rt_uint8_t PWRCTRL_INSTAON_THRESH_MASK   = 0xDF;
static const rt_uint8_t PWRCTRL_FILTER_SETTLE_MASK    = 0xEF;

// Position of flags in their respective registers
static const rt_uint8_t MEASURE_AUTOSLEEP_POS         = 6;
static const rt_uint8_t MEASURE_ACTPROC_POS           = 4;
static const rt_uint8_t TIMING_ODR_POS                = 5;
static const rt_uint8_t TIMING_WUR_POS                = 2;
static const rt_uint8_t INSTAON_THRESH_POS            = 5;
static const rt_uint8_t FIFO_CRL_SAMP8_POS            = 0;
static const rt_uint8_t FIFO_CRL_MODE_POS             = 1;
static const rt_uint8_t FIFO_CRL_FORMAT_POS           = 3;
static const rt_uint8_t PWRCTRL_FILTER_SETTLE_POS     = 4;

static const rt_uint8_t DATA_RDY      = 1;
static const rt_uint8_t FIFO_RDY      = 2;
static const rt_uint8_t FIFO_FULL     = 4;
static const rt_uint8_t FIFO_OVR      = 8;

/*Acceleremoter configuration*/
static const rt_uint8_t ACT_VALUE       =  30;     // Activity threshold value
static const rt_uint8_t INACT_VALUE     =  30;     // Inactivity threshold value
static const rt_uint8_t ACT_TIMER       =  1;      // Activity timer value in multiples of 3.3ms
static const rt_uint8_t INACT_TIMER     =  1;      // Inactivity timer value in multiples of 26ms
static const rt_uint8_t ADXL_INT1_PIN   = 7;
static const rt_uint8_t ADXL_INT2_PIN   = 5;
static const rt_uint8_t ADXL_SPI_RNW    = 1;

static const float ADXL372_SCALEG = 0.1; // g per lsb

struct adxl372_xyz
{
    float x;
    float y;
    float z;
};
typedef struct adxl372_xyz *adxl372_xyz_t;

extern rt_err_t rt_hw_spi_device_attach(const char *bus_name, const char *device_name, rt_base_t cs_pin);

__weak void adxl372_inact_event_handler(void);

void adxl372_recv_inact_event(void *parameter);
rt_err_t adxl372_recv_inact_event_thd_start(void);
rt_err_t adxl372_recv_inact_event_thd_stop(void);

void adxl372_inactive_irq_callback(void *args);
rt_err_t adxl372_int1_pin_irq_enable(void);
rt_err_t adxl372_int1_pin_irq_disable(void);
rt_err_t rt_hw_spi_adxl372_init(void);
rt_err_t adxl372_init(rt_uint16_t inact_ms, rt_uint16_t inact_threshold);
rt_err_t adxl732_read(rt_uint8_t reg, rt_uint8_t *data, rt_uint16_t size);
rt_err_t adxl732_write(rt_uint8_t reg, rt_uint8_t *data, rt_uint16_t size);

rt_err_t adxl372_query_dev_info(void);
rt_err_t adxl372_query_xyz(adxl372_xyz_t xyz);
rt_err_t adxl372_set_measure(rt_uint8_t *val);
rt_err_t adxl372_set_power_ctl(rt_uint8_t *val);
rt_err_t adxl372_set_odr(rt_uint8_t *val);
rt_err_t adxl372_set_time_inact(rt_uint16_t milliseconds);
rt_err_t adxl372_set_thresh_inact(rt_uint16_t threshold);
rt_err_t adxl372_set_int1_map(rt_uint8_t *val);
rt_err_t adxl372_set_hpf(rt_uint8_t *val);
rt_err_t adxl372_enable_inactive_irq(rt_uint16_t milliseconds, rt_uint16_t threshold);
rt_err_t adxl372_reset(void);
rt_err_t adxl372_set_standby(void);

static void test_adxl372(int argc, char **argv);

#endif  // __ADXL372_H__
