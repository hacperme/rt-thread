#ifndef __RT_API_H__
#define __RT_API_H__

#include "rttypes.h"
#include "rtthread.h"

typedef rt_err_t(*rt_thread_mdelay_api_ptr_t)(rt_int32_t ms);
typedef int(*rt_kprintf_api_ptr_t)(const char *fmt, ...);
typedef void *(*rt_malloc_api_ptr_t)(rt_size_t size);
typedef void(*rt_free_api_ptr_t)(void *ptr);
typedef void *(*rt_realloc_api_ptr_t)(void *ptr, rt_size_t newsize);
typedef void *(*rt_calloc_api_ptr_t)(rt_size_t count, rt_size_t size);

#endif
