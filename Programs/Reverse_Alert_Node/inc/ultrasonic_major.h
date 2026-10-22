// ultrasonic_major.h  (NEW FILE) - HC-SR04 style sensor, Trigger = P0.20, Echo = P0.19
#ifndef ULTRASONIC_MAJOR_H
#define ULTRASONIC_MAJOR_H

#include "major_type.h"

#define ULTRA_FAIL 999          // returned by dist_cal() when no echo was received

void INIT_Ultrasonic(void);
u32  dist_cal(void);            // distance in cm (1..400) or ULTRA_FAIL

#endif
