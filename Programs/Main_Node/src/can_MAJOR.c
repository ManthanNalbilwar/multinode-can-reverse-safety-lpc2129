// CAN Controller 1 driver for the LPC2129.
// Handles CAN1 initialization, transmission and reception.

#include <LPC21xx.h>
#include "major_type.h"
#include "can_major.h"
#include "can_defines_major.h"

/****************************************
        CAN1 INITIALIZATION
****************************************/
void Init_CAN1(void)
{
    PINSEL1 |= RD1_PIN;     // P0.25 = RD1 (TD1 is a dedicated pin)
    C1MOD  = 1;             // Reset mode: configuration allowed
    AFMR   = 2;             // Acceptance filter bypass: accept every received frame
    C1BTR  = BTR_LVAL;      // Bit timing
    C1MOD  = 0;             // Leave reset mode -> CAN1 active
}

/****************************************
        CAN1 TRANSMISSION
****************************************/
// Returns 1 when the frame was transmitted, 0 on timeout.
// FIX: both while() loops had no timeout. With no second node on the bus (or a
//      missing transceiver / no ACK) the controller retries forever and the whole
//      node froze inside CAN1_Tx (LCD stopped, switches dead).
u8 CAN1_Tx(struct CAN_Frame txFrame)
{
    u32 t = CAN_TX_TIMEOUT;

    // Wait for a free transmit buffer 1
    while((C1GSR & TBS1_BIT_READ) == 0)
    {
        if(--t == 0) return 0;
    }

    C1TID1 = txFrame.ID;
    C1TFI1 = ((u32)txFrame.vbf.RTR << RTR_BIT) |
             ((u32)txFrame.vbf.DLC << DLC_BITS);

    if(txFrame.vbf.RTR != 1)
    {
        C1TDA1 = txFrame.Data1;
        C1TDB1 = txFrame.Data2;
    }

    C1CMR = STB1_BIT_SET | TR_BIT_SET;      // select buffer 1 + request transmission

    t = CAN_TX_TIMEOUT;
    while((C1GSR & TCS1_BIT_READ) == 0)
    {
        if(--t == 0)
        {
            C1CMR = AT_BIT_SET | STB1_BIT_SET;   // abort the pending transmission of buffer 1
            return 0;
        }
    }
    return 1;
}

/****************************************
        CAN1 RECEPTION
****************************************/
// Call only when (C1GSR & RBS_BIT_READ) is set, or accept that it blocks.
void CAN1_Rx(struct CAN_Frame *rxFrame)
{
    while((C1GSR & RBS_BIT_READ) == 0);

    rxFrame->ID      = C1RID & 0x7FF;           // 11-bit identifier
    rxFrame->vbf.RTR = (C1RFS >> RTR_BIT) & 1;
    rxFrame->vbf.DLC = (C1RFS >> DLC_BITS) & 0x0F;

    if(rxFrame->vbf.RTR == 0)
    {
        rxFrame->Data1 = C1RDA;
        rxFrame->Data2 = C1RDB;
    }
    else
    {
        rxFrame->Data1 = 0;     // FIX: do not leave stale data from the previous frame
        rxFrame->Data2 = 0;
    }

    C1CMR = RRB_BIT_SET;        // release receive buffer
}
