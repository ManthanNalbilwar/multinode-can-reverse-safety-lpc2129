// Reverse Node: receives the vehicle mode (CAN 0x102), measures obstacle distance with an
// ultrasonic sensor in Reverse mode and sends distance + alert (CAN 0x201).
//
// 0x201 payload (DLC = 3): byte0..1 = distance in cm (little endian), byte2 = alert
//                          alert: 0 = safe, 1 = closer than 20 cm, 2 = sensor error

#include <LPC21xx.h>
#include "major_type.h"
#include "DELAY_MAJOR.h"
#include "can_major.h"
#include "can_defines_major.h"
#include "ultrasonic_major.h"

#define ID_MODE        0x102
#define ID_REVERSE     0x201
#define ALERT_DIST_CM  20
#define ALERT_NONE     0
#define ALERT_CLOSE    1
#define ALERT_SENSOR   2

static u8 mode = 0;               // 0 = Forward, 1 = Reverse

static void can_poll(void)        // FIX: drain all frames (old code read only one per loop)
{
    struct CAN_Frame rx;

    while(C1GSR & RBS_BIT_READ)
    {
        CAN1_Rx(&rx);
        if(rx.ID == ID_MODE)
            mode = (rx.Data1 & 0xFF) ? 1 : 0;
    }
}

// Three measurements, median of the valid ones.
// FIX 1: the old plain average hid obstacles: one failed reading (999) + two real 10 cm
//        readings averaged to 339 cm -> "SAFE" while an obstacle was 10 cm away.
// FIX 2: >= 60 ms between ultrasonic shots, otherwise the echo of the previous shot is
//        picked up as a fake echo.
static u32 measure(u8 *alert)
{
    u32 v[3], tmp;
    u8  n = 0, i, j;

    for(i = 0; i < 3; i++)
    {
        tmp = dist_cal();
        if(tmp != ULTRA_FAIL)
            v[n++] = tmp;
        delay_ms(60);
    }

    if(n == 0)                    // FIX: failed sensor was reported as 999 cm = "SAFE". Now an explicit error.
    {
        *alert = ALERT_SENSOR;
        return ULTRA_FAIL;
    }

    for(i = 0; i < n; i++)        // tiny insertion sort
        for(j = i + 1; j < n; j++)
            if(v[j] < v[i]) { tmp = v[i]; v[i] = v[j]; v[j] = tmp; }

    tmp = v[(n - 1) / 2];         // n=1 -> v0, n=2 -> smaller (safer), n=3 -> median
    *alert = (tmp < ALERT_DIST_CM) ? ALERT_CLOSE : ALERT_NONE;
    return tmp;
}

int main(void)
{
    struct CAN_Frame tx;
    u32 distance;
    u8  alert;

    Init_CAN1();
    INIT_Ultrasonic();

    while(1)
    {
        can_poll();

        if(mode)
        {
            distance = measure(&alert);
            can_poll();                       // mode may have changed during the 180 ms measurement
            if(mode)
            {
                tx.ID      = ID_REVERSE;
                tx.vbf.RTR = 0;
                tx.vbf.DLC = 3;               // FIX: was DLC 2 with 'alert' in Data2 -> alert was never transmitted
                tx.Data1   = (distance & 0xFFFF) | ((u32)alert << 16);
                tx.Data2   = 0;
                CAN1_Tx(tx);
            }
        }

        delay_ms(50);
    }
}
