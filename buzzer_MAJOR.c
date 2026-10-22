#include <LPC21xx.h>          // FIX: was <lpc21xx.h> (fails on case-sensitive systems)
#include "buzzer_major.h"

#define BUZZER 20             // Buzzer is connected to P0.20 (ACTIVE-LOW circuit)

// FIX 1: Buzzer_init() used IOCLR0 = LOW = buzzer ON (active-low), so the buzzer
//        screamed from power-up until the first Buzzer_OFF(). Now it starts OFF.
// FIX 2: IOSET0/IOCLR0 are "write 1 to act" registers. 'IOCLR0 |= x' reads a
//        write-only register first; use plain '=' so no other pin is touched.
void Buzzer_init(void)
{
    IODIR0 |= (1UL << BUZZER);   // P0.20 output
    IOSET0  = (1UL << BUZZER);   // HIGH = OFF for active-low buzzer
}

void Buzzer_ON(void)
{
    IOCLR0 = (1UL << BUZZER);    // LOW = buzzer ON
}

void Buzzer_OFF(void)
{
    IOSET0 = (1UL << BUZZER);    // HIGH = buzzer OFF
}
