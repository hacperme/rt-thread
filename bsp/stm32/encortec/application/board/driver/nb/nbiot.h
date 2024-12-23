#ifndef __NBIOT_H__
#define __NBIOT_H__

#include <rtthread.h>
#include <at.h>

#define NBIOT_AT_UART_NAME "uart3"

struct network_config {
    char *apn;
    char *band;
    char *cipca;
};
typedef struct network_config *network_config_t;

struct lwm2m_config {
    char *pk;
    char *ps;
    int server_type;
    char *server_URL;
    int lifetime;
    int buffer_mode;
    int context_id;
    int tsl_mode;
};
typedef struct lwm2m_config *lwm2m_config_t;

struct datetime {
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
    int zone;
};
typedef struct datetime *datetime_t;

struct ServerCtrlData {
    int Cat1_CollectInterval;
    int NB_CollectInterval;
    int Esp32_AP_Switch;
    char Cat1_File_Upload_File_Times[512];
    int Cat1_File_Upload_File_Type;
    int Cat1_File_Upload_Switch;
};

/* init AT client */
rt_err_t nbiot_at_client_init(void);

/* enable and disable NBIOT sleep mode*/
rt_err_t nbiot_disable_sleep_mode();
rt_err_t nbiot_enable_sleep_mode();

/* make AT echo mode*/
rt_err_t nbiot_enable_echo(int enable);

/* make cfun switch */
rt_err_t nbiot_set_cfun_mode(int mode);

/* check and set network configure*/
rt_err_t nbiot_check_network(int retry_times);
rt_err_t nbiot_set_network_config(network_config_t config);

/* check and set lwm2m configure*/
rt_err_t nbiot_check_lwm2m_config(lwm2m_config_t config);
rt_err_t nbiot_set_lwm2m_config(lwm2m_config_t config);

/* register or deregister to lwm2m2 */
rt_err_t nbiot_lwm2m_register();
rt_err_t nbiot_lwm2m_deregister();

/*check lwm2m server register state */
rt_err_t nbiot_check_qiotstate();

/* send report model data*/
rt_err_t nbiot_report_model_data(const char *data, rt_size_t length);
rt_err_t nbiot_report_ctrl_data(const char *data, rt_size_t length);
rt_err_t nbiot_recv_ctrl_data(int req_length, struct ServerCtrlData *server_ctrl_data);

/* get current datetime from NBIOT */
rt_err_t nbiot_get_current_datetime(datetime_t dt);

rt_err_t nbiot_set_qiotlocext(char *nmea_string);

rt_err_t get_nbiot_imei(char *output);

/* report mcu version to cloud */
rt_err_t nbiot_config_mcu_version(void);

rt_err_t get_nbiot_csq(int *rssi, int *ber);

#endif
