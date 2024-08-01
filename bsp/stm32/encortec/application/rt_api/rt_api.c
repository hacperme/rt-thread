
#include "common.h"
#include <stdarg.h>
#include "rt_api.h"
#include <stddef.h>

static rt_kprintf_api_ptr_t rt_kprintf_api_ptr = NULL;
int rt_kprintf(const char *fmt, ...) {
    if(rt_kprintf_api_ptr == NULL) {
        rt_kprintf_api_ptr = (rt_kprintf_api_ptr_t)get_per_api_ptr(__FUNCTION__);
    }

    if(rt_kprintf_api_ptr != NULL) {
        va_list args;
        int len;

        va_start(args, fmt);
        len = rt_kprintf_api_ptr(fmt, args);
        va_end(args);

        return len;
    } else {
        return 0;
    }
}

static rt_malloc_api_ptr_t rt_malloc_api_ptr = NULL;
void *rt_malloc(rt_size_t size) {
    if(rt_malloc_api_ptr == NULL) {
        rt_malloc_api_ptr = (rt_malloc_api_ptr_t)get_per_api_ptr(__FUNCTION__);
    }

    if(rt_malloc_api_ptr != NULL) {
        return rt_malloc_api_ptr(size);
    } else {
        return (void *)NULL;
    }
}

static rt_free_api_ptr_t rt_free_api_ptr = NULL;
void rt_free(void *ptr) {
    if(rt_free_api_ptr == NULL) {
        rt_free_api_ptr = (rt_free_api_ptr_t)get_per_api_ptr(__FUNCTION__);
    }

    if(rt_free_api_ptr != NULL) {
        rt_free_api_ptr(ptr);
    }
}

static rt_realloc_api_ptr_t rt_realloc_api_ptr = NULL;
void *rt_realloc(void *ptr, rt_size_t newsize) {
    if(rt_realloc_api_ptr == NULL) {
        rt_realloc_api_ptr = (rt_realloc_api_ptr_t)get_per_api_ptr(__FUNCTION__);
    }

    if(rt_realloc_api_ptr != NULL) {
        return rt_realloc_api_ptr(ptr, newsize);
    } else {
        return (void *)NULL;
    }
}

static rt_calloc_api_ptr_t rt_calloc_api_ptr = NULL;
void *rt_calloc(rt_size_t count, rt_size_t size) {
    if(rt_calloc_api_ptr == NULL) {
        rt_calloc_api_ptr = (rt_calloc_api_ptr_t)get_per_api_ptr(__FUNCTION__);
    }

    if(rt_calloc_api_ptr != NULL) {
        return rt_calloc_api_ptr(count, size);
    } else {
        return (void *)NULL;
    }
}

static rt_thread_mdelay_api_ptr_t rt_thread_mdelay_api_ptr = NULL;
rt_err_t rt_thread_mdelay(rt_int32_t ms) {
    if(rt_thread_mdelay_api_ptr == NULL) {
        rt_thread_mdelay_api_ptr = (rt_thread_mdelay_api_ptr_t)get_per_api_ptr(__FUNCTION__);
    }

    if(rt_thread_mdelay_api_ptr != NULL) {
        return rt_thread_mdelay_api_ptr(ms);
    } else {
        return RT_ERROR;
    }
}