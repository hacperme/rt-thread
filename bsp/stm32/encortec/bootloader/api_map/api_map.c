#include "api_map.h"
#include "rtthread.h"

static rt_api_list_t rt_api_list[] = {
    {0x39fb0d36, rt_thread_init},
    {0x09a96298, rt_thread_detach},
#ifdef RT_USING_HEAP
    {0x26b5b34c, rt_thread_create},
    {0x4f2baae6, rt_thread_delete},
#endif
    {0x28137e6e, rt_thread_close},
    {0x9bdbde88, rt_thread_self},
    {0x9fdde7da, rt_thread_find},
    {0xe07db55e, rt_thread_startup},
    {0x68aff522, rt_thread_yield},
    {0xe3f8006a, rt_thread_delay},
    {0x98e90f34, rt_thread_delay_until},
    {0x07980e78, rt_thread_mdelay},
    {0x6adeab1c, rt_thread_control},
    {0x165cb79e, rt_thread_suspend},
    {0x848030d0, rt_thread_suspend_with_flag},
    {0xa3ac46f6, rt_thread_resume},
    {0x772fa776, rt_thread_get_name},
#ifdef RT_USING_SIGNALS
    {0x408cbf10, rt_thread_alloc_sig},
    {0x33f73ffe, rt_thread_free_sig},
    {0x7a99b762, rt_thread_kill},
#endif
    {0x7dc83712, rt_enter_critical},
    {0x9c1777da, rt_exit_critical},
    {0xf8f4a09c, rt_exit_critical_safe},
    {0xf04c8494, rt_critical_level},
#ifdef RT_USING_SEMAPHORE
    {0xa9cae0ac, rt_sem_init},
    {0xb29a6f6a, rt_sem_detach},
#ifdef RT_USING_HEAP
    {0x585b4982, rt_sem_create},
    {0x12879088, rt_sem_delete},
#endif
    {0xfd4bacca, rt_sem_take},
    {0xfd5a88f0, rt_sem_take_interruptible},
    {0x4caf0896, rt_sem_take_killable},
    {0x9458fbee, rt_sem_trytake},
    {0x8ff7fe70, rt_sem_release},
    {0x9dcf32ee, rt_sem_control},
#endif
#ifdef RT_USING_MUTEX
    {0x6dfa7072, rt_mutex_init},
    {0x033243bc, rt_mutex_detach},
#ifdef RT_USING_HEAP
    {0x6a3311b0, rt_mutex_create},
    {0x48350f0a, rt_mutex_delete},
#endif
    {0xbc8efe40, rt_mutex_drop_thread},
    {0x9b0b9c66, rt_mutex_setprioceiling},
    {0xf9b2cd12, rt_mutex_getprioceiling},
    {0x58226984, rt_mutex_take},
    {0x5782bcf4, rt_mutex_trytake},
    {0x7ddbed72, rt_mutex_take_interruptible},
    {0x8b3a169c, rt_mutex_take_killable},
    {0x9c62f25a, rt_mutex_release},
    {0x0f0b01c0, rt_mutex_control},
#endif
#ifdef RT_USING_EVENT
    {0xe92939a8, rt_event_init},
    {0x20473e76, rt_event_detach},
#ifdef RT_USING_HEAP
    {0xe9b5de36, rt_event_create},
    {0xdf2d9e04, rt_event_delete},
#endif
    {0xfef08982, rt_event_send},
    {0x68551e34, rt_event_recv},
    {0x105a7b02, rt_event_recv_interruptible},
    {0xd8c6a84c, rt_event_recv_killable},
    {0x177a6ab2, rt_event_control},
#endif
#ifdef RT_USING_MESSAGEQUEUE
    {0xca884a68, rt_mq_init},
    {0x91aefeb6, rt_mq_detach},
#ifdef RT_USING_HEAP
    {0x3e5965f6, rt_mq_create},
    {0x0b98f144, rt_mq_delete},
#endif
    {0xf405c5c2, rt_mq_send},
    {0x15d48178, rt_mq_send_interruptible},
    {0xc1413c8e, rt_mq_send_killable},
    {0x85ff1066, rt_mq_send_wait},
    {0xd8c20ab4, rt_mq_send_wait_interruptible},
    {0x6ba26db2, rt_mq_send_wait_killable},
    {0x55498fbe, rt_mq_urgent},
    {0x88224774, rt_mq_recv},
    {0x1bb7bec2, rt_mq_recv_interruptible},
    {0x9607b58c, rt_mq_recv_killable},
    {0x3ea0e6f2, rt_mq_control},
#ifdef RT_USING_MESSAGEQUEUE_PRIORITY
    {0x39c48afe, rt_mq_send_wait_prio},
    {0xd65517e8, rt_mq_recv_prio},
#endif
#endif
    {0xeee0d5c2, rt_interrupt_enter},
    {0xeca23242, rt_interrupt_leave},
#ifdef RT_DEBUGING_ASSERT
    {0x3a31369c, rt_assert_handler},
#endif
#ifdef RT_USING_CONSOLE
    {0x6e3516b8, rt_kputs},
    {0x35baa892, rt_kprintf},
#endif
#ifdef RT_USING_HEAP
    {0x686b950a, rt_malloc},
    {0x8f97b332, rt_free},
    {0x05169dac, rt_realloc},
    {0x7bfa4cb0, rt_calloc},
	{0xf40ed0fa, rt_memory_info},
#endif
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
