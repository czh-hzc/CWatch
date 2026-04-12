#ifndef __COMPASS_H__
#define __COMPASS_H__

float Compass_Calculate(float ax, float ay, float az,
                        float gx, float gy, float gz,
                        float mx, float my, float mz);
void Compass_Reset(void);

#endif /* __COMPASS_H__ */
