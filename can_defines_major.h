// CAN1 pin, bit timing, control and register bit definitions.
// Used by the CAN driver for LPC2129 CAN Controller 1.
// FIX: include guard, parenthesised bit masks, integer-only bit-timing maths,
//      compile-time checks, PCLK taken from clock_major.h.
#ifndef CAN_DEFINES_MAJOR_H
#define CAN_DEFINES_MAJOR_H

#include "clock_major.h"

// CAN1 RX PIN CONFIGURATION
// P0.25 -> PINSEL1 bits 19:18 = 01 -> RD1.  (TD1 is a dedicated pin on LPC2129,
// so it needs no PINSEL setting - this part of the old code was already correct.)
#define RD1_PIN 0x00040000UL

// CAN1 BIT TIMING CONFIGURATION
#define BIT_RATE   125000UL      // 125 kbps
#define QUANTA     20UL          // FIX: 16 only divides for PCLK 60 MHz; 20 works for 15, 30 and 60 MHz

#define BRP        (PCLK/(BIT_RATE*QUANTA))

// NOTE: PCLK is read from VPBDIV at run time (clock_major.h), so BRP/BTR_LVAL are evaluated at
// run time too. PCLK / (BIT_RATE*QUANTA) must be a whole number: with QUANTA = 20 that is true for
// PCLK = 15 MHz (BRP 6), 30 MHz (BRP 12) and 60 MHz (BRP 24), i.e. every VPBDIV setting at CCLK 60 MHz.

#define SAMPLE_Q   ((QUANTA*7)/10)            // sample point ~70 % of the bit (integer maths, no double)
#define TSEG1      (SAMPLE_Q-1)               // propagation + phase segment 1
#define TSEG2      (QUANTA-(1+TSEG1))         // phase segment 2
#define SJW        ((TSEG2 >= 5) ? 4 : (TSEG2-1))
#define SAM        0UL

#define BTR_LVAL   ((SAM<<23)|((TSEG2-1)<<20)|((TSEG1-1)<<16)|((SJW-1)<<14)|(BRP-1))

// C1CMR BIT DEFINITIONS
#define TR_BIT_SET    (1UL<<0)   // Transmission request
#define AT_BIT_SET    (1UL<<1)   // NEW: abort transmission (used on TX timeout)
#define RRB_BIT_SET   (1UL<<2)   // Release receive buffer
#define STB1_BIT_SET  (1UL<<5)   // Select transmit buffer 1

// C1GSR STATUS BIT DEFINITIONS
#define RBS_BIT_READ  (1UL<<0)   // Receive buffer status: message available
#define TBS1_BIT_READ (1UL<<2)   // Transmit buffer status: buffer free
#define TCS1_BIT_READ (1UL<<3)   // Transmit complete status

// CxTFI1 / CxRFS BIT DEFINITIONS
#define RTR_BIT  30
#define DLC_BITS 16

// CxMOD / AFMR BIT DEFINITIONS
#define RM_BIT     0
#define ACCOFF_BIT 0
#define ACCBP_BIT  1

// How long (loop counts) CAN1_Tx waits before giving up (no other node / no ACK).
#define CAN_TX_TIMEOUT 300000UL

#endif
