#ifndef __CONTROL_H__
#define __CONTROL_H__

#include <rtthread.h>
#include "lpm.h"
#include "nbiot.h"
#include "at_client_ssl.h"
#include "board.h"

enum AntennaType {MAIN_ANT=0, REMPTE_ANT};
enum ModuleType {NBIOT_MODULE=0, CAT1_MODULE};
enum SimCard {SIM1=0, SIM2};

int board_pins_init(void);

rt_err_t sensor_pwron_pin_enable(rt_uint8_t mode);

rt_err_t antenna_active();

rt_err_t antenna_deactive();

rt_err_t antenna_type_select(enum AntennaType antenna_type);

rt_err_t antenna_switch_to_module(enum ModuleType switch_to);

void antenna_init(enum ModuleType switch_to, enum AntennaType antenna_type);

rt_err_t antenna_deinit();

rt_err_t sim_card_select(enum SimCard sim);

rt_err_t sim_card_switch_to_module(enum ModuleType switch_to);

void sim_init(enum ModuleType switch_to, enum SimCard sim);

void sim_deinit();

void nbiot_init();

void nbiot_deinit();

rt_err_t cat1_power_ctrl(int state);

rt_err_t cat1_init();

void cat1_deinit();

rt_err_t antenna_type_switch();

enum AntennaType get_current_antenna_no();

void debug_led1_pin_init(void);
rt_err_t debug_led1_pin_enable(rt_uint8_t mode);

void save_imei_to_file(const char *imei);
void read_imei_from_file(char *output, int read_length);

#endif