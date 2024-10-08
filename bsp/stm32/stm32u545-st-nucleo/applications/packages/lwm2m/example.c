#define ENABLE_EXAMPLE
#ifdef ENABLE_EXAMPLE


#include "lwm2m.h"


extern rt_err_t atcmd_send(const char *dev_name, at_response_t resp, const char *at_string);


rt_err_t atcmd_qsclk(const char *dev_name, at_response_t resp)
{
    // AT+QSCLK=0
    return atcmd_send(dev_name, resp, "AT+QSCLK=0");
}


rt_err_t atcmd_qidnscfg(const char *dev_name, at_response_t resp)
{
    // AT+QIDNSCFG=0,8.8.8.8,2402:4e00::
    return atcmd_send(dev_name, resp, "AT+QIDNSCFG=0,8.8.8.8,2402:4e00::");
}


/* just for rt msh test */
int nbiot_at_client_send(int argc, char **argv)
{
    rt_err_t err = RT_EOK;

    if (argc != 2) {
        LOG_E("at_cli_send [command]  - AT client send commands to AT server.\n");
        return -RT_ERROR;
    }

    at_response_t resp = RT_NULL;
    resp = at_create_resp(512, 0, rt_tick_from_millisecond(AT_WAIT_RESP_MAX_MILLISECOND));
    if (!resp) {
        LOG_E("No memory for response structure!\n");
        return -RT_ENOMEM;
    }

    const char *cmd = argv[1];
    if (!rt_strcmp(cmd, "atcmd_qlwconfig")) {
        // AT+QLWCONFIG=0,"leshan.eclipseprojects.io",5684,"Encortec",900,0,"863663063984075","000102030405060708090a0b0c0d0e0f"
        err = atcmd_qlwconfig(
            AT_NBIOT_DEVICE_NAME,
            resp,
            0,
            "23.97.187.154", // 不需要DNS
            5684,
            "Encortec",
            86400,
            0,
            "863663063984075",
            "000102030405060708090a0b0c0d0e0f"
        );
    }
    else if (!rt_strcmp(cmd, "atcmd_qlwreg")) {
        err = atcmd_qlwreg(AT_NBIOT_DEVICE_NAME, resp);
    }
    else if (!rt_strcmp(cmd, "atcmd_qlwdereg")) {
        err = atcmd_qlwdereg(AT_NBIOT_DEVICE_NAME, resp);
    }
    else if (!rt_strcmp(cmd, "atcmd_qlwupdate")) {
        // AT+QLWUPDATE=0,86400
        err = atcmd_qlwupdate(AT_NBIOT_DEVICE_NAME, resp, 0, 86400, 0);
    }
    else if (!rt_strcmp(cmd, "atcmd_qlwaddobj")) {
        // AT+QLWADDOBJ=3304,0,4,5700,5701,5603,5604
        int resource_ids[4] = {5700, 5701, 5603, 5604};
        err = atcmd_qlwaddobj(AT_NBIOT_DEVICE_NAME, resp, 3304, 0, 4, resource_ids);
    }
    else if (!rt_strcmp(cmd, "atcmd_qlwdelobj")) {
        // AT+QLWDELOBJ=17
        err = atcmd_qlwdelobj(AT_NBIOT_DEVICE_NAME, resp, 17);
    }
    else if (!rt_strcmp(cmd, "atcmd_qlwrdrsp")) {
        // AT+QLWRDRSP=62953,1,9,0,0,1,5,"abcde",0
        err = atcmd_qlwrdrsp(AT_NBIOT_DEVICE_NAME, resp, 62953, 1, 9, 0, 0, 1, 5, "abcde", 0);
    }
    else if (!rt_strcmp(cmd, "atcmd_qlwwrrsp")) {
        // AT+QLWWRRSP=36560,2
        err = atcmd_qlwwrrsp(AT_NBIOT_DEVICE_NAME, resp, 36560, 2);
    }
    else if (!rt_strcmp(cmd, "atcmd_qlwexeresp")) {
        // AT+QLWEXERSP=39040,2
        err = atcmd_qlwexeresp(AT_NBIOT_DEVICE_NAME, resp, 39040, 2);
    }
    else if (!rt_strcmp(cmd, "atcmd_qlwobsrsp")) {
        // AT+QLWOBSRSP=624,1,9,0,0,1,5,"abcde",0
        err = atcmd_qlwobsrsp(AT_NBIOT_DEVICE_NAME, resp, 624, 1, 9, 0, 0, 1, 5, "abcde", 0);
    }
    else if (!rt_strcmp(cmd, "atcmd_qlwnotify")) {
        // AT+QLWNOTIFY=3304,1,5700,4,4,33.3,0,1,0
        err = atcmd_qlwnotify(AT_NBIOT_DEVICE_NAME, resp, 3304, 1, 5700, 4, 4, "33.3", 0, 1, 0);
    }
    else if (!rt_strcmp(cmd, "atcmd_qlwstatus")) {
        // AT+QLWSTATUS?
        err = atcmd_qlwstatus(AT_NBIOT_DEVICE_NAME, resp);
    }
    else if (!rt_strcmp(cmd, "atcmd_qlwrecover")) {
        // AT+QLWRECOVER
        err = atcmd_qlwrecover(AT_NBIOT_DEVICE_NAME, resp);
    }
    else if (!rt_strcmp(cmd, "atcmd_qidnscfg")) {
        err = atcmd_qidnscfg(AT_NBIOT_DEVICE_NAME, resp);
    }
    else if (!rt_strcmp(cmd, "atcmd_qsclk")) {
        // AT+QSCLK=0
        err = atcmd_qsclk(AT_NBIOT_DEVICE_NAME, resp);
    }
    else {
        LOG_E("atcmd_send unknown cmd: %s\n", cmd);
        err = RT_ERROR;
    }

    LOG_D("atcmd_send err: %d\n", err);
    char *line_ptr = resp->buf;
    for (int i=1; i <= resp->line_counts; i++) {
        LOG_D("recv line %d: %s", i, line_ptr);
        line_ptr += rt_strlen(line_ptr) + 1;
    }


    at_delete_resp(resp);

    return err;
}


/* just for rt msh test*/
MSH_CMD_EXPORT(nbiot_at_client_init, AT Client init);
MSH_CMD_EXPORT(nbiot_at_client_send, AT Client send commands to AT Server and get response data);

#endif
