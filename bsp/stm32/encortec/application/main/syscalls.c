/*
 * @FilePath: memory.c
 * @Author: Jack Sun (jack.sun@quectel.com)
 * @brief     : <Description>
 * @version   : v1.0.0
 * @Date: 2024-09-06 17:19:38
 * @copyright : Copyright (c) 2024
 */

#include "rtthread.h"
#include <reent.h>

void* __wrap_malloc(size_t size)
{
    return rt_malloc(size);
}

void* __wrap_calloc(size_t nitems, size_t size)
{
    return rt_calloc(nitems, size);
}

void* __wrap_realloc(void *ptr, size_t size)
{
    return rt_realloc(ptr, size);
}

void __wrap_free(void* ptr)
{
    return rt_free(ptr);
}

int _gettimeofday_r (struct _reent *reent, struct timeval *__tp, void *__tzp)
{
    int result;

    result = gettimeofday(__tp, (struct timezone *)__tzp);
    if (result != 0)
    {
        reent->_errno = EINVAL;
    }

    return result;
}
