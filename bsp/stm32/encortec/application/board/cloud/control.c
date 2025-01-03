#include <stdio.h>
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


// 从文件中读取最有信号天线
rt_err_t get_antenna_from_file()
{
    FILE *fp = fopen("/antenna_signal.txt", "r");
    if (fp == NULL) {
        log_debug("cannot open antenna_signal.txt");
        return RT_ERROR;
    }

    char antenna_type = '\0';
    fread(&antenna_type, 1, 1, fp);
    if (antenna_type == '0') {
        current_antenna = MAIN_ANT;
        log_debug("got antenna from file: %s\n", "MAIN_ANT");
        fclose(fp);
        return RT_EOK;
    }
    else if (antenna_type == '1') {
        current_antenna = REMPTE_ANT;
        log_debug("got antenna from file: %s\n", "REMPTE_ANT");
        fclose(fp);
        return RT_EOK;
    }
    else {
        log_debug("got antenna from file: %s\n", "FAILED");
        fclose(fp);
        return RT_ERROR;
    }
}


// 使用nb两次注网来选择最优信号强度的天线
void check_antenna_signal_strength()
{
    FILE *fp = fopen("/antenna_signal.txt", "w");
    if (fp == NULL) {
        current_antenna = MAIN_ANT;
        log_debug("cannot open antenna_signal.txt, set antenna to default MAIN_ANT");
        return;
    }

    // 测试主天线
    antenna_active();
    antenna_switch_to_module(NBIOT_MODULE);
    sim_init(NBIOT_MODULE, SIM1);
    nbiot_power_on();
    nbiot_disable_sleep_mode();
    
    int main_rssi = 99;
    int rempte_rssi = 99;
    int ber = 99;
    int count = 0;

    count = 0;
    while (count < 3) {
        // 测试主天线信号强度
        antenna_type_select(MAIN_ANT);
        nbiot_set_cfun_mode(0);
        rt_thread_mdelay(200);
        nbiot_set_cfun_mode(1);
        if ((nbiot_check_network(10) == RT_EOK) && (get_nbiot_csq(&main_rssi, &ber) == RT_EOK)) {
            // 记录当前天线信号
            log_debug("got MAIN_ANT rssi: %d; ber: %d\n", main_rssi, ber);
            break;
        }
        count++;
    }

    count = 0;
    while (count < 3) {
        // 测试副天线信号强度
        antenna_type_select(REMPTE_ANT);
        nbiot_set_cfun_mode(0);
        rt_thread_mdelay(200);
        nbiot_set_cfun_mode(1);
        if ((nbiot_check_network(10) == RT_EOK) && (get_nbiot_csq(&rempte_rssi, &ber) == RT_EOK)) {
            // 记录当前天线信号
            log_debug("got REMPTE_ANT rssi: %d; ber: %d\n", rempte_rssi, ber);
            break;
        }
        count++;
    }

    log_info("MAIN_ANT rssi: %d; REMPTE_ANT rssi: %d\n", main_rssi, rempte_rssi);

    if ((main_rssi != 99) && (rempte_rssi != 99)) {
        // 成功获取主副天线信号值比较两个值大小
        current_antenna = main_rssi >= rempte_rssi ? MAIN_ANT : REMPTE_ANT;
    }
    else {
        // 副天线无信号值或者主副天线都无信号值则用主天线
        current_antenna = rempte_rssi == 99 ? MAIN_ANT : REMPTE_ANT;
    }

    log_info("choose current_antenna: %s\n", current_antenna == MAIN_ANT ? "MAIN_ANT" : "REMPTE_ANT");
    fwrite(current_antenna == MAIN_ANT ? "0" : "1", 1, 1 , fp);

    fclose(fp);
}


void nbiot_init()
{
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
            result = cat1_wait_rdy(15000);
            if (result != RT_EOK) {
                log_debug("can not cat1_wait_rdy");
                return result;
            }
            log_debug("recv rdy");
        }
        else {
             log_debug("cat1_check_state == RT_EOK");
        }
    }
    else {
        // AT+QPOWD
        log_debug("cat1 execute AT+QPOWD");
        cat1_qpowd();
        cat1_wait_powered_down(15000);
        cat1_power_off();
    }

    return result;
}

rt_err_t cat1_init()
{
    rt_err_t result = RT_EOK;

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
