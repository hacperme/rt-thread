#ifndef __HDL_DEBUG_H__
#define __HDL_DEBUG_H__

#include "hdl_config.h"

#ifdef __cplusplus
extern "C" {
#endif

// ToDo Porting: Please use your platform API to implement it.

#ifdef HDL_DEBUG
// #define HDL_LOGI(fmt,arg...)   LOG_I(HostDL, "[HostDL]: "fmt,##arg)
#define HDL_LOGI(fmt,arg...)   log_info("[HostDL]: "fmt,##arg)
#else
#define HDL_LOGI(fmt,arg...)
#endif

// #define HDL_LOGE(fmt,arg...)   LOG_E(HostDL, "[HostDL]: "fmt,##arg)
#define HDL_LOGE(fmt,arg...)   log_error("[HostDL]: "fmt,##arg)
// #define HDL_SUCCESS_LOG(X)     LOG_I(HostDL, "%s %s", X, ((success) ? "success" : "fail"))
#define HDL_SUCCESS_LOG(X)     log_info("%s %s", X, ((success) ? "success" : "fail"))

#ifndef HDL_Require_Noerr_Action
#define HDL_Require_Noerr_Action(X, GOTO_LABEL, Fun)                                \
    do {                                                                            \
        if(!(X)) {                                                                  \
            { HDL_LOGE("%s fail", Fun); }                                           \
            goto GOTO_LABEL;                                                        \
        }                                                                           \
    } while(0)
#endif

#ifndef HDL_MAIN_LOG
#define HDL_MAIN_LOG(fmt,arg...)                                                    \
    do                                                                              \
    {                                                                               \
        log_info("##################################################");             \
        log_info(fmt, ##arg);                                                       \
        log_info("##################################################");             \
    } while(0)
#endif

#ifdef __cplusplus
}
#endif

#endif
