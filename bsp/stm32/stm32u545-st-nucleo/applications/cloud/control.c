#include <rtthread.h>
#include "board_pin.h"
#include "lpm.h"
#include "nbiot.h"
#include "control.h"

#define DBG_TAG "control"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

rt_err_t antenna_active()
{
    rt_err_t result = antenna_active_pin_enable(PIN_HIGH);
    LOG_D("antenna_active result: %d", result);
    return result;
}

rt_err_t antenna_deactive()
{
    rt_err_t result = antenna_active_pin_enable(PIN_LOW);
    LOG_D("antenna_deactive result: %d", result);
    return result;
}

rt_err_t antenna_type_select(enum AntennaType antenna_type)
{
    // Selects which antenna is used. Low = antenna on the main board. High = antenna on rempte board
    rt_err_t result = intn_ext_ant_pin_enable(antenna_type == MAIN_ANT ? PIN_LOW : PIN_HIGH);
    LOG_D("antenna_type_select \"%s\", result: %d", antenna_type == MAIN_ANT ? "main" : "rempte", result);
    return result;
}

rt_err_t antenna_switch_to_module(enum ModuleType switch_to)
{
    // Selects source of RF to antennas - Low for NB-IoT (BC660K), High for Cat1 (EG916)
    rt_err_t result = nb_cat1_rf_pin_enable(switch_to == NBIOT_MODULE ? PIN_LOW : PIN_HIGH);
    LOG_D("antenna_switch_to_module \"%s\", result: %d", switch_to == NBIOT_MODULE ? "NBIot" : "Cat1", result);
    return result;
}

void antenna_init(enum ModuleType switch_to, enum AntennaType antenna_type) 
{
	antenna_active();
	antenna_type_select(antenna_type);
	antenna_switch_to_module(switch_to);
}

rt_err_t antenna_deinit() 
{
	return antenna_deactive();
}

rt_err_t sim_card_select(enum SimCard sim)
{
    // Connects SIM1 (low) or SIM2 (high) to the modem (CAT1bis if CAT1 power on, else EG915)
    rt_err_t result = sim_select_pin_enable(sim == SIM1 ? PIN_LOW : PIN_HIGH);
    LOG_D("sim_card_select \"%s\", result: %d", sim == SIM1 ? "SIM1" : "SIM2", result);
    return result;
}

rt_err_t sim_card_switch_to_module(enum ModuleType switch_to)
{
    // low for cat1, high for nb
    rt_err_t result = nbiot_boot_pin_enable(switch_to == NBIOT_MODULE ? PIN_HIGH : PIN_LOW);
    LOG_D("sim_card_switch_to_module \"%s\", result: %d", switch_to == NBIOT_MODULE ? "NBIot" : "Cat1", result);
    return result;
}

void sim_init(enum ModuleType switch_to, enum SimCard sim)
{
    sim_card_select(sim);
    sim_card_switch_to_module(switch_to);
}

void sim_deinit()
{
    // do nothing
}

void nbiot_init()
{
    nbiot_at_client_init();
    antenna_init(NBIOT_MODULE, MAIN_ANT);
    sim_init(NBIOT_MODULE, SIM1);
    nbiot_power_on();
    nbiot_disable_sleep_mode();
}

void nbiot_deinit()
{
    nbiot_enable_sleep_mode();
    nbiot_power_off();
    sim_deinit();
    antenna_deinit();
}

rt_err_t cat1_power_ctrl(int state)
{
    rt_err_t result = RT_EOK;

    if (state) {
        if (cat1_check_state() != RT_EOK) {
            LOG_D("cat1_check_state != RT_EOK");
            cat1_power_on();

            // wait rdy
            result = cat1_wait_rdy();
            if (result != RT_EOK) {
                LOG_D("can not cat1_wait_rdy");
                return result;
            }
            LOG_D("recv rdy");

            cat1_set_cfun_mode(0);
            LOG_D("cat1_set_cfun_mode 0");
            rt_thread_mdelay(200);
            cat1_set_cfun_mode(1);
            LOG_D("cat1_set_cfun_mode 1");
        }
        else {
             LOG_D("cat1_check_state == RT_EOK");
        }
    }
    else {
        // AT+QPOWD
        LOG_D("cat1 execute AT+QPOWD");
        cat1_qpowd();
    }

    return result;
}

rt_err_t cat1_init()
{
    rt_err_t result = RT_EOK;

    at_ssl_client_init();
    antenna_init(CAT1_MODULE, MAIN_ANT);
    sim_init(CAT1_MODULE, SIM2);

    result = cat1_power_ctrl(1);
    if (result != RT_EOK) {
        LOG_D("cat1 power on failed");
        antenna_deinit();
        sim_deinit();
        return RT_ERROR;
    }

    LOG_D("cat1 init success");
    return result;
}

void cat1_deinit()
{
    cat1_power_ctrl(0);
    sim_deinit();
    antenna_deinit();
}

// for test
