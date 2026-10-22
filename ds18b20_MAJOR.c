// Driver for the DS18B20 temperature sensor using the 1-Wire protocol.
// Data line = P0.19 with an EXTERNAL pull-up resistor (about 4.7 k to 3.3 V).

#include <LPC21xx.h>
#include "DELAY_MAJOR.h"
#include "major_type.h"
#include "DS18B20_MAJOR.h"

#define D (1UL << 19)

// FIX: open-drain emulation.
// The old code drove the line HIGH from the LPC (push-pull) while the DS18B20 could
// still be pulling it LOW (ReadBit re-enabled the output right after sampling).
// That is a bus contention. Now the LPC only ever drives LOW, or releases the line
// (input) and lets the pull-up make it HIGH.
#define DQ_LOW()      do { IOCLR0 = D; IODIR0 |= D; } while(0)
#define DQ_RELEASE()  do { IODIR0 &= ~D; } while(0)
#define DQ_READ()     ((IOPIN0 & D) ? 1 : 0)

/****************************************
        RESET + PRESENCE
****************************************/
// Returns 1 if a DS18B20 answered with a presence pulse, else 0.
// FIX 1: old function returned 0 when present (confusing) and compared a whole-port
//        snapshot with a second, later read of the pin.
// FIX 2: presence is now sampled 70 us after release. The sensor starts its pulse
//        15..60 us after release, so sampling at 54 us could be too early.
unsigned char ResetDS18b20(void)
{
    unsigned char present;

    DQ_LOW();
    delay_us(480);          // reset pulse >= 480 us
    DQ_RELEASE();
    delay_us(70);
    present = (DQ_READ() == 0);
    delay_us(410);          // finish the >= 480 us reset slot
    return present;
}

/****************************************
        BIT / BYTE LEVEL
****************************************/
unsigned char ReadBit(void)
{
    unsigned char b;

    DQ_LOW();
    delay_us(2);            // start of read slot (1-2 us)
    DQ_RELEASE();
    delay_us(8);            // sample inside the 15 us window
    b = DQ_READ();
    delay_us(55);           // rest of the 60+ us slot, line stays released (no contention)
    return b;
}

void WriteBit(unsigned char Dbit)
{
    if(Dbit)
    {
        DQ_LOW();
        delay_us(2);        // short low = write 1
        DQ_RELEASE();
        delay_us(60);
    }
    else
    {
        DQ_LOW();
        delay_us(60);       // long low = write 0
        DQ_RELEASE();
        delay_us(2);        // recovery
    }
}

unsigned char ReadByte(void)
{
    unsigned char i, Din = 0;

    for(i = 0; i < 8; i++)                  // LSB first
        if(ReadBit())
            Din |= (unsigned char)(1u << i);
    return Din;
}

void WriteByte(unsigned char Dout)
{
    unsigned char i;

    for(i = 0; i < 8; i++)                  // LSB first
    {
        WriteBit(Dout & 0x01);
        Dout >>= 1;
    }
}

/****************************************
        TEMPERATURE
****************************************/
unsigned char DS18B20_Start(void)
{
    if(!ResetDS18b20())
        return 0;
    WriteByte(0xCC);        // SKIP ROM  (only one sensor on the bus)
    WriteByte(0x44);        // CONVERT T
    return 1;
}

int DS18B20_Read(void)
{
    unsigned char lo, hi;

    if(!ResetDS18b20())
        return DS18B20_ERROR;
    WriteByte(0xCC);        // SKIP ROM
    WriteByte(0xBE);        // READ SCRATCHPAD
    lo = ReadByte();        // byte 0 = temperature LSB
    hi = ReadByte();        // byte 1 = temperature MSB
    // FIX: sign-extend. The old code built an unsigned 16-bit value, so -10 C came out as ~4085 C.
    return (int)(s16)(((u16)hi << 8) | lo);
}

int ReadTemp(void)
{
    u32 timeout = 0;

    if(!DS18B20_Start())
        return DS18B20_ERROR;

    // FIX (the big one): while converting, the DS18B20 answers every read slot with 0
    // and answers 1 when finished. The old code looped "while(ReadByte() == 0xFF)",
    // i.e. it left the loop IMMEDIATELY (busy = 0x00) and read the scratchpad before
    // the conversion was done -> stale values / the power-on value 85 C.
    while(ReadBit() == 0)
    {
        if(++timeout > 20000)       // 20000 x ~65 us = 1.3 s  (12-bit conversion is <= 750 ms)
            return DS18B20_ERROR;
    }
    return DS18B20_Read();
}
