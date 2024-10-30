#ifndef __HDL_API_H__
#define __HDL_API_H__

#include "hdl_brom_base.h"
#include "hdl_da_cmd.h"

#ifdef __cplusplus
extern "C" {
#endif

bool hdl_connect(const hdl_da_info_t* da_info, const hdl_connect_arg_t* connect_arg, hdl_da_report_t* da_report);
bool hdl_format(const hdl_format_arg_t* format_arg, const hdl_da_report_t* da_report);
bool hdl_download(const hdl_download_arg_t* download_arg, const hdl_da_report_t* da_report);
bool hdl_readback(const hdl_readback_arg_t* readback_arg, const hdl_da_report_t* da_report);
bool hdl_disconnect_auto_reboot(bool enable);

#ifdef __cplusplus
}
#endif

#endif //__HDL_API_H__

