#include "rtthread.h"
#include "data_hub.h"

systemdata_t system_data;
rt_mutex_t data_mutex;

void DataHub_Init(void) 
{
    data_mutex = rt_mutex_create("data_mut", RT_IPC_FLAG_PRIO);
    if (data_mutex == RT_NULL) 
    {
        rt_kprintf("Data Hub Mutex create failed!\n");
    }
}