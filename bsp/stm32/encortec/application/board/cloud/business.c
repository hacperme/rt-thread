#include "nbiot.h"
#include "gnss.h"
#include "lpm.h"
#include "voltage.h"
#include "tmp116.h"
#include "hdc3021.h"
#include "fdc1004.h"
#include "adxl372.h"
#include "board_pin.h"
#include <string.h>
#include "cJSON.h"
#include "at_client_ssl.h"
#include "control.h"
#include "data_save_as_file.h"
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include "settings.h"
#include <math.h>

#include "logging.h"
// #define DBG_TAG "business"
// #define DBG_LVL DBG_LOG
// #include <rtdbg.h>

static settings_t settings = {0};
static settings_params_t *settings_params = NULL;

static int start_tick_ms = 0;
static int end_tick_ms = 0;

struct FileSystem fs;

/*status defines*/
enum {
    RECORD_WAKEUP_SROUCE,
    EXTERNAL_DEVICES_INIT,
    COLLECT_SENSOR_DATA,
    SAVE_SENSOR_DATA,
    NBIOT_INIT,
    NBIOT_WAIT_NETWORK_RDY,
    NBIOT_WAIT_SERVER_CONNECT,
    NBIOT_REPORT_CONTROL_DATA,
    NBIOT_REPORT_SENSOR_DATA,
    CAT1_INIT,
    CAT1_WAIT_NETWORK_RDY,
    CAT1_UPLOAD_FILE,
    SLEEP,
};

static rt_sem_t stm32_sleep_ack_sem = RT_NULL;
void adxl372_inact_event_handler(void)
{
    // log_debug("adxl372_inact_event_handler called");
    rt_sem_release(stm32_sleep_ack_sem);
    adxl372_int1_pin_irq_disable();
}

enum wakeup_source_type {RTC_SOURCE, SHAKE_SOURCE, OTHER_SOURCE};
enum wakeup_source_type wakeup_source_flag = -1;
int record_wakeup_source()
{
    log_debug("record wakeup source");
    rt_uint8_t source = get_wakeup_source();
    if (is_rtc_wakeup(&source)) {
        wakeup_source_flag = RTC_SOURCE;  // rtc
    }
    else if (is_pin_wakeup(&source)) {
        wakeup_source_flag = SHAKE_SOURCE;  // shake
    }
    else {
        wakeup_source_flag = OTHER_SOURCE;
    }
    log_debug("wakeup souce flag: %d", wakeup_source_flag);
    return 0;
}

int external_devices_init()
{
    log_debug("external devices init");
    rt_err_t res = RT_EOK;

    //TODO: check file system `GNSS.reported`

    antenna_active_pin_enable(PIN_HIGH);
    sensor_pwron_pin_enable(PIN_HIGH);
    flash_pwron_pin_enable(PIN_LOW);
    sim_select_pin_enable(PIN_LOW);  // set to nb
    nb_cat1_rf_pin_enable(PIN_LOW);  // low for nbiot rf
    nbiot_power_off();
    cat1_power_off();
    intn_ext_ant_pin_enable(PIN_LOW);  // low for main ant
    esp32_power_off();
    gnss_open();

    rt_uint16_t milliscond = 520;
    rt_uint16_t threshold = 10;  // 0.1 g
    rt_uint8_t measure_val = 0x03;
    rt_uint8_t odr_val = 0x60;
    rt_uint8_t hpf_val = 0x03;

    res = adxl372_init();
    log_debug("adxl372_init %s", res != RT_EOK ? "failed" : "success");
    res = adxl372_set_measure_config(&measure_val, &odr_val, &hpf_val);
    log_debug(
        "adxl372_set_measure_config(measure_val=0x%02X, odr_val=0x%02X, hpf_val=0x%02X) %s",
        measure_val, odr_val, hpf_val, res != RT_EOK ? "failed" : "success"
    );

    return 0;
}

struct SensorData
{
    float temp1;
    float temp2;
    float temp3;
    float humi;
    float lat;
    float lng;
    float acc_x;
    float acc_y;
    float acc_z;
    float water_level;
    int cur_vol;
    int vcat_vol;
    int vbat_vol;
};
struct SensorData sensor_data = {0};
static struct rt_i2c_bus_device *iic_dev;
char nmea_buf[1024] = {0};
float ACC_XYZ_BUFF[1024][3] = {0};
int read_acc_xyz_result = RT_EOK;
int collect_sensor_data()
{
    rt_err_t res = RT_EOK;
    log_debug("collect sensor data");
    iic_dev = rt_i2c_bus_device_find("i2c1");

    if (wakeup_source_flag != RTC_SOURCE) {
        read_acc_xyz_result = adxl372_measure_acc(ACC_XYZ_BUFF, 1024);
        log_debug("adxl372_measure_acc %s", read_acc_xyz_result == RT_EOK ? "success" : "failed");

        // read cur_vol, vcat_vol, vbat_vol
        rt_uint16_t cur_vol = 0;
        rt_uint16_t vcat_vol = 0;
        rt_uint16_t vbat_vol = 0;
        res = cur_vol_read(&cur_vol);
        log_debug("cur_vol_read %s, cur_vol %dmv", res == RT_EOK ? "success" : "failed", cur_vol);
        res = vcap_vol_read(&vcat_vol);
        log_debug("vcap_vol_read %s, vcat_vol %dmv", res == RT_EOK ? "success" : "failed", vcat_vol);
        res = vbat_vol_read(&vbat_vol);
        log_debug("vbat_vol_read %s, vbat_vol %dmv", res == RT_EOK ? "success" : "failed", vbat_vol);
        sensor_data.cur_vol = cur_vol;
        sensor_data.vcat_vol = vcat_vol;
        sensor_data.vbat_vol = vbat_vol;
    }
    
    // temp1, temp2
    float temp1 = 0.0;
    float temp2 = 0.0;
    rt_uint8_t addrs[2] = {TMP116_1_ADDR, TMP116_2_ADDR};

    char msg[128] = {0};
    res = temp116_read_temperture(iic_dev, addrs[0], &temp1);
    snprintf(msg, 128, "temp116_read_temperture %s, temp3=%f", RT_EOK ? "failed" : "success", temp1);
    log_debug(msg);
    // log_debug("temp116_read_temperture %s, temp1=%f", res != RT_EOK ? "failed" : "success", temp1);

    res = temp116_read_temperture(iic_dev, addrs[1], &temp2);
    rt_memset(msg, 0, 128);
    snprintf(msg, 128, "temp116_read_temperture %s, temp3=%f", res != RT_EOK ? "failed" : "success", temp2);
    log_debug(msg);
    // log_debug("temp116_read_temperture %s, temp2=%f", res != RT_EOK ? "failed" : "success", temp2);
    if (res == RT_EOK) {
        sensor_data.temp1 = temp1;
        sensor_data.temp2 = temp2;
    }

    // temp3, humi
    float temp3 = 0.0;
    float humi = 0.0;
    res = hdc3021_read_temp_humi(iic_dev, &temp3, &humi);
    rt_memset(msg, 0, 128);
    snprintf(msg, 128, "hdc3021_read_temp_humi %s, temp3=%f, humi=%f", res != RT_EOK ? "failed" : "success", temp3, humi);
    log_debug(msg);
    // log_debug("hdc3021_read_temp_humi %s, temp3=%f, humi=%f", res != RT_EOK ? "failed" : "success", temp3, humi);
    if (res == RT_EOK) {
        sensor_data.temp3 = temp3;
        sensor_data.humi = humi;
    }

    if (wakeup_source_flag != RTC_SOURCE) {
        // water level
        float value = 0.0;
        res = fdc1004_meas_data(iic_dev, &value);
        log_debug("fdc1004_meas_data %s, value=%f", res != RT_EOK ? "failed" : "success", value);
        if (res == RT_EOK) {
            sensor_data.water_level = value;
        }

        char msg[26] = {0};
        // read nmea
        nmea_item nmea_item = {0};
        for (int i=0; i < 30; i++) {
            rt_memset(&nmea_item, 0, sizeof(nmea_item));
            gnss_read_nmea_item(&nmea_item, 1000);
            if (nmea_item.is_valid) {
                rt_memcpy(nmea_buf, nmea_item.GNGGA, rt_strlen(nmea_item.GNGGA));
                log_debug("gnss_read_nmea_item %s", res == RT_EOK ? "success" : "failed");
                log_debug("nmea_item->GNRMC: %s", nmea_item.GNRMC);
                log_debug("nmea_item->is_valid: %d", nmea_item.is_valid);
                log_debug("nmea_item->GNGGA: %s", nmea_item.GNGGA);
                log_debug("nmea_item->GNGLL: %s", nmea_item.GNGLL);
                log_debug("nmea_item->GNVTG: %s", nmea_item.GNVTG);
                log_debug("nmea_item->GNGSA_SIZE: %d", nmea_item.GNGSA_SIZE);
                break;
            }
            rt_thread_mdelay(1000);
        }
    }

    return 0;
}

static int set_network_config_flag = 0;
static int nbiot_wait_network_retry_times = 0;
static struct datetime current_time;
static char report_timestamp_buf[64] = {0};

enum nbiot_network_status {
    NBIOT_NETWORK_RDY,
    NBIOT_NETWORK_NOT_RDY,
    NBIOT_NETWORK_RETRY
};

enum nbiot_network_status nbiot_wait_network_ready()
{
    if (nbiot_wait_network_retry_times >= 3) {
        log_debug("nbiot_wait_network_retry_times >= 3, goto sleep");
        return NBIOT_NETWORK_NOT_RDY;
    }
    nbiot_wait_network_retry_times += 1;

    log_debug("nbiot wait network ready");

    nbiot_enable_echo(0);
    if (nbiot_check_network(10) == RT_EOK) {
        log_debug("nbiot network has been ready");
        rt_err_t result = RT_EOK;
        result = nbiot_get_current_datetime(&current_time);
        log_debug("current_time.year: %d", current_time.year);
        log_debug("current_time.month: %d", current_time.month);
        log_debug("current_time.day: %d", current_time.day);
        log_debug("current_time.hour: %d", current_time.hour);
        log_debug("current_time.minute: %d", current_time.minute);
        log_debug("current_time.second: %d", current_time.second);
        result = rtc_set_datetime(current_time.year + 2000, current_time.month, current_time.day, current_time.hour, current_time.minute, current_time.second);
        log_debug("set rtc time result: %d", result);
        return NBIOT_NETWORK_RDY;
    }
    else {
        if (! set_network_config_flag) {
            struct network_config config = {"QUECTEL.VF.LPWA", "2,8,20", "3,0"};
            if(nbiot_set_network_config(&config) == RT_EOK) {
                log_debug("nbiot_set_network_config success");
                set_network_config_flag = 1;
            }
            else {
                log_debug("nbiot_set_network_config failed");
            }
        }
        return NBIOT_NETWORK_RETRY;
    }
}

static int set_lwm2m_config_flag = 0;
static int nbiot_wait_server_connect_retry_times = 0;

enum nbiot_server_connect_status {
    NBIOT_SERVER_CONNECT_RDY,
    NBIOT_SERVER_CONNECT_NOT_RDY,
    NBIOT_SERVER_CONNECT_RETRY
};

int nbiot_wait_server_connect_ready()
{
    if (nbiot_wait_server_connect_retry_times >= 5) {
        log_debug("nbiot_wait_server_connect_retry_times >= 5, goto sleep");
        return NBIOT_SERVER_CONNECT_NOT_RDY;
    }
    nbiot_wait_server_connect_retry_times += 1;

    log_debug("nbiot wait server connect");
    rt_err_t result = RT_EOK;

    if (nbiot_check_qiotstate() == RT_EOK) {
        // server connect ok, goto report control data to server
        return NBIOT_SERVER_CONNECT_RDY;
    }
    else {
        nbiot_lwm2m_deregister();
        struct lwm2m_config config = {"pe15TE", "aXp5Y0hudFBkbmho", 0, "coap://iot-south.quecteleu.com:5683", 86400, 1, 1, 1};
        if (! set_lwm2m_config_flag && nbiot_check_lwm2m_config(&config) != RT_EOK) {
            if (nbiot_set_lwm2m_config(&config) == RT_EOK) {
                set_lwm2m_config_flag = 1;
                log_debug("nbiot_set_lwm2m_config success");
            }
            else {
                log_debug("nbiot_set_lwm2m_config failed");
            }
        }
        // connect failed, but network ready
        if (nbiot_check_network(1) == RT_EOK) {
            nbiot_lwm2m_register();
            return NBIOT_SERVER_CONNECT_RETRY;
        }
        else {
            // connect failed and network not ready
            return NBIOT_NETWORK_RETRY;
        }
    }
}

static int nbiot_report_ctrl_retry_times = 0;
static struct ServerCtrlData server_ctrl_data = {0};

enum nbiot_report_ctrl_data_status {
    NBIOT_REPORT_CTRL_DATA_SUCCESS,
    NBIOT_REPORT_CTRL_DATA_FAILED,
    NBIOT_REPORT_CTRL_DATA_RETRY
};

int nbiot_report_ctrl_data_to_server()
{
    if (nbiot_report_ctrl_retry_times >= 3) {
        log_debug("nbiot_report_ctrl_retry_times >= 3, goto report sensor data");
        return NBIOT_REPORT_CTRL_DATA_FAILED;
    }
    nbiot_report_ctrl_retry_times += 1;

    log_debug("nbiot report ctrl data to server");
    char data[128] = {0};
    snprintf(data, 128, "{\"12\":%d,\"13\":false}", settings_params->collect_interval);
    if (nbiot_report_ctrl_data(data, rt_strlen(data)) == RT_EOK) {
        nbiot_recv_ctrl_data(64, &server_ctrl_data);  // read ctrl data from server
        log_debug("server_ctrl_data.CollectInterval: %d", server_ctrl_data.CollectInterval);
        log_debug("server_ctrl_data.Cat1FileUpload: %d", server_ctrl_data.Cat1FileUpload);
        log_debug("report ctrl data to server success, goto, report sensor data");
        settings_params_t p = {0};
        if (server_ctrl_data.CollectInterval != 0) {
            p.collect_interval = server_ctrl_data.CollectInterval;
            settings_update(&settings, &p);
        }
        return NBIOT_REPORT_CTRL_DATA_SUCCESS;
    }
    else {
        log_debug("report ctrl data to server falied");
        if (nbiot_check_qiotstate() == RT_EOK) {
            return NBIOT_REPORT_CTRL_DATA_RETRY;
        }
        else {
            log_debug("lwm2m server connect error, goto wait server connect");
            return NBIOT_SERVER_CONNECT_RETRY;
        }
    }
}

static int nbiot_report_sensor_data_retry_times = 0;

enum nbiot_report_sensor_data_status {
    NBIOT_REPORT_SENSOR_DATA_SUCCESS,
    NBIOT_REPORT_SENSOR_DATA_FAILED,
    NBIOT_REPORT_SENSOR_DATA_RETRY
};

int nbiot_report_sensor_data_to_server()
{
    if (nbiot_report_sensor_data_retry_times >= 3) {
        log_debug("nbiot_report_sensor_data_retry_times >= 3, goto cat1 upload file");
        return NBIOT_REPORT_SENSOR_DATA_FAILED;
    }
    nbiot_report_sensor_data_retry_times += 1;
    
    log_debug("nbiot report sensor data to server");

    if (rt_strlen(nmea_buf) > 0) {
        log_debug("send nmea_buf data!");
        nbiot_set_qiotlocext(nmea_buf);
    }
    else {
        log_debug("nmea_buf char is NULL");
    }

    char data_string[1024] = {0};
    if (wakeup_source_flag != RTC_SOURCE) {
        snprintf(
            data_string, 
            sizeof(data_string), 
            "{" \
            "\"1\":%f,\"5\":%f,\"11\":%f,\"2\":%f,\"6\":%f,\"7\":%f,\"8\":%f," \
            "\"9\":%f,\"10\":%f,\"15\":%f,\"16\":%d,\"17\":%d,\"18\":%d,\"14\":%d," \
            "\"20\":%d,\"21\":%s,\"3\":{\"1\":\"%s\",\"2\":\"%s\"}" \
            "}",
            sensor_data.temp1, 
            sensor_data.temp2, 
            sensor_data.temp3, 
            sensor_data.humi, 
            sensor_data.lat, 
            sensor_data.lng, 
            sensor_data.acc_x, 
            sensor_data.acc_y, 
            sensor_data.acc_z,
            sensor_data.water_level,
            sensor_data.cur_vol,
            sensor_data.vcat_vol,
            sensor_data.vbat_vol,
            wakeup_source_flag,
            21,
            "true",
            "abcdefghij",
            "0213456789"
        );
    }
    else {
        snprintf(
            data_string, 
            sizeof(data_string), 
            "{\"1\":%f,\"5\":%f,\"11\":%f,\"2\":%f,\"14\":%d}",
            sensor_data.temp1, 
            sensor_data.temp2, 
            sensor_data.temp3, 
            sensor_data.humi,
            wakeup_source_flag
        );
    }
    if (nbiot_report_model_data(data_string, rt_strlen(data_string)) == RT_EOK) {
        return NBIOT_REPORT_SENSOR_DATA_SUCCESS;
    }
    else {
        if (nbiot_check_qiotstate() == RT_EOK) {
            return NBIOT_REPORT_SENSOR_DATA_RETRY;
        }
        else {
            return NBIOT_SERVER_CONNECT_RETRY;
        }
    }
}

enum cat1_network_status {
    CAT1_NETWORK_RDY,
    CAT1_NETWORK_NOT_RDY,
    CAT1_NETWORK_ERROR
};
static int set_cat1_network_config_flag = 0;
static int cat1_wait_network_retry_times = 0;
enum cat1_network_status cat1_wait_network_ready()
{
    if (cat1_wait_network_retry_times >= 3) {
        log_debug("cat1_wait_network_retry_times >= 3, goto sleep");
        return CAT1_NETWORK_ERROR;
    }
    cat1_wait_network_retry_times += 1;

    cat1_enable_echo(0);
    // wait network ready for cat1
    if (cat1_check_network(10) != RT_EOK) {
        log_debug("cat1 network not ready");

        if (! set_cat1_network_config_flag) {
            if(cat1_set_network_config() == RT_EOK) {
                log_debug("cat1_set_network_config success");
                set_cat1_network_config_flag = 1;
            }
            else {
                log_debug("cat1_set_network_config failed");
            }
        }

        return CAT1_NETWORK_NOT_RDY;
    }
    else {
        log_debug("cat1 network ready");
        return CAT1_NETWORK_RDY;
    }
}

enum cat1_upload_file_status {
    CAT1_UPLOAD_FILE_SUCCESS,
    CAT1_UPLOAD_FILE_FAILED,
    CAT1_UPLOAD_FILE_ERROR
};

static int cat1_upload_file_retry_times = 0;

extern int at_https_upload_file(const char *filename);
int cat1_upload_file()
{
    if (cat1_upload_file_retry_times >= 3) {
        log_debug("cat1_upload_file_retry_times >= 3, goto sleep");
        return CAT1_UPLOAD_FILE_ERROR;
    }
    cat1_upload_file_retry_times += 1;

    rt_err_t result = RT_EOK;
    log_debug("cat1 upload file, Cat1FileUpload: %d", server_ctrl_data.Cat1FileUpload);

    char *file_name = NULL;
    while (file_name = get_oldest_file_name(&fs)) {
        log_debug("file name is: %s", file_name);
        // 需要上报
        // if (at_http_upload_file_chunked(file_name) == -1) {
        if (at_https_upload_file(file_name) == -1) {
            log_debug("at http upload file failed");
            return CAT1_UPLOAD_FILE_FAILED;
        }
        else {
            delete_oldest_file(&fs);
            log_debug("at http upload '%s' success", file_name);
        }
    }

    return CAT1_UPLOAD_FILE_SUCCESS;
}

static int DEFAULT_REMAINING_SECONDS = 600;
void stm32_sleep()
{   
    log_debug("wait stm32_sleep_ack_sem release");
    rt_sem_take(stm32_sleep_ack_sem, rt_tick_from_millisecond(1000));
    log_debug("go into sleep");

    sensor_pwron_pin_enable(PIN_LOW);
    esp32_power_off();
    gnss_close();
    flash_pwron_pin_enable(PIN_LOW);
    
    cat1_deinit();
    antenna_active_pin_enable(PIN_LOW);

    end_tick_ms = rt_tick_get_millisecond();
    log_debug("start tick ms: %d", start_tick_ms);
    log_debug("end tick ms: %d", end_tick_ms);
    log_debug("settings_params->collect_interval: %d", settings_params->collect_interval);

    int remaining = 0;
    remaining = settings_params->collect_interval - ((end_tick_ms - start_tick_ms) / 1000);
    log_debug("remaining: %d", remaining);
    rtc_set_wakeup(remaining > 0 ? remaining : 10);
    shut_down();
}

char sensor_data_buffer[4096] = {0};
void save_sensor_data()
{
    int length = 0;
    char temp_buf[128] = {0};
    log_debug("save sensor data");
    
    snprintf(
        sensor_data_buffer + strlen(sensor_data_buffer), 
        sizeof(sensor_data_buffer) - strlen(sensor_data_buffer), 
        "timestamp: %d/%02d/%02d %02d:%02d:%02d\r\n",
        current_time.year + 2000,
        current_time.month,
        current_time.day,
        current_time.hour,
        current_time.minute,
        current_time.second
    );

    // temperature1: 25.0
    snprintf(sensor_data_buffer + strlen(sensor_data_buffer), sizeof(sensor_data_buffer) - strlen(sensor_data_buffer), "temperature1: %0.2f\r\n", sensor_data.temp1);

    // temperature2: 25.0
    snprintf(sensor_data_buffer + strlen(sensor_data_buffer), sizeof(sensor_data_buffer) - strlen(sensor_data_buffer), "temperature2: %0.2f\r\n", sensor_data.temp2);

    // temperature3: 25.0
    snprintf(sensor_data_buffer + strlen(sensor_data_buffer), sizeof(sensor_data_buffer) - strlen(sensor_data_buffer), "temperature3: %0.2f\r\n", sensor_data.temp3);

    // humidity: 56
    snprintf(sensor_data_buffer + strlen(sensor_data_buffer), sizeof(sensor_data_buffer) - strlen(sensor_data_buffer), "humidity: %0.2f\r\n", sensor_data.humi);

    // water_level: 2.0
    snprintf(sensor_data_buffer + strlen(sensor_data_buffer), sizeof(sensor_data_buffer) - strlen(sensor_data_buffer), "water_level: %0.2f\r\n", sensor_data.water_level);

    // vbat: 3.6
    snprintf(sensor_data_buffer + strlen(sensor_data_buffer), sizeof(sensor_data_buffer) - strlen(sensor_data_buffer), "vbat: %d\r\n", sensor_data.vbat_vol);

    // vcur: 2.9
    snprintf(sensor_data_buffer + strlen(sensor_data_buffer), sizeof(sensor_data_buffer) - strlen(sensor_data_buffer), "vcur: %d\r\n", sensor_data.cur_vol);

    // vcap: 3.3
    snprintf(sensor_data_buffer + strlen(sensor_data_buffer), sizeof(sensor_data_buffer) - strlen(sensor_data_buffer), "vcap: %d\r\n", sensor_data.vcat_vol);

    // GNSS: $GNGGA:,,,,,,GNSS:
    snprintf(sensor_data_buffer + strlen(sensor_data_buffer), sizeof(sensor_data_buffer) - strlen(sensor_data_buffer), "GNSS: ");
    snprintf(sensor_data_buffer + strlen(sensor_data_buffer), sizeof(sensor_data_buffer) - strlen(sensor_data_buffer), nmea_buf);
    snprintf(sensor_data_buffer + strlen(sensor_data_buffer), sizeof(sensor_data_buffer) - strlen(sensor_data_buffer), "\r\n");

    // log_debug("sensor_data_buffer:\r\n%s", sensor_data_buffer);
    data_save_as_file(&fs, sensor_data_buffer, strlen(sensor_data_buffer), false);
    memset(sensor_data_buffer, 0, sizeof(sensor_data_buffer));

    log_debug("read_acc_xyz_result: %d", read_acc_xyz_result);

    if (read_acc_xyz_result == RT_EOK)
    {
        snprintf(sensor_data_buffer + strlen(sensor_data_buffer), sizeof(sensor_data_buffer) - strlen(sensor_data_buffer), "acceleration:\r\n");
        for (rt_uint16_t i = 0; i < 1024; i++)
        {   
            if (fabs(ACC_XYZ_BUFF[i][0]) > fabs(sensor_data.acc_x)) {
                log_debug("set acc_x %f", ACC_XYZ_BUFF[i][0]);
                sensor_data.acc_x = ACC_XYZ_BUFF[i][0];
            }

            if (fabs(ACC_XYZ_BUFF[i][1]) > fabs(sensor_data.acc_y)) {
                log_debug("set acc_y %f", ACC_XYZ_BUFF[i][1]);
                sensor_data.acc_y = ACC_XYZ_BUFF[i][1];
            }

            if (fabs(ACC_XYZ_BUFF[i][2]) > fabs(sensor_data.acc_z)) {
                log_debug("set acc_z %f", ACC_XYZ_BUFF[i][2]);
                sensor_data.acc_z = ACC_XYZ_BUFF[i][2];
            }

            rt_memset(temp_buf, 0, 128);
            length = sprintf(temp_buf, "%0.2f,%0.2f,%0.2f\r\n", ACC_XYZ_BUFF[i][0], ACC_XYZ_BUFF[i][1], ACC_XYZ_BUFF[i][2]);
            // log_debug("temp_buf: %s", temp_buf);
            // 循环写三轴数据
            if (strlen(sensor_data_buffer) + length > sizeof(sensor_data_buffer)) {
                if (data_save_as_file(&fs, sensor_data_buffer, strlen(sensor_data_buffer), false) == 0) {
                    log_debug("Data saved successfully.");
                    rt_memset(sensor_data_buffer, 0, sizeof(sensor_data_buffer));
                    snprintf(sensor_data_buffer + strlen(sensor_data_buffer), sizeof(sensor_data_buffer) - strlen(sensor_data_buffer), temp_buf);
                } else {
                    log_error("Data saved failed.");
                    // 本次写入失败，重试写入，最多重试3次?
                }
            }
            else {
                snprintf(sensor_data_buffer + strlen(sensor_data_buffer), sizeof(sensor_data_buffer) - strlen(sensor_data_buffer), temp_buf);
            }
        }
    }
}

static rt_mq_t sm_mq = NULL;

static void sm_set_status(int status) {
    rt_mq_send(sm_mq, &status, sizeof(int));
}

static int sm_get_status(void) {
    int status = -1;
    rt_mq_recv(sm_mq, &status, sizeof(int), RT_WAITING_FOREVER);
    return status;
}

static void sm_set_initial_status(int status) {
    sm_set_status(status);
}

static void sm_init(void) {
    sm_mq = rt_mq_create("sm_mq", 16, 8, RT_IPC_FLAG_FIFO);
    sm_set_initial_status(RECORD_WAKEUP_SROUCE);
    // sm_set_initial_status(CAT1_INIT);
}

void main_business_entry(void)
{
    rt_err_t result;
    int status;
    int rv = 0;
    
    if (settings_init(&settings, "/settings.conf", NULL) == 0) {
        settings_params = settings_read(&settings);
        if (!settings_params) {
            settings_params = &settings.params;
        }
        if (settings_params->collect_interval == 0) {
            settings_params->collect_interval = 600;
        }

        log_info("settings_params->collect_interval: %d", settings_params->collect_interval);
    }
 
    stm32_sleep_ack_sem = rt_sem_create("stm32_sleep_ack_sem", 0, RT_IPC_FLAG_FIFO);
    rtc_init();

    start_tick_ms = rt_tick_get_millisecond();
    log_debug("start tick ms: %d", start_tick_ms);

    sm_init();
    data_save_as_file_init(&fs, 24 * 1024);

    while (1)
    {
        status = sm_get_status();
        switch (status)
        {
            case RECORD_WAKEUP_SROUCE:
                record_wakeup_source();
                sm_set_status(EXTERNAL_DEVICES_INIT);
                break;
            case EXTERNAL_DEVICES_INIT:
                external_devices_init();
                sm_set_status(COLLECT_SENSOR_DATA);
                break;
            case COLLECT_SENSOR_DATA:
                collect_sensor_data();
                sm_set_status(NBIOT_INIT);
                break;
            case NBIOT_INIT:
                nbiot_init();
                sm_set_status(NBIOT_WAIT_NETWORK_RDY);
                break;
            case NBIOT_WAIT_NETWORK_RDY:
                nbiot_set_cfun_mode(0);
                log_debug("nbiot_set_cfun_mode 0");
                rt_thread_mdelay(200);
                nbiot_set_cfun_mode(1);
                log_debug("nbiot_set_cfun_mode 1");
                rv = nbiot_wait_network_ready();
                if (rv == NBIOT_NETWORK_NOT_RDY) {
                    nbiot_deinit();
                    save_sensor_data();
                    sm_set_status(SLEEP);
                }
                else if (rv == NBIOT_NETWORK_RETRY) {
                    nbiot_set_cfun_mode(0);
                    log_debug("nbiot_set_cfun_mode 0");
                    rt_thread_mdelay(200);
                    nbiot_set_cfun_mode(1);
                    log_debug("nbiot_set_cfun_mode 1");
                    sm_set_status(NBIOT_WAIT_NETWORK_RDY);
                }
                else {
                    save_sensor_data();
                    sm_set_status(NBIOT_WAIT_SERVER_CONNECT);
                }
                break;
            case NBIOT_WAIT_SERVER_CONNECT:
                rv = nbiot_wait_server_connect_ready();
                if (rv == NBIOT_SERVER_CONNECT_NOT_RDY) {
                    nbiot_deinit();
                    sm_set_status(SLEEP);
                }
                else if (rv == NBIOT_SERVER_CONNECT_RDY) {
                    sm_set_status(NBIOT_REPORT_CONTROL_DATA);
                }
                else if (rv == NBIOT_SERVER_CONNECT_RETRY) {
                    sm_set_status(NBIOT_WAIT_SERVER_CONNECT);
                }
                else if (rv == NBIOT_NETWORK_RETRY) {
                    sm_set_status(NBIOT_WAIT_NETWORK_RDY);
                }
                else {
                    // pass
                }
                break;
            case NBIOT_REPORT_CONTROL_DATA:
                rv = nbiot_report_ctrl_data_to_server();
                if (rv == NBIOT_REPORT_CTRL_DATA_FAILED || rv == NBIOT_REPORT_CTRL_DATA_SUCCESS) {
                    sm_set_status(NBIOT_REPORT_SENSOR_DATA);
                }
                else if (rv == NBIOT_REPORT_CTRL_DATA_RETRY) {
                    sm_set_status(NBIOT_REPORT_CONTROL_DATA);
                }
                else if (rv == NBIOT_SERVER_CONNECT_RETRY) {
                    sm_set_status(NBIOT_WAIT_SERVER_CONNECT);
                }
                else  {
                    // pass
                }
                break;
            case NBIOT_REPORT_SENSOR_DATA:
                rv = nbiot_report_sensor_data_to_server();
                if (rv == NBIOT_REPORT_SENSOR_DATA_FAILED || rv == NBIOT_REPORT_SENSOR_DATA_SUCCESS) {
                    nbiot_deinit();
                    sm_set_status(CAT1_INIT);
                }
                else if (rv == NBIOT_REPORT_SENSOR_DATA_RETRY) {
                    sm_set_status(NBIOT_REPORT_SENSOR_DATA);
                }
                else if (rv == NBIOT_SERVER_CONNECT_RETRY) {
                    sm_set_status(NBIOT_WAIT_SERVER_CONNECT);
                }
                else {
                    // pass
                }
                break;
            case CAT1_INIT:
                if (cat1_init() != RT_EOK) {
                    sm_set_status(SLEEP);
                }
                else {
                    sm_set_status(CAT1_WAIT_NETWORK_RDY);
                }
                break;
            case CAT1_WAIT_NETWORK_RDY:
                rv = cat1_wait_network_ready();
                if (rv == CAT1_NETWORK_NOT_RDY) {
                    cat1_set_cfun_mode(0);
                    rt_thread_mdelay(200);
                    cat1_set_cfun_mode(1);
                    sm_set_status(CAT1_WAIT_NETWORK_RDY);
                }
                else if (rv == CAT1_NETWORK_RDY) {
                    sm_set_status(CAT1_UPLOAD_FILE);
                }
                else if (rv == CAT1_NETWORK_ERROR) {
                    cat1_deinit();
                    sm_set_status(SLEEP);
                }
                break;
            case CAT1_UPLOAD_FILE:
                rv = cat1_upload_file();
                if (rv == CAT1_UPLOAD_FILE_SUCCESS || rv == CAT1_UPLOAD_FILE_ERROR) {
                    sm_set_status(SLEEP);
                }
                else {
                    sm_set_status(CAT1_UPLOAD_FILE);
                }
                break;
            case SLEEP:
                stm32_sleep();
                break;
            default:
                log_warn("unknown status: %d", status);
                break;
        }
    }
}
// MSH_CMD_EXPORT(main_business_entry, main_business_entry);
