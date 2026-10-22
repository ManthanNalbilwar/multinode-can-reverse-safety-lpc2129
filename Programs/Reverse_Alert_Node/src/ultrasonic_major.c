// Ultrasonic sensor driver used by the Reverse Node.
// Trigger pulse -> measure Echo pulse width with Timer0 -> convert to cm.

#include <LPC21xx.h>
#include "DELAY_MAJOR.h"
#include "major_type.h"
#include "clock_major.h"
#include "ultrasonic_major.h"

#define trr_pin   20            // P0.20 = Trigger (output)
#define echo_pin  19            // P0.19 = Echo    (input; 5 V sensors need a divider to 3.3 V!)

#define ECHO_START_TIMEOUT_US  30000UL   // echo must start within 30 ms
#define ECHO_MAX_US            30000UL   // longest pulse we wait for (~515 cm)

void INIT_Ultrasonic(void)
{
    IODIR0 |=  (1UL << trr_pin);
    IODIR0 &= ~(1UL << echo_pin);
    IOCLR0  =  (1UL << trr_pin);

    // FIX: T0PR counts PR+1 clocks. 'T0PR = 15' gave 16 clocks per tick = 1.067 us, so every
    //      distance was ~6 % too small. For exactly 1 us per tick: PR = PCLK_in_MHz - 1.
    T0TCR = 0x02;                            // reset
    T0PR  = (PCLK / 1000000UL) - 1;
    T0TCR = 0x00;                            // stopped
}

static void send_pulse(void)
{
    IOCLR0 = (1UL << trr_pin);
    delay_us(2);
    IOSET0 = (1UL << trr_pin);
    delay_us(10);                            // >= 10 us trigger pulse
    IOCLR0 = (1UL << trr_pin);
}

// Returns echo pulse width in us, 0 = no echo started.
// FIX: timeouts were counted in loop iterations (30000 loops ~ only 4-5 ms at 60 MHz), which
//      cut off every echo longer than ~80 cm and made the result depend on compiler
//      optimisation. Now Timer0 (free running from the trigger) measures real time.
static u32 read_echo(void)
{
    u32 t_rise, t_fall;

    T0TCR = 0x02;                            // reset counter
    T0TCR = 0x01;                            // run

    while(((IOPIN0 >> echo_pin) & 1) == 0)   // wait for echo HIGH
    {
        if(T0TC > ECHO_START_TIMEOUT_US)
        {
            T0TCR = 0x00;
            return 0;                        // sensor not answering (wiring / power)
        }
    }
    t_rise = T0TC;

    while((IOPIN0 >> echo_pin) & 1)          // wait for echo LOW
    {
        if((T0TC - t_rise) > ECHO_MAX_US)
        {
            T0TCR = 0x00;
            return ECHO_MAX_US;              // nothing in range: clamp -> reported as 400 cm
        }
    }
    t_fall = T0TC;
    T0TCR = 0x00;

    return (t_fall - t_rise);
}

u32 dist_cal(void)
{
    u32 pulse, distance;

    send_pulse();
    pulse = read_echo();

    if(pulse == 0)
        return ULTRA_FAIL;

    distance = pulse / 58;                   // us / 58 = cm
    if(distance > 400) distance = 400;
    if(distance == 0)  distance = 1;         // 0 is never a valid reading
    return distance;
}
