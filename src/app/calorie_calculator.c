#include "calorie_calculator.h"

#define CALORIE_PER_100_STEPS      4U
#define STEPS_PER_CALORIE          (100U / CALORIE_PER_100_STEPS)
#define STEPS_PER_CALORIE_ROUND_UP ((STEPS_PER_CALORIE + 1U) / 2U)

rt_uint32_t CalorieCalculator_CalculateBySteps(rt_uint32_t step_count)
{
    rt_uint32_t calorie;
    rt_uint32_t remainder;

    calorie = step_count / STEPS_PER_CALORIE;
    remainder = step_count % STEPS_PER_CALORIE;
    if (remainder >= STEPS_PER_CALORIE_ROUND_UP)
    {
        calorie++;
    }

    return calorie;
}
