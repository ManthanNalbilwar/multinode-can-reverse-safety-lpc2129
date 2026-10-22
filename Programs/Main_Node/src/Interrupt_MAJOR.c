// interrupt_MAJOR.c
// External interrupts of the Main Node (switches moved because the LCD data bus now uses P0.0-P0.7):
//   EINT0 (P0.16) -> Mode switch        EINT1 (P0.14) -> Left indicator
//   EINT2 (P0.15) -> Right indicator
// Timer1 is used as a free-running 1 ms counter for switch debouncing and scheduling.

#include <LPC21xx.h>
#include "major_type.h"
#include "clock_major.h"
#include "interrupt_major.h"

#define DEBOUNCE_MS 250UL

static u32 last_edge[3];          // time of the last edge seen on EINT0/1/2

void eint0_isr(void) __irq;
void eint1_isr(void) __irq;
void eint2_isr(void) __irq;

u32 millis(void)
{
    return T1TC;                  // Timer1 counter ticks every 1 ms
}

// Returns 1 if this edge is a real press, 0 if it is contact bounce.
// The timestamp is refreshed on EVERY edge, so a burst of bounces stays blocked.
static u8 press_accepted(u8 n)
{
    u32 now = T1TC;
    u8  ok  = ((now - last_edge[n]) >= DEBOUNCE_MS);
    last_edge[n] = now;
    return ok;
}

void interrupt_enable(void)
{
    // FIX: switches bounce for several ms; every bounce used to toggle mode / indicator again.
    // Timer1: 1 ms tick
    T1TCR = 0x02;                                // reset
    T1PR  = (PCLK / 1000UL) - 1;                 // prescaler -> 1 ms per T1TC count
    T1TCR = 0x01;                                // run

    // Pins: P0.16 = EINT0 (PINSEL1 bits 1:0 = 01), P0.14 = EINT1 (PINSEL0 bits 29:28 = 10),
    //       P0.15 = EINT2 (PINSEL0 bits 31:30 = 10)   [LPC2129 manual, Tables 58 and 60]
    // P0.1 / P0.3 / P0.7 are NOT touched any more - they belong to the LCD data bus (GPIO).
    PINSEL0 &= ~((3UL<<28) | (3UL<<30));
    PINSEL0 |=  ((2UL<<28) | (2UL<<30));
    PINSEL1 &= ~(3UL<<0);
    PINSEL1 |=  (1UL<<0);

    // FIX: set edge/polarity FIRST, clear flags AFTER. Changing mode/polarity can itself
    //      raise a spurious flag, which the old order (clear first) left pending.
    EXTMODE  = 0x07;                             // edge sensitive
    EXTPOLAR = 0x00;                             // falling edge
    EXTINT   = 0x07;                             // clear pending flags

    VICIntSelect &= ~((1UL<<14)|(1UL<<15)|(1UL<<16));   // FIX: only these 3 -> IRQ (old code zeroed all 32 bits)

    VICVectCntl0 = (1UL<<5) | 14;   VICVectAddr0 = (u32)eint0_isr;
    VICVectCntl1 = (1UL<<5) | 15;   VICVectAddr1 = (u32)eint1_isr;
    VICVectCntl2 = (1UL<<5) | 16;   VICVectAddr2 = (u32)eint2_isr;

    VICIntEnable = (1UL<<14) | (1UL<<15) | (1UL<<16);   // enable last, after vectors are ready
}

/**************** MODE SWITCH - EINT0 ****************/
void eint0_isr(void) __irq
{
    if(press_accepted(0))
    {
        mode = !mode;
        tx_mode_flag = 1;       // main loop sends CAN ID 0x102
    }
    EXTINT = 1<<0;
    VICVectAddr = 0;
}

/**************** LEFT SWITCH - EINT1 ****************/
void eint1_isr(void) __irq
{
    if(press_accepted(1))
    {
        if(left_flag == 0) { left_flag = 1; right_flag = 0; indicator_cmd = 'L'; }
        else               { left_flag = 0;                 indicator_cmd = 'S'; }
        tx_ind_flag = 1;        // main loop sends CAN ID 0x101
    }
    EXTINT = 1<<1;
    VICVectAddr = 0;
}

/**************** RIGHT SWITCH - EINT2 ****************/
void eint2_isr(void) __irq
{
    if(press_accepted(2))
    {
        if(right_flag == 0) { right_flag = 1; left_flag = 0; indicator_cmd = 'R'; }
        else                { right_flag = 0;                indicator_cmd = 'S'; }
        tx_ind_flag = 1;
    }
    EXTINT = 1<<2;
    VICVectAddr = 0;
}
