// clock_major.h  (NEW FILE)
// ONE place for the clocks. CAN bit timing (can_defines_major.h) and the Timer0/Timer1
// prescalers (ultrasonic / debounce) are all derived from here.
//
// Old project: CAN code assumed PCLK = 60 MHz, ultrasonic code assumed 15 MHz. Both cannot be true.
//
// PCLK is now READ FROM THE CHIP at run time:  PCLK = CCLK / VPBDIV
//   VPBDIV = 0 -> CCLK/4 (reset value, UM: Table 34),  1 -> CCLK/1,  2 -> CCLK/2
// CCLK is 60 MHz (PLL set up by Startup.s). The delay_us/delay_ms loops are calibrated for
// the same 60 MHz, so this is the one assumption the whole project already depends on.
// >>> If your CCLK is NOT 60 MHz, change CCLK below (and recalibrate delay_MAJOR.c). <<<
#ifndef CLOCK_MAJOR_H
#define CLOCK_MAJOR_H

#include <LPC21xx.h>

#define CCLK 60000000UL

#define PCLK_DIVIDER  (((VPBDIV & 3) == 1) ? 1UL : (((VPBDIV & 3) == 2) ? 2UL : 4UL))
#define PCLK          (CCLK / PCLK_DIVIDER)

#endif
