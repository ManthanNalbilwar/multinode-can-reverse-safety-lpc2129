// CAN Controller 1 function declarations and CAN frame structure.
#ifndef _CAN_H_
#define _CAN_H_

#include "major_type.h"

// Structure used to store a CAN message.
struct CAN_Frame
{
        u32 ID;                    // 11-bit CAN identifier

        struct BitField
        {
                u8 RTR : 1;        // 0 = Data Frame, 1 = Remote Frame
                u8 DLC : 4;        // Number of data bytes actually sent on the bus (0..8)
        }vbf;

        u32 Data1, Data2;          // Data1 = bytes 1-4, Data2 = bytes 5-8
        // NOTE: only DLC bytes go on the wire. Bytes beyond DLC are NOT transmitted.
};

void Init_CAN1(void);

// FIX: now returns 1 = sent, 0 = timeout (no ACK / bus problem). Earlier it could hang forever.
u8   CAN1_Tx(struct CAN_Frame);

void CAN1_Rx(struct CAN_Frame *);

#endif
