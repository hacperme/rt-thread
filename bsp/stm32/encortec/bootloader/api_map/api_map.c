#include "api_map.h"
#include "rtthread.h"

static rt_api_list_t rt_api_list[] = {
    {0x07980e78, rt_thread_mdelay},
    {0x35baa892, rt_kprintf},
    {0x686b950a, rt_malloc}, 
    {0x8f97b332, rt_free},   
    {0x05169dac, rt_realloc},
    {0x7bfa4cb0, rt_calloc}
};

#define MAX_API_NUM (sizeof(rt_api_list)/sizeof(rt_api_list[0]))

static uint32_t get_api_hash(const char * src)
{    
    uint32_t h, v;
    char * p; 

    for(h=0, p = (char *)src; *p ; p++)
    {
        h = 5527 * h + 7 *(*p);
        v = h & 0x0000ffff;
        h ^= v*v;
    }

    return h;
}

static int32_t match_in_list(int32_t hash, rt_api_list_t* api_list, int32_t list_max )
{
    int32_t iMax = list_max - 1, iMin = 0;


    if( hash < api_list[iMin].hash || hash > api_list[iMax].hash)
        return -1;
    
    do {
        if( hash > api_list[iMin + (iMax - iMin) / 2].hash)
            iMin = iMin + (iMax - iMin) / 2;
        else
            iMax = iMin + (iMax - iMin) / 2;

    } while((iMax - iMin) > 1);

    if( hash == api_list[iMax].hash) {
        return iMax;
    } else if( hash == api_list[iMin].hash) {
        return iMin;
    } else {
        return -1;
    }
}

static void api_list_split_array(rt_api_list_t* arr, int32_t low, int32_t high, int32_t *p ) 
{ 
    int32_t i, j;
    int32_t item;
    rt_api_list_t t;

    item = arr[low].hash;
    i = low;
    j = high;

    while( i < j )
    {
        /*  move from R to L in search of element < item */
        while ( arr[j].hash> item )
            j = j - 1;

        /*  move from L to R in search of element > item */
        while ( arr[i].hash<= item  &&  i < j )
            i = i + 1;

        if ( i < j )
        {
            t = arr[i];
            arr[i] = arr[j];
            arr[j] = t;
        }
    }
    *p = j;
    t = arr[low];
    arr[low] = arr[*p];
    arr[*p] = t;
}

static void api_list_quick_sort(rt_api_list_t* arr, int32_t low, int32_t high )
{
    int32_t pos;

    if( low < high )
    {
        api_list_split_array( arr, low, high, &pos );
        api_list_quick_sort( arr, low, pos - 1 );
        api_list_quick_sort( arr, pos + 1, high );
    }
}

uint32_t get_per_api_ptr(const char * func_name)
{
    int32_t resid = -1;
    uint32_t hash = 0;

    if(func_name == NULL) {
        return 0;
    }

    hash = get_api_hash(func_name);
    resid = match_in_list(hash, rt_api_list, MAX_API_NUM);

    if((resid >= 0) && (resid < MAX_API_NUM)) {
        return (uint32_t)rt_api_list[resid].api_ptr;
    } else {
        return 0;
    }
}

void rt_api_map_init(void)
{
    api_list_quick_sort(rt_api_list, 0, MAX_API_NUM - 1);
}
