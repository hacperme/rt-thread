/*
 * @FilePath: memory.c
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-09-06 17:19:38
 * @copyright : Copyright (c) 2024
 */
#include "rtthread.h"
#include "memory.h"

void * malloc(rt_size_t size)
{
    return rt_malloc(size);
}

void free(void *ptr) {
    rt_free(ptr);
}

void * realloc(void *ptr, rt_size_t newsize) {
    return rt_realloc(ptr, newsize);
}

void * calloc(rt_size_t count, rt_size_t size) {
    return rt_calloc(count, size);
}

caddr_t _sbrk(int increment)
{
    return (caddr_t)-1;

    // extern char __heap_end[];
    // extern char __heap_start[];

    // static char *s_pHeapEnd = RT_NULL;

    // if (!s_pHeapEnd)
    //     s_pHeapEnd = __heap_start;

    // if (s_pHeapEnd + increment > __heap_end)
    //     return (caddr_t)-1;

    // char *pOldHeapEnd = s_pHeapEnd;
    // s_pHeapEnd += increment;
    // return (caddr_t)pOldHeapEnd;
}