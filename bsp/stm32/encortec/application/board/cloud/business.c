#include "nbiot.h"
#include "gnss.h"
#include "lpm.h"
#include "tmp116.h"
#include "hdc3021.h"
#include "fdc1004.h"
#include "adxl372.h"
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
#include <dirent.h>
#include "at_client_https.h"
#include "voltage_adc.h"
#include "led.h"
#include "watch_dog.h"
#include "at_data_transf.h"


#include "logging.h"
// #define DBG_TAG "business"
// #define DBG_LVL DBG_LOG
// #include <rtdbg.h>

static rt_uint8_t wdg_id;
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
    ESP32_WIFI_TRANSFER_DATA,
    SLEEP,
    CHECK_ANTENNA_SIGNAL_STRENGTH
};

// static rt_sem_t stm32_sleep_ack_sem = RT_NULL;
// void adxl372_inact_event_handler(void)
// {
//     // log_debug("adxl372_inact_event_handler called");
//     if (stm32_sleep_ack_sem != RT_NULL)
//     {
//         rt_sem_release(stm32_sleep_ack_sem);
//     }
//     adxl372_int1_pin_irq_disable();
//     // adxl372_set_standby();
//     // adxl372_recv_inact_event_thd_stop();
// }

enum wakeup_source_type {RTC_SOURCE, SHAKE_SOURCE, OTHER_SOURCE};
enum wakeup_source_type wakeup_source_flag = -1;
static int reset_source_flag = 0;
int record_wakeup_source()
{
    log_debug("record wakeup source");
    rt_uint8_t source = get_wakeup_source();
    log_debug("wakeup source: %d", source);
    if (is_rtc_wakeup(&source)) {
        wakeup_source_flag = RTC_SOURCE;  // rtc
    }
    else if (is_pin_wakeup(&source)) {
        wakeup_source_flag = SHAKE_SOURCE;  // shake
    }
    else {
        wakeup_source_flag = OTHER_SOURCE;
    }
    log_debug("wakeup source flag: %d", wakeup_source_flag);

    rt_uint8_t rst_status = get_reset_source();
    log_debug("rst_status=%d", rst_status);
    if ((rst_status & (1 << 0)) >> 0 == 1) {
        // 首次上电安装
        reset_source_flag = 1;
        debug_led1_on();
    }
    else {
        reset_source_flag = 0;
    }
    log_debug("reset source flag: %d", reset_source_flag);

    return 0;
}

int external_devices_init()
{
    log_debug("external devices init");
    rt_err_t res = RT_EOK;

    nbiot_at_client_init();  // nb at engine
    at_ssl_client_init();  // cat1 at engine

    //TODO: check file system `GNSS.reported`
    sensor_pwron_pin_enable(PIN_HIGH);
    nbiot_power_off();
    cat1_power_off();
    esp32_power_off();

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
    int vcap_vol;
    int vbat_vol;
    rt_int16_t *x_buf;
    rt_int16_t *y_buf;
    rt_int16_t *z_buf;
    rt_uint16_t xyz_size;
    rt_uint16_t *cur_buff;
    int cur_buff_size;
};
struct SensorData sensor_data = {0};
static struct rt_i2c_bus_device *iic_dev;
char nmea_buf[1024] = {0};
unsigned int xyz_read_start_timestamp = 0;


rt_err_t get_nmea_item_data(nmea_item *item)
{
    FILE *fd = NULL;
    time_t cur_timestamp = 0;
    time_t prev_timestamp = 0;
    char nmea_time_filename[] = "/prev_nmea_time.bin";

    // 判断上次读取的时间
    time(&cur_timestamp);
    log_debug("get_nmea_item_data cur_timestamp: %d\n", cur_timestamp);

    fd = fopen(nmea_time_filename, "rb");
    if (fd) {
        log_debug("get_nmea_item_data timstamp file exists.");
        fread(&prev_timestamp, 1, sizeof(prev_timestamp), fd);
        log_debug("get_nmea_item_data prev_timestamp: %d\n", prev_timestamp);
        if (!reset_source_flag && (cur_timestamp - prev_timestamp < 30 * 24 * 3600)) {
            // 距上次上报位置数据未满30天 且 非首次安装 则需要上报
            log_debug("get_nmea_item_data, prev_timestamp - cur_timestamp < 30 days");
            fclose(fd);
            fd = NULL;
            return RT_ERROR;
        }
        fclose(fd);
        fd = NULL;
    }

    gnss_open();

    // 读取定位数据
    for (int i=0; i < 30; i++) {
        rt_memset(item, 0, sizeof(item));
        gnss_read_nmea_item(item, 1000);
        if (item->is_valid) {
            rt_memcpy(nmea_buf, item->GNGGA, rt_strlen(item->GNGGA));
            log_debug("nmea_item->GNRMC: %s", item->GNRMC);
            log_debug("nmea_item->is_valid: %d", item->is_valid);
            log_debug("nmea_item->GNGGA: %s", item->GNGGA);
            log_debug("nmea_item->GNGLL: %s", item->GNGLL);
            log_debug("nmea_item->GNVTG: %s", item->GNVTG);
            log_debug("nmea_item->GNGSA_SIZE: %d", item->GNGSA_SIZE);
            break;
        }
        rt_thread_mdelay(1000);
    }

    gnss_close();

    if (item->is_valid) {
        // 记录当前上报的时间
        fd = fopen(nmea_time_filename, "wb");
        if (fd) {
            fwrite(&cur_timestamp, 1, sizeof(cur_timestamp), fd);
            log_debug("get_nmea_item_data write cur_timestamp to file: %d", cur_timestamp);
            fclose(fd);
        }
        return RT_EOK;
    }

    return RT_ERROR;
}


int collect_sensor_data()
{
    rt_err_t res = RT_EOK;
    log_debug("collect sensor data");

    rt_uint16_t milliscond = 520;
    rt_uint16_t threshold = 5;         // 0.1 g
    rt_uint8_t measure_val = 0x00;
    rt_uint8_t odr_val = 0x40;          // 1600Hz
    rt_uint8_t hpf_val = 0x03;
    rt_uint8_t fifo_format = 0;         // FIFO store x y z.
    rt_uint8_t fifo_mode = 1;           // stream mode.
    rt_uint16_t fifo_samples = 170;

    adc_dma_init();

    // read cur_vol, vcap_vol, vbat_vol
    rt_uint16_t vcap_vol, vbat_vol;
    vcap_vol = vbat_vol = 0;

    if (wakeup_source_flag != RTC_SOURCE) {
        res = adxl372_init();
        log_debug("adxl372_init %s", res != RT_EOK ? "failed" : "success");
        res = adxl372_set_measure_config(&measure_val, &odr_val, &hpf_val, &fifo_format, &fifo_mode, &fifo_samples);
        log_debug(
            "adxl372_set_measure_config(measure_val=0x%02X, odr_val=0x%02X, hpf_val=0x%02X) %s",
            measure_val, odr_val, hpf_val, res != RT_EOK ? "failed" : "success"
        );
        res = adxl372_enable_inactive_irq(&milliscond, &threshold);
        log_debug(
            "adxl372_enable_inactive_irq(milliscond=%d, threshold=%d) %s",
            milliscond, threshold, res != RT_EOK ? "failed" : "success"
        );

        cur_vol_read_start();
        xyz_read_start_timestamp = (unsigned int)time(NULL);
        res = adxl372_read_fifo_xyz(&(sensor_data.x_buf), &(sensor_data.y_buf), &(sensor_data.z_buf), &(sensor_data.xyz_size));
        log_debug("adxl372_read_fifo_xyz res=%d, xyz_size=%d", res, sensor_data.xyz_size);
        cur_vol_read_stop();

        // 计算三轴数据峰值
        rt_int16_t abs_temp_value = 0;
        for (rt_uint16_t i = 0; i < sensor_data.xyz_size; i++)
        {
            // log_debug("X=%d, Y=%d, Z=%d", sensor_data.x_buf[i], sensor_data.y_buf[i], sensor_data.z_buf[i]);
            abs_temp_value = abs(sensor_data.x_buf[i]);  // x
            if (abs_temp_value > sensor_data.acc_x) {
                sensor_data.acc_x = abs_temp_value;
            }

            abs_temp_value = abs(sensor_data.y_buf[i]);  // y
            if (abs_temp_value > sensor_data.acc_y) {
                sensor_data.acc_y = abs_temp_value;
            }

            abs_temp_value = abs(sensor_data.z_buf[i]);  // z
            if (abs_temp_value > sensor_data.acc_z) {
                sensor_data.acc_z = abs_temp_value;
            }
        }

        rt_uint16_t *cur_buff;
        rt_uint16_t cur_buff_size = 0;
        res = cur_vol_read(&cur_buff, &cur_buff_size);
        log_debug(
            "cur_vol_read %s, cur_buff_size=%d, cur_buff=0x%08X",
            res == RT_EOK ? "success" : "failed", cur_buff_size, cur_buff
        );
        sensor_data.cur_buff = cur_buff;
        sensor_data.cur_buff_size = (int)cur_buff_size;
    }

    iic_dev = rt_i2c_bus_device_find("i2c1");

    // temp1, temp2
    float temp1 = 0.0;
    float temp2 = 0.0;
    rt_uint8_t addrs[2] = {TMP116_1_ADDR, TMP116_2_ADDR};

    res = temp116_read_temperture(iic_dev, addrs[0], &temp1);
    log_debug("temp116_read_temperture %s, temp3=%f", RT_EOK ? "failed" : "success", temp1);

    res = temp116_read_temperture(iic_dev, addrs[1], &temp2);
    log_debug("temp116_read_temperture %s, temp3=%f", res != RT_EOK ? "failed" : "success", temp2);

    if (res == RT_EOK) {
        sensor_data.temp1 = temp1;
        sensor_data.temp2 = temp2;
    }

    // temp3, humi
    float temp3 = 0.0;
    float humi = 0.0;
    res = hdc3021_read_temp_humi(iic_dev, &temp3, &humi);
    log_debug("hdc3021_read_temp_humi %s, temp3=%f, humi=%f", res != RT_EOK ? "failed" : "success", temp3, humi);
    if (res == RT_EOK) {
        sensor_data.temp3 = temp3;
        sensor_data.humi = humi;
    }

    // water level
    float value = 0.0;
    res = fdc1004_meas_data(iic_dev, &value);
    log_debug("fdc1004_meas_data %s, value=%f", res != RT_EOK ? "failed" : "success", value);
    if (res == RT_EOK) {
        sensor_data.water_level = value;
    }

    res = vcap_vol_read(&vcap_vol);
    log_debug("vcap_vol_read %s, vcap_vol=%d", res == RT_EOK ? "success" : "failed", vcap_vol);
    res = vbat_vol_read(&vbat_vol);
    log_debug("vbat_vol_read %s, vbat_vol=%d", res == RT_EOK ? "success" : "failed", vbat_vol);
    sensor_data.vcap_vol = (int)vcap_vol;
    sensor_data.vbat_vol = (int)vbat_vol;

    adc_dma_deinit();
    sensor_pwron_pin_enable(PIN_LOW);

    nmea_item nmea_item = {0};
    res = get_nmea_item_data(&nmea_item);
    if (res == RT_EOK) {
        log_debug("need report nmea data...");
        rt_memcpy(nmea_buf, nmea_item.GNGGA, rt_strlen(nmea_item.GNGGA));
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

    nbiot_lwm2m_deregister();
    // nbiot_config_mcu_version();
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
        if (nbiot_check_qiotstate(30) == RT_EOK) {
            return NBIOT_SERVER_CONNECT_RDY;
        }
        return NBIOT_SERVER_CONNECT_RETRY;
    }
    else {
        // connect failed and network not ready
        return NBIOT_NETWORK_RETRY;
    }
}

void get_random_number(char *output) {
    unsigned int tick = rt_tick_get();
    srand(rt_tick_get());

    int min = 48;
    int max = 122;
    int random_value;
    for (int i = 0; i < 10;) {
        random_value = min + rand() % ((max + 1) - min);
        if ((random_value <= 63 && random_value >= 58) || (random_value <= 96 && random_value >= 91)) {
            continue;
        }
        output[i] = random_value;
        i++;
    }
}

static int nbiot_report_ctrl_retry_times = 0;
static struct ServerCtrlData server_ctrl_data = {0};

enum nbiot_report_ctrl_data_status {
    NBIOT_REPORT_CTRL_DATA_SUCCESS,
    NBIOT_REPORT_CTRL_DATA_FAILED,
    NBIOT_REPORT_CTRL_DATA_RETRY
};

extern cJSON *read_json_obj_from_file(const char *file_path);
char nbiot_imei_string[16] = {0};
char ssid_string[64] = {0};
char pwd_string[64];
int nbiot_report_ctrl_data_to_server()
{
    if (nbiot_report_ctrl_retry_times >= 3) {
        log_debug("nbiot_report_ctrl_retry_times >= 3, goto report sensor data");
        return NBIOT_REPORT_CTRL_DATA_FAILED;
    }
    nbiot_report_ctrl_retry_times += 1;

    log_debug("nbiot report ctrl data to server");

    cJSON *data = cJSON_CreateObject();
    cJSON_AddNumberToObject(data, "12", settings_params->cat1_collect_interval);  // Cat1 Collect Interval
    cJSON_AddNumberToObject(data, "24", settings_params->nb_collect_interval);  // NB Collect Interval
    cJSON_AddNumberToObject(data, "4", get_current_antenna_no());  // Antennae No

    // WIFI_Config
    if (strlen(nbiot_imei_string) == 0) {
        if (get_nbiot_imei(nbiot_imei_string) != RT_EOK) {
            memset(nbiot_imei_string, 0, sizeof(nbiot_imei_string));
            memcpy(nbiot_imei_string, "123456", 6);
        }
        save_imei_to_file(nbiot_imei_string);
    }
    sprintf(ssid_string, "encortec-%s", nbiot_imei_string + 9);
    cJSON *wifi_config = cJSON_CreateObject();
    cJSON_AddStringToObject(wifi_config, "1", ssid_string);

    memset(pwd_string, 0, 64);
    get_random_number(pwd_string);
    cJSON_AddStringToObject(wifi_config, "2", pwd_string);

    cJSON_AddItemToObject(data, "3", wifi_config);

    // upload files
    cJSON *file_upload_event = read_json_obj_from_file("/upload_files.json");
    if (file_upload_event) {
        log_debug("no upload json files");
        cJSON_AddItemToObject(data, "22", file_upload_event);
        remove("/upload_files.json");
    }

    char *data_string = cJSON_PrintUnformatted(data);
    log_debug("data_string: %s", data_string);
    cJSON_Delete(data);

    rt_err_t result = nbiot_report_ctrl_data(data_string, rt_strlen(data_string));
    free(data_string);

    if (result == RT_EOK) {
        nbiot_recv_ctrl_data(256, &server_ctrl_data);  // read ctrl data from server
        log_debug("server_ctrl_data.Cat1_CollectInterval: %d", server_ctrl_data.Cat1_CollectInterval);
        log_debug("server_ctrl_data.NB_CollectInterval: %d", server_ctrl_data.NB_CollectInterval);
        log_debug("server_ctrl_data.Esp32_AP_Switch: %d", server_ctrl_data.Esp32_AP_Switch);
        log_debug("server_ctrl_data.Cat1_File_Upload_File_Times: %s", server_ctrl_data.Cat1_File_Upload_File_Times);
        log_debug("server_ctrl_data.Cat1_File_Upload_File_Type: %d", server_ctrl_data.Cat1_File_Upload_File_Type);
        log_debug("server_ctrl_data.Cat1_File_Upload_Switch: %d", server_ctrl_data.Cat1_File_Upload_Switch);
        log_debug("report ctrl data to server success, goto, report sensor data");
        settings_params_t p = {0};
        if (server_ctrl_data.Cat1_CollectInterval != 0) {
            p.cat1_collect_interval = server_ctrl_data.Cat1_CollectInterval;
            settings_update(&settings, &p);
        }
        if (server_ctrl_data.NB_CollectInterval != 0) {
            p.nb_collect_interval = server_ctrl_data.NB_CollectInterval;
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
    int rssi = 99;
    int ber = 99;

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

    cJSON *data = cJSON_CreateObject();
    cJSON_AddNumberToObject(data, "1", sensor_data.temp1);  // Rail temperature 1
    cJSON_AddNumberToObject(data, "5", sensor_data.temp2);  // Rail temperature 2
    cJSON_AddNumberToObject(data, "11", sensor_data.temp3);  // Air temperature
    cJSON_AddNumberToObject(data, "2", sensor_data.humi);  // Humidity
    cJSON_AddNumberToObject(data, "14", wakeup_source_flag);  // Wake up Source Flag
    
    if (wakeup_source_flag != RTC_SOURCE) {
        // Acceleration
        cJSON *acc = cJSON_CreateObject();
        cJSON_AddNumberToObject(acc, "1", sensor_data.acc_x);
        cJSON_AddNumberToObject(acc, "2", sensor_data.acc_y);
        cJSON_AddNumberToObject(acc, "3", sensor_data.acc_z);
        cJSON_AddItemToObject(data, "19", acc);

        cJSON_AddNumberToObject(data, "15", isinf(sensor_data.water_level) ? 0 : sensor_data.water_level);  // Water Level
        cJSON_AddNumberToObject(data, "16", sensor_data.cur_vol);  // Conversion voltage
        cJSON_AddNumberToObject(data, "17", sensor_data.vcap_vol);  // Capacitance voltage
        cJSON_AddNumberToObject(data, "18", sensor_data.vbat_vol);  // Battery voltage

        get_nbiot_csq(&rssi, &ber);
        cJSON_AddNumberToObject(data, "20", rssi);  // NB RSSI
    }

    char *data_string = cJSON_PrintUnformatted(data);
    rt_err_t result = nbiot_report_model_data(data_string, rt_strlen(data_string));
    cJSON_Delete(data);
    free(data_string);

    if (result == RT_EOK) {
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

int split_string_in_place(char *s)
{
    int lines = 0;
    while (*s != '\0') {
        if (*s == ',') {
            *s = '\0';
            lines++;
        }
        s++;
    }
    return lines + 1;
}

static int cat1_upload_file_retry_times = 0;
extern int at_https_upload_file(const char *filename);
extern void save_json_obj_to_file(const char *file_path, cJSON *root);
int cat1_upload_file()
{
    if (cat1_upload_file_retry_times >= 3) {
        log_debug("cat1_upload_file_retry_times >= 3, goto sleep");
        return CAT1_UPLOAD_FILE_ERROR;
    }
    cat1_upload_file_retry_times += 1;

    rt_err_t result = RT_EOK;

    char parent_dir[20] = {0};
    memset(parent_dir, 0, sizeof(parent_dir));
    if (server_ctrl_data.Cat1_File_Upload_File_Type == 1) {  // 数据文件
        strcat(parent_dir, "/data/");
        strcat(parent_dir, nbiot_imei_string);
    }
    else if (server_ctrl_data.Cat1_File_Upload_File_Type == 2) { // 系统文件
        strcat(parent_dir, "/log/");
        strcat(parent_dir, nbiot_imei_string);
    }
    else {

    }

    log_debug("cat1 upload file parent_dir dir: %s", parent_dir);
    int nums = split_string_in_place(server_ctrl_data.Cat1_File_Upload_File_Times);
    log_debug("got sub dir nums: %d", nums);

    char *sub_dir = server_ctrl_data.Cat1_File_Upload_File_Times;
    char target_dir[128] = {0};
    char upload_file_path[128] = {0};
    DIR *dir;
    struct dirent *ent;
    cJSON *root = cJSON_CreateObject();
    cJSON *array = cJSON_CreateArray();
    char upload_file_name[64] = {0};
    cJSON *temp_obj;

    if (at_https_open() != 0) {
        return CAT1_UPLOAD_FILE;  // try again
    }

    for (int i=1; i <= nums; i++) {
        log_debug("got sub_dir %d: %s", i, sub_dir);
        if (!strlen(sub_dir)) {
            continue;
        }

        memset(target_dir, 0, sizeof(target_dir));
        sprintf(target_dir, "%s/%s", parent_dir, sub_dir);
        log_debug("got target dir: %s", target_dir);

        // 遍历文件 for test
        // list_files(target_dir);
        log_debug("----- uploading dir \"%s\" files: ------", target_dir);
        if ((dir = opendir(target_dir)) != NULL) {
            while ((ent = readdir(dir)) != NULL) {
                memset(upload_file_path, 0, sizeof(upload_file_path));
                sprintf(upload_file_path, "%s/%s", target_dir, ent->d_name);
                log_debug("uploading file \"%s\", %d Bytes", upload_file_path, get_file_size(upload_file_path));

                wdg_feed_soft(wdg_id);
                if (at_https_upload_file(upload_file_path) == -1) {
                    break;
                    log_debug("at https upload file failed");
                }
                else {
                    log_debug("at https upload \"%s\" success", upload_file_path);
                    remove(upload_file_path);
                    memset(upload_file_name, 0, sizeof(upload_file_name));
                    sprintf(upload_file_name, "%s/%s", AWS_BUCKET, ent->d_name);
        
                    temp_obj = cJSON_CreateObject();
                    cJSON_AddStringToObject(temp_obj, "1", sub_dir);
                    cJSON_AddStringToObject(temp_obj, "2", upload_file_name);
                    cJSON_AddNumberToObject(temp_obj, "3", server_ctrl_data.Cat1_File_Upload_File_Type);
                    cJSON_AddItemToArray(array, temp_obj);
                }
            }
            closedir(dir);
        }
        log_debug("----- uploading dir \"%s\" completed ------", target_dir);

        sub_dir += (strlen(sub_dir) + 1);
    }

    at_https_close();

    if (cJSON_GetArraySize(array) > 0) {
        cJSON_AddItemToObject(root, "23", array);
        save_json_obj_to_file("/upload_files.json", root);
    }
    cJSON_Delete(root);

    return CAT1_UPLOAD_FILE_SUCCESS;
}

static int DEFAULT_REMAINING_SECONDS = 600;
void stm32_sleep()
{   
    // log_debug("wait stm32_sleep_ack_sem release");
    // rt_sem_take(stm32_sleep_ack_sem, rt_tick_from_millisecond(1000));
    log_debug("go into sleep");

    end_tick_ms = rt_tick_get_millisecond();
    log_debug("start tick ms: %d", start_tick_ms);
    log_debug("end tick ms: %d", end_tick_ms);
    log_debug("settings_params->nb_collect_interval: %d", settings_params->nb_collect_interval);

    int remaining = 0;

    // 获取当前时间
    time_t now = time(NULL);
    struct tm *timeinfo = localtime(&now);
    log_debug("stm32_sleep got cur time: %04d-%02d-%02d %02d:%02d:%02d", timeinfo->tm_year + 1900, timeinfo->tm_mon, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
    // 2024-10-29 06:50:36
    if ((timeinfo->tm_year + 1900 != 2000) && (timeinfo->tm_hour >= 0 && timeinfo->tm_hour < 6)) {
        // 时间戳正确 且 在0点至6点区间, 不上报
        remaining = (6 - timeinfo->tm_hour) * 3600 - timeinfo->tm_min * 60 - timeinfo->tm_sec;
        rt_kprintf("cannot wakeup rtc between [0~6]\n");
    }
    else {
        remaining = settings_params->nb_collect_interval - ((end_tick_ms - start_tick_ms) / 1000);
        rt_kprintf("calculate remaining with end and start timestamp\n");
    }
    log_debug("remaining: %d", remaining);

    rtc_set_wakeup(remaining > 0 ? remaining : 10);
    shut_down();
}

struct FileFormatData
{
    char separator;
    int year;
    int month;
    int day;
    int hour;
    int minute;
    float lat;
    float lng;
    int zone;
    float temp1;
    float temp2;
    float temp3;
    float humi;
    float water_level;
    int vcap_vol;
    int vbat_vol;
};

void save_sensor_data()
{
    int length = 0;
    char temp_buf[128] = {0};
    log_debug("save sensor data");

    struct FileFormatData data = {0};

    time_t rawtime;
    time(&rawtime);
    rt_kprintf("save_sensor_data get raw time: %d\n", rawtime);
    struct tm *utc_time = gmtime(&rawtime);

    data.separator = wakeup_source_flag == RTC_SOURCE ? 0xaa : 0xff;
    data.year = utc_time->tm_year+1900;
    data.month = utc_time->tm_mon+1;
    data.day = utc_time->tm_mday;
    data.hour = utc_time->tm_hour;
    data.minute = utc_time->tm_min;
    data.lat = sensor_data.lat;
    data.lng = sensor_data.lng;
    data.zone = 0;
    data.temp1 = sensor_data.temp1;
    data.temp2 = sensor_data.temp2;
    data.temp3 = sensor_data.temp3;
    data.humi = sensor_data.humi;
    data.water_level = sensor_data.water_level;
    data.vcap_vol = sensor_data.vcap_vol;
    data.vbat_vol = sensor_data.vbat_vol;

    data_save_as_file(&fs, (char *)&data, sizeof(data), true, false);
    log_debug("write data file size: %d", sizeof(data));

    if (wakeup_source_flag != RTC_SOURCE) {
        unsigned int sample_size = (unsigned int)(sensor_data.xyz_size);
        data_save_as_file(&fs, (char *)&sample_size, sizeof(sample_size), true, true);
        log_debug("write sample_size: %d", sample_size);

        data_save_as_file(&fs, (char *)&xyz_read_start_timestamp, sizeof(xyz_read_start_timestamp), true, true);
        log_debug("write xyz_read_start_timestamp: %d", xyz_read_start_timestamp);

        data_save_as_file(&fs, (char *)(sensor_data.x_buf), sensor_data.xyz_size * 2, true, true);
        data_save_as_file(&fs, (char *)(sensor_data.y_buf), sensor_data.xyz_size * 2, true, true);
        data_save_as_file(&fs, (char *)(sensor_data.z_buf), sensor_data.xyz_size * 2, true, true);

        data_save_as_file(&fs, (char *)(&(sensor_data.cur_buff_size)), sizeof(sensor_data.cur_buff_size), true, true);
        log_debug("write sensor_data.cur_buff_size: %d", sensor_data.cur_buff_size);
        data_save_as_file(&fs, (char *)(sensor_data.cur_buff), sensor_data.cur_buff_size * 2, true, true);
    }

    // for test
    // char temp[32] = {0};
    // rt_kprintf("\n=====================================\n");
    // rt_kprintf("Separator: %d\n", data.separator);
    // rt_kprintf("YYYY: %d\n", data.year);
    // rt_kprintf("MM: %d\n", data.month);
    // rt_kprintf("DD: %d\n", data.day);
    // rt_kprintf("hh: %d\n", data.hour);
    // rt_kprintf("mm: %d\n", data.minute);

    // sprintf(temp, "%f", data.lat);
    // rt_kprintf("N: %s\n", temp);

    // rt_memset(temp, 0, 32);
    // sprintf(temp, "%f", data.lng);
    // rt_kprintf("E: %s\n", temp);

    // rt_kprintf("Z: %d\n", data.zone);

    // rt_memset(temp, 0, 32);
    // sprintf(temp, "%f", data.temp1);
    // rt_kprintf("TEMP1: %s\n", temp);

    // rt_memset(temp, 0, 32);
    // sprintf(temp, "%f", data.temp2);
    // rt_kprintf("TEMP2: %s\n", temp);

    // rt_memset(temp, 0, 32);
    // sprintf(temp, "%f", data.temp3);
    // rt_kprintf("HUM_TEMP: %s\n", temp);
    
    // rt_memset(temp, 0, 32);
    // sprintf(temp, "%f", data.humi);
    // rt_kprintf("HUMI: %s\n", temp);
    
    // rt_memset(temp, 0, 32);
    // sprintf(temp, "%f", data.water_level);
    // rt_kprintf("Water: %s\n", temp);

    // rt_kprintf("Capacitor_V: %d\n", data.vcap_vol);
    // rt_kprintf("Bat_V: %d\n", data.vbat_vol);
    // rt_kprintf("Sample_Size: %d\n", sensor_data.xyz_size);
    // rt_kprintf("Start_Timestamp: %u\n", xyz_read_start_timestamp);

    // int i = 0;
    // if (sensor_data.xyz_size > 0) {
    //     rt_kprintf("X samples: ");
    //     for (i = 0; i < sensor_data.xyz_size; i++) {
    //         rt_kprintf("%d,", sensor_data.x_buf[i]);
    //     }
    //     rt_kprintf("\n");
    //     rt_kprintf("Y samples: ");
    //     for (i = 0; i < sensor_data.xyz_size; i++) {
    //         rt_kprintf("%d,", sensor_data.y_buf[i]);
    //     }
    //     rt_kprintf("\n");
    //     rt_kprintf("Z samples: ");
    //     for (i = 0; i < sensor_data.xyz_size; i++) {
    //         rt_kprintf("%d,", sensor_data.z_buf[i]);
    //     }
    //     rt_kprintf("\n");
    // }
    // rt_kprintf("Sample_Size2: %d\n", sensor_data.cur_buff_size);
    // if (sensor_data.cur_buff_size > 0) {
    //     rt_kprintf("Track return voltages: ");
    //     for (i = 0; i < sensor_data.cur_buff_size; i++) {
    //             rt_kprintf("%d,", sensor_data.cur_buff[i]);
    //     }
    //     rt_kprintf("\n");
    // }
    // rt_kprintf("\n=====================================\n");

    // 删除超过30天的文件
    delete_old_dirs(&fs);
}


int is_cat1_upload_every_30_days()
{
    FILE *fd = NULL;
    time_t cur_timestamp = 0;
    time_t prev_timestamp = 0;
    char cat1_uploadtime_filename[] = "/prev_cat1_uploadtime.bin";

    // 判断上次读取的时间
    time(&cur_timestamp);
    log_debug("is_cat1_upload_every_30_days cur_timestamp: %d\n", cur_timestamp);

    fd = fopen(cat1_uploadtime_filename, "rb");
    if (fd) {
        log_debug("is_cat1_upload_every_30_days timstamp file exists.");
        fread(&prev_timestamp, 1, sizeof(prev_timestamp), fd);
        log_debug("is_cat1_upload_every_30_days prev_timestamp: %d\n", prev_timestamp);
        if (cur_timestamp - prev_timestamp < 30 * 24 * 3600) {
            // 距上次上报数据未满30天
            log_debug("is_cat1_upload_every_30_days prev_timestamp - cur_timestamp < 30 days");
            fclose(fd);
            fd = NULL;
            return 0;
        }
        fclose(fd);
        fd = NULL;
    }

    log_debug("is_cat1_upload_every_30_days file not exists or over 30 days, upload file soon.");

    // 设置cat1上报当月的所有数据参数
    DIR *dir;
    struct dirent *ent;

    // 获取文件路径
    if ((dir = opendir(fs.base)) != NULL) {
        if (strlen(server_ctrl_data.Cat1_File_Upload_File_Times) > 0) {
            strcat(server_ctrl_data.Cat1_File_Upload_File_Times, ",");
        }
        while ((ent = readdir(dir)) != NULL) {
            strcat(server_ctrl_data.Cat1_File_Upload_File_Times, ent->d_name);
            strcat(server_ctrl_data.Cat1_File_Upload_File_Times, ",");
            log_debug("is_cat1_upload_every_30_days append dir: %s", ent->d_name);
        }
        if (server_ctrl_data.Cat1_File_Upload_File_Times[strlen(server_ctrl_data.Cat1_File_Upload_File_Times) - 1] == ',') {
            server_ctrl_data.Cat1_File_Upload_File_Times[strlen(server_ctrl_data.Cat1_File_Upload_File_Times) - 1] = '\0';
        }
    }
    else {
        return 0;
    }

    server_ctrl_data.Cat1_File_Upload_File_Type = 1;
    log_debug("is_cat1_upload_every_30_days File Times: %s", server_ctrl_data.Cat1_File_Upload_File_Times);


    // 记录当前上报的时间
    fd = fopen(cat1_uploadtime_filename, "wb");
    if (fd) {
        fwrite(&cur_timestamp, 1, sizeof(cur_timestamp), fd);
        log_debug("is_cat1_upload_every_30_days write cur_timestamp to file: %d", cur_timestamp);
        fclose(fd);
    }

    return 1;
}


int should_cat1_upload_files()
{
    if (is_cat1_upload_every_30_days()) {
        return 1;
    }

    if ((server_ctrl_data.Cat1_File_Upload_File_Type == 1 || server_ctrl_data.Cat1_File_Upload_File_Type == 2) \
        && strlen(server_ctrl_data.Cat1_File_Upload_File_Times) > 0) {
            return 1;
    }
    return 0;
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
}

extern rt_err_t esp32_wifi_transfer();

static rt_device_t wdg_device = RT_NULL;

static void feed_rtwdg(void)
{
    rt_err_t res = rt_device_control(wdg_device, RT_DEVICE_CTRL_WDT_KEEPALIVE, RT_NULL);
    // log_debug("RT_DEVICE_CTRL_WDT_KEEPALIVE %d", res);
}

void main_business_entry(void)
{
    rt_err_t result;
    int status;
    int rv = 0;
    read_imei_from_file(nbiot_imei_string, 15);
    log_debug("read_imei_from_file: %s", nbiot_imei_string);

    if (settings_init(&settings, "/settings.conf", NULL) == 0) {
        settings_params = settings_read(&settings);
        if (!settings_params) {
            settings_params = &settings.params;
        }
        if (settings_params->cat1_collect_interval == 0) {
            settings_params->cat1_collect_interval = 600;
        }
        if (settings_params->nb_collect_interval == 0) {
            settings_params->nb_collect_interval = 900;
        }

        log_info("settings_params->cat1_collect_interval: %d", settings_params->cat1_collect_interval);
        log_info("settings_params->nb_collect_interval: %d", settings_params->nb_collect_interval);
    }
 
    // stm32_sleep_ack_sem = rt_sem_create("stm32_sleep_ack_sem", 0, RT_IPC_FLAG_FIFO);
    rtc_init();

    start_tick_ms = rt_tick_get_millisecond();
    log_debug("start tick ms: %d", start_tick_ms);

    sm_init();

    char temp_base[32] = {0};
    if (strlen(nbiot_imei_string)) {
        sprintf(temp_base, "/data/%s", nbiot_imei_string);
        data_save_as_file_init(&fs, 0, ".dat", temp_base, -1);
    }
    else {
        data_save_as_file_init(&fs, 0, ".dat", "/data", -1);
    }

    board_pins_init();

    debug_led1_pin_init();

    // setup watch dog
    wdg_device = rt_device_find("wdt");
    wdg_init(100, feed_rtwdg);
    rt_device_control(wdg_device, RT_DEVICE_CTRL_WDT_START, RT_NULL);

    wdg_create_soft(&wdg_id, 300*1000, BLOCK_TYPE_NO_BLOCK, RT_NULL);
    
    while (1)
    {
        status = sm_get_status();
        wdg_feed_soft(wdg_id);

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
                save_sensor_data();
                if (reset_source_flag) {
                    debug_led1_off();
                }
                sm_set_status(CHECK_ANTENNA_SIGNAL_STRENGTH);
                break;
            case CHECK_ANTENNA_SIGNAL_STRENGTH:
                // 首次上电检查主副天线信号强度
                if (reset_source_flag || get_antenna_from_file() == RT_ERROR) {
                    check_antenna_signal_strength(reset_source_flag);
                }
                sm_set_status(NBIOT_INIT);
                break;
            case NBIOT_INIT:
                debug_led1_start_flash(500);
                nbiot_init();
                sm_set_status(NBIOT_WAIT_NETWORK_RDY);
                break;
            case NBIOT_WAIT_NETWORK_RDY:
                rv = nbiot_wait_network_ready();
                if (rv == NBIOT_NETWORK_NOT_RDY) {
                    nbiot_deinit();
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
                    debug_led1_stop_flash();
                    if (should_cat1_upload_files()) {
                        sm_set_status(CAT1_INIT);
                    }
                    else {
                        sm_set_status(ESP32_WIFI_TRANSFER_DATA);
                    }
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
                    sm_set_status(ESP32_WIFI_TRANSFER_DATA);
                }
                else {
                    sm_set_status(CAT1_WAIT_NETWORK_RDY);
                }
                break;
            case CAT1_WAIT_NETWORK_RDY:
                rv = cat1_wait_network_ready();
                if (rv == CAT1_NETWORK_NOT_RDY) {
                    cat1_set_cfun_mode(0);
                    log_debug("cat1_set_cfun_mode 0");
                    rt_thread_mdelay(200);
                    cat1_set_cfun_mode(1);
                    log_debug("cat1_set_cfun_mode 1");
                    sm_set_status(CAT1_WAIT_NETWORK_RDY);
                }
                else if (rv == CAT1_NETWORK_RDY) {
                    sm_set_status(CAT1_UPLOAD_FILE);
                }
                else if (rv == CAT1_NETWORK_ERROR) {
                    cat1_deinit();
                    sm_set_status(ESP32_WIFI_TRANSFER_DATA);
                }
                break;
            case CAT1_UPLOAD_FILE:
                rv = cat1_upload_file();
                if (rv == CAT1_UPLOAD_FILE_SUCCESS || rv == CAT1_UPLOAD_FILE_ERROR) {
                    cat1_deinit();
                    sm_set_status(ESP32_WIFI_TRANSFER_DATA);
                }
                else {
                    sm_set_status(CAT1_UPLOAD_FILE);
                }
                break;
            case ESP32_WIFI_TRANSFER_DATA:
                esp32_wifi_transfer();
                sm_set_status(SLEEP);
                break;
            case SLEEP:
                debug_led1_stop_flash();
                stm32_sleep();
                break;
            default:
                log_warn("unknown status: %d", status);
                break;
        }
    }
}

extern void nand_to_esp32(void);
rt_err_t esp32_wifi_transfer()
{
    rt_err_t result = RT_EOK;
    // server_ctrl_data.Esp32_AP_Switch = 1; // for test
    if (server_ctrl_data.Esp32_AP_Switch) {
        debug_led1_start_flash(250);

        nand_to_esp32();
        esp_at_init();
        esp32_power_pin_init();
        esp32_power_on();

        if (esp_wait_rdy() != 0) {
            log_debug("can not wait esp32 ready");
            return RT_ERROR;
        }
        log_debug("esp wait rdy");

        result = esp32_transf_data(
            ssid_string, strlen(ssid_string),
            pwd_string, strlen(pwd_string),
            nbiot_imei_string, strlen(nbiot_imei_string)
        );
        // result = esp32_transf_data(
        //     "ESP_TEST", strlen("ESP_TEST"),
        //     pwd_string, strlen(pwd_string),
        //     "THIS IS DT TEST", strlen("THIS IS DT TEST")
        // );
        if (result != RT_EOK) {
            log_debug("esp32_transf_data error");
            debug_led1_stop_flash();
            return result;
        }
        
        rt_uint32_t event;
        for (int i=0; i<300; i++) {
            rt_thread_mdelay(1000);
            if (esp_recv_event(&event) != RT_EOK) {
                continue;
            }
            if (event & (STA_CONNECT_EVENT | AP_STARTED_EVENT | DT_STARTED_EVENT)) {
                i = 0;  // 重新计时
            }
            if (event & (STA_DISCONNECT_EVENT | AP_STOPPED_EVENT | DT_ERROR_EVENT | DT_SUCCESS_EVENT | DT_CLOSE_EVENT | DT_NO_CONN_LONG_TIME)) {
                break;
            }
        }

        debug_led1_stop_flash();
        esp32_power_off();
    }

    return RT_EOK;
}

cJSON *read_json_obj_from_file(const char *file_path)
{
    FILE *fd = NULL;
    int length = 0;

    fd = fopen(file_path, "rb");
    if (!fd) {
        log_debug("open file \"%s\" failed", file_path);
        return NULL;
    }
    
    fseek(fd, 0, SEEK_END);
    length = ftell(fd);
    fseek(fd, 0, SEEK_SET);
    log_debug("got \"%s\" file length: %d", file_path, length);

    char *json_string = (char *)malloc(length + 1);
    memset(json_string, 0, length + 1);
    fread(json_string, 1, length, fd);
    log_debug("read json string: %s\n", json_string);
    fclose(fd);

    cJSON *root = cJSON_Parse(json_string);

    free(json_string);
    return root;
}

void save_json_obj_to_file(const char *file_path, cJSON *root)
{
    FILE *fd = NULL;
    char *data = NULL;
    fd = fopen(file_path, "wb");
    if (fd) {
        data = cJSON_PrintUnformatted(root);
        fwrite(data, 1, strlen(data), fd);
        fclose(fd);
        free(data);
    }
}
