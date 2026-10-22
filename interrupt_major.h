// interrupt_major.h  (NEW FILE)  - shared declarations of the interrupt module
#ifndef INTERRUPT_MAJOR_H
#define INTERRUPT_MAJOR_H

#include "major_type.h"

void interrupt_enable(void);   // switches (EINT0/1/2) + 1 ms time base (Timer1)
u32  millis(void);             // milliseconds since interrupt_enable()

// Shared with the main program
extern volatile u8 mode;           // 0 = Forward, 1 = Reverse
extern volatile u8 left_flag;
extern volatile u8 right_flag;
extern volatile u8 tx_ind_flag;    // FIX: one flag per message (was one shared tx_flag that could lose a message)
extern volatile u8 tx_mode_flag;
extern volatile u8 indicator_cmd;  // 'L', 'R' or 'S'

#endif
