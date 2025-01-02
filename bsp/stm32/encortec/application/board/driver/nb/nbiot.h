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

enum
{
    QIOT_OTA_TASK_NOTIFY = 10700, /* There is an upgrade task */
    QIOT_OTA_START = 10701,       /* the device starts downloading firmware package. */
    QIOT_OTA_DOWNLOADING = 10702, /* Firmware is downloaded in progress. */
    QIOT_OTA_DOWNLOADED = 10703,  /* the firmware package is downloaded. */
    QIOT_OTA_UPDATING = 10704,    /* Firmware is being upgraded. */
    QIOT_OTA_UPDATE_OK = 10705,   /* Firmware is upgraded successfully. */
    QIOT_OTA_UPDATE_FAIL = 10706, /* Failed to upgrade the firmware. */
    QIOT_OTA_UPDATE_FLAG = 10707, /* Advertisement of the first device operation result */
};

typedef enum
{
    OTA_TASK_STATE_NONE = 0,
    OTA_TASK_STATE_RECV,
    OTA_TASK_STATE_DOWNLOADING,
    OTA_TASK_STATE_DOWNLOADED,
    OTA_TASK_STATE_UPGRADEING,
    OTA_TASK_STATE_FINISH
}ota_task_state_t;


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
/* query if there is an upgrade task */
rt_err_t nbiot_ota_req(void);
/**
 * @brief 
 * 
 * @param [in] action 
 * 0: Reject upgrade
 * 1: Confirm upgrade
 * 2: MCU requests to download the next firmware block,
 * 3: MCU reports updating status
 * @return rt_err_t 
 */
rt_err_t nbiot_ota_update_action(int action);


ota_task_state_t nbiot_get_ota_task_state(void);

/**
 * @brief 
 * 
 * @return int 
 *  -- 0  ALL firmware download completed
 *  -- 1  Currrent firmware download completed, neet to dowload the next firmware component
 *  -- 2  Firmware fragment download completed, need to request the next firmware fragment 
 *  -- 3  Need to continue reading data
 *  -- -1 save data error 
 */
int nbiot_save_ota_data(void);

void nbiot_clean_ota_task(void);

rt_err_t get_nbiot_csq(int *rssi, int *ber);

rt_err_t check_and_set_macrai();

rt_err_t check_and_set_psms();

#endif
