// Controls the vehicle's left and right indicator LEDs from CAN commands (ID 0x101).
// LEDs on P0.0..P0.7, ACTIVE-LOW (0 = LED ON).

#include <LPC21xx.h>
#include "major_type.h"
#include "DELAY_MAJOR.h"
#include "can_major.h"
#include "can_defines_major.h"

#define ID_INDICATOR 0x101
#define LED_MASK     0xFFUL
#define STEP_MS      70          // time per LED step
#define POLL_MS      10          // CAN is checked every 10 ms (7 x 10 = 70 ms)

// 'L' = Left, 'R' = Right, 'S' = Off
static u8 indicator_cmd = 'S';

// FIX: the old code read ONE CAN frame per full LED sweep (8 x 70 ms = 560 ms), but this
//      node also receives 0x102 and 0x201 (accept-all filter, ~10 frames/s in Reverse).
//      The tiny receive buffer overflowed and the 0x101 command (especially 'S' = OFF)
//      arrived late or was lost. Now every pending frame is read, and this is called
//      every 10 ms - even while an LED animation is running.
static void can_poll(void)
{
    struct CAN_Frame rx;

    while(C1GSR & RBS_BIT_READ)
    {
        CAN1_Rx(&rx);
        if(rx.ID == ID_INDICATOR)
            indicator_cmd = (u8)rx.Data1;
    }
}

int main(void)
{
    u8 step = 0, last_cmd = 'S', cmd, w;

    Init_CAN1();

    // FIX: IO0DIR / IO0SET / IO0CLR do not exist in LPC21xx.h (that naming belongs to
    //      LPC214x.h) -> "undefined identifier". The names for LPC2129 are IODIRx/IOSETx/IOCLRx.
    IODIR0 |= LED_MASK;      // P0.0..P0.7 outputs
    IOSET0  = LED_MASK;      // active-low -> all LEDs OFF

    while(1)
    {
        can_poll();
        cmd = indicator_cmd;

        if(cmd != last_cmd)  // new command: restart the sweep from the first LED
        {
            step = 0;
            last_cmd = cmd;
        }

        if(cmd == 'L' || cmd == 'R')
        {
            IOSET0 = LED_MASK;                                  // all OFF
            IOCLR0 = 1UL << ((cmd == 'L') ? step : (7 - step)); // one LED ON

            for(w = 0; w < (STEP_MS / POLL_MS); w++)
            {
                delay_ms(POLL_MS);
                can_poll();
                if(indicator_cmd != cmd)        // command changed -> react at once
                    break;
            }
            if(++step > 7) step = 0;
        }
        else
        {
            IOSET0 = LED_MASK;   // indicators OFF
            step = 0;
            delay_ms(POLL_MS);
        }
    }
}
