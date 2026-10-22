// DELAY FUNCTION DEFINITIONS (software loops, calibrated for CCLK = 60 MHz)
#include "major_type.h"      // FIX: the two includes had swapped comments
#include "DELAY_MAJOR.h"

// FIX: the counters are now 'volatile'. Without it an optimising compiler
//      (-O1/-O2) deletes these empty loops and every delay becomes ~0
//      (this breaks LCD timing, DS18B20 timing and the ultrasonic trigger).
void delay_us(u32 dlyus)
{
        volatile u32 n;
        for(n = dlyus * 12; n > 0; n--);
}

void delay_ms(u32 dlyms)
{
        volatile u32 n;
        for(n = dlyms * 12000; n > 0; n--);
}

void delay_s(u32 dlys)
{
        while(dlys--)
                delay_ms(1000);      // FIX: dlys*12000000 overflowed u32 for dlys > 357
}
