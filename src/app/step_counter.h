#ifndef __STEP_COUNTER_H__
#define __STEP_COUNTER_H__

#include "rtthread.h"

rt_err_t StepCounter_Init(void);
rt_err_t StepCounter_Read(rt_uint32_t *step_count);

#endif /* __STEP_COUNTER_H__ */
