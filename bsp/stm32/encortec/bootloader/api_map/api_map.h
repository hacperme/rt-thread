
#ifndef __RT_API_MAP__
#define __RT_API_MAP__

#include <stdint.h>

typedef struct {
    int32_t hash;
    void *api_ptr;
} rt_api_list_t;

extern void rt_api_map_init(void);

#endif
