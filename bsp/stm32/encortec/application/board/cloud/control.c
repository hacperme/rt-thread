#include <rtthread.h>
#include "board.h"
#include "lpm.h"
#include "nbiot.h"
#include "control.h"

#include "logging.h"
// #define DBG_TAG "control"
// #define DBG_LVL DBG_LOG
// #include <rtdbg.h>

void sensor_pwron_pin_init(void)
{
    rt_pin_mode(SENSOR_PWRON_PIN, PIN_MODE_OUTPUT);
}

rt_err_t sensor_pwron_pin_enable(rt_uint8_t mode)
{
    rt_pin_write(SENSOR_PWRON_PIN, mode);
    return rt_pin_read(SENSOR_PWRON_PIN) == mode ? RT_EOK : RT_ERROR;
}

void antenna_active_pin_init(void)
{
    rt_pin_mode(ANTENNA_ACTIVE_PIN, PIN_MODE_OUTPUT);
}

rt_err_t antenna_active_pin_enable(rt_uint8_t mode)
{
    rt_pin_write(ANTENNA_ACTIVE_PIN, mode);
    return rt_pin_read(ANTENNA_ACTIVE_PIN) == mode ? RT_EOK : RT_ERROR;
}

void intn_ext_ant_pin_init(void)
{
    rt_pin_mode(INTN_EXT_ANT_PIN, PIN_MODE_OUTPUT);
}

rt_err_t intn_ext_ant_pin_enable(rt_uint8_t mode)
{
    rt_pin_write(INTN_EXT_ANT_PIN, mode);
    return rt_pin_read(INTN_EXT_ANT_PIN) == mode ? RT_EOK : RT_ERROR;
}

void nb_cat1_rf_pin_init(void)
{
    rt_pin_mode(NB_CAT1_RF_PIN, PIN_MODE_OUTPUT);
}

rt_err_t nb_cat1_rf_pin_enable(rt_uint8_t mode)
{
    rt_pin_write(NB_CAT1_RF_PIN, mode);
    return rt_pin_read(NB_CAT1_RF_PIN) == mode ? RT_EOK : RT_ERROR;
}

void sim_select_pin_init(void)
{
    rt_pin_mode(SIM_SELECT_PIN, PIN_MODE_OUTPUT);
}

rt_err_t sim_select_pin_enable(rt_uint8_t mode)
{
    rt_pin_write(SIM_SELECT_PIN, mode);
    return rt_pin_read(SIM_SELECT_PIN) == mode ? RT_EOK : RT_ERROR;
}

void nbiot_boot_pin_init(void)
{
    rt_pin_mode(NBIOT_BOOT_PIN, PIN_MODE_OUTPUT);
}

rt_err_t nbiot_boot_pin_enable(rt_uint8_t mode)
{
    rt_pin_write(NBIOT_BOOT_PIN, mode);
    return rt_pin_read(NBIOT_BOOT_PIN) == mode ? RT_EOK : RT_ERROR;
}

void flash_pwron_pin_init(void)
{
    rt_pin_mode(FLASH_PWRON_PIN, PIN_MODE_OUTPUT);
}

rt_err_t flash_pwron_pin_enable(rt_uint8_t mode)
{
    rt_pin_write(FLASH_PWRON_PIN, mode);
    return rt_pin_read(FLASH_PWRON_PIN) == mode ? RT_EOK : RT_ERROR;
}


int board_pins_init(void)
{
    rt_err_t res = RT_EOK;

    sensor_pwron_pin_init();
    intn_ext_ant_pin_init();
    antenna_active_pin_init();
    nb_cat1_rf_pin_init();
    sim_select_pin_init();
    nbiot_boot_pin_init();
    flash_pwron_pin_init();

    return res;
}

rt_err_t antenna_active()
{
    rt_err_t result = antenna_active_pin_enable(PIN_HIGH);
    log_debug("antenna_active result: %d", result);
    return result;
}

rt_err_t antenna_deactive()
{
    rt_err_t result = antenna_active_pin_enable(PIN_LOW);
    log_debug("antenna_deactive result: %d", result);
    return result;
}

rt_err_t antenna_type_select(enum AntennaType antenna_type)
{
    // Selects which antenna is used. Low = antenna on the main board. High = antenna on rempte board
    rt_err_t result = intn_ext_ant_pin_enable(antenna_type == MAIN_ANT ? PIN_LOW : PIN_HIGH);
    log_debug("antenna_type_select \"%s\", result: %d", antenna_type == MAIN_ANT ? "main" : "rempte", result);
    return result;
}

enum AntennaType current_antenna = MAIN_ANT;
rt_err_t antenna_type_switch()
{
    enum AntennaType temp = current_antenna == MAIN_ANT ? REMPTE_ANT : MAIN_ANT;
    antenna_type_select(temp);
    current_antenna = temp;
}

enum AntennaType get_current_antenna_no()
{
    log_debug("current antenna no: %d\n", current_antenna);
    return current_antenna == MAIN_ANT ? 0 : 1;
}

rt_err_t antenna_switch_to_module(enum ModuleType switch_to)
{
    // Selects source of RF to antennas - Low for NB-IoT (BC660K), High for Cat1 (EG916)
    rt_err_t result = nb_cat1_rf_pin_enable(switch_to == NBIOT_MODULE ? PIN_LOW : PIN_HIGH);
    log_debug("antenna_switch_to_module \"%s\", result: %d", switch_to == NBIOT_MODULE ? "NBIot" : "Cat1", result);
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
    log_debug("sim_card_select \"%s\", result: %d", sim == SIM1 ? "SIM1" : "SIM2", result);
    return result;
}

rt_err_t sim_card_switch_to_module(enum ModuleType switch_to)
{
    // low for cat1, high for nb
    rt_err_t result = nbiot_boot_pin_enable(switch_to == NBIOT_MODULE ? PIN_HIGH : PIN_LOW);
    log_debug("sim_card_switch_to_module \"%s\", result: %d", switch_to == NBIOT_MODULE ? "NBIot" : "Cat1", result);
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

    antenna_active();
    antenna_type_select(current_antenna);
    antenna_switch_to_module(NBIOT_MODULE);

    sim_init(NBIOT_MODULE, SIM1);
    nbiot_power_on();
    nbiot_disable_sleep_mode();
}

void nbiot_deinit()
{
    nbiot_enable_sleep_mode();
    nbiot_power_off();
    sim_deinit();

    antenna_switch_to_module(NBIOT_MODULE);
    antenna_type_select(current_antenna);
    // antenna_deactive();
}

rt_err_t cat1_power_ctrl(int state)
{
    rt_err_t result = RT_EOK;

    if (state) {
        if (cat1_check_state() != RT_EOK) {
            log_debug("cat1_check_state != RT_EOK");
            cat1_power_on();

            // wait rdy
            result = cat1_wait_rdy();
            if (result != RT_EOK) {
                log_debug("can not cat1_wait_rdy");
                return result;
            }
            log_debug("recv rdy");

            cat1_set_cfun_mode(0);
            log_debug("cat1_set_cfun_mode 0");
            rt_thread_mdelay(200);
            cat1_set_cfun_mode(1);
            log_debug("cat1_set_cfun_mode 1");
        }
        else {
             log_debug("cat1_check_state == RT_EOK");
        }
    }
    else {
        // AT+QPOWD
        log_debug("cat1 execute AT+QPOWD");
        cat1_qpowd();
        cat1_wait_powered_down();
        cat1_power_off();
    }

    return result;
}

rt_err_t cat1_init()
{
    rt_err_t result = RT_EOK;
    at_ssl_client_init();

    antenna_active();
    antenna_type_select(current_antenna);
    antenna_switch_to_module(CAT1_MODULE);
    sim_init(CAT1_MODULE, SIM2);

    result = cat1_power_ctrl(1);
    if (result != RT_EOK) {
        log_debug("cat1 power on failed");
        sim_init(NBIOT_MODULE, SIM1);
        antenna_switch_to_module(NBIOT_MODULE);
        antenna_type_select(current_antenna);
        // antenna_deactive();
        sim_deinit();
        return RT_ERROR;
    }

    log_debug("cat1 init success");
    return result;
}

void cat1_deinit()
{
    cat1_power_ctrl(0);
    sim_deinit();

    sim_init(NBIOT_MODULE, SIM1);
    antenna_switch_to_module(NBIOT_MODULE);
    antenna_type_select(current_antenna);
    // antenna_deactive();
}


#include <stdio.h>
#include <string.h>

void save_imei_to_file(const char *imei)
{
    FILE *f = fopen("/imei.txt", "w");
    fwrite(imei, 1, strlen(imei), f);
    fclose(f);
}

void read_imei_from_file(char *output, int read_length)
{
    FILE *f = fopen("/imei.txt", "r");
    if (!f) {
        return;
    }
    fread(output, 1, read_length, f);
    fclose(f);
}


void test_antenna_auto_switch(void)
{
    nbiot_at_client_init();

    antenna_active();
    antenna_type_select(current_antenna);
    antenna_switch_to_module(NBIOT_MODULE);

    sim_init(NBIOT_MODULE, SIM1);
    nbiot_power_on();
    nbiot_disable_sleep_mode();

    extern int get_nbiot_csq(int *, int *);
    int rssi = -1;
    int ber = -1;

    while (1) {
        antenna_type_switch();
        get_nbiot_csq(&rssi, &ber);
        rt_kprintf("rssi: %d, ber: %d\n", rssi, ber);
        rt_thread_mdelay(3000);
    }
}
