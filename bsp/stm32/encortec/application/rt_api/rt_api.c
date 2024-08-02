
#include "common.h"
#include <stdarg.h>
#include "rt_api_addr.h"
#include "rttypes.h"
#include <stddef.h>


typedef int(*rt_kprintf_api_ptr_t)(const char *fmt, ...);
int rt_kprintf(const char *fmt, ...) {
        va_list args;
        int len;

        va_start(args, fmt);
        len = ((rt_kprintf_api_ptr_t)(rt_kprintf_addr))(fmt, args);
        va_end(args);

        return len;
}


typedef rt_err_t(*rt_thread_mdelay_api_ptr_t)(rt_int32_t ms);
rt_err_t rt_thread_mdelay(rt_int32_t ms) {
    return ((rt_thread_mdelay_api_ptr_t)(rt_thread_mdelay_addr))(ms);
}
