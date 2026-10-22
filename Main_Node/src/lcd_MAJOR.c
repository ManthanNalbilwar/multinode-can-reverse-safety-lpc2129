// major_lcd.c
// LCD driver functions (8-bit mode, 20x4 line addressing).

#include <LPC21xx.h>
#include "major_type.h"
#include "major_lcd_defines.h"
#include "DEFINES_MAJOR.h"
#include "DELAY_MAJOR.h"
#include "major_lcd.h"       // FIX: was "MAJOR_LCD.h" -> file not found on case-sensitive systems

#define LCD_DATA_MASK (0xFFUL << LCD_DATA)

// Send one byte of data/command to the LCD (RS must already be selected).
void WriteLCD(u8 data)
{
    SCLRBIT(IOCLR0, RW);                    // RW = 0 : write
    // FIX: old code did a read-modify-write on IOPIN0 (touches every pin of the port,
    //      incl. DS18B20, buzzer, EINT pins). Now only the 8 data pins are changed.
    IOCLR0 = LCD_DATA_MASK;
    IOSET0 = ((u32)data << LCD_DATA);
    SSETBIT(IOSET0, EN);                    // EN = 1
    delay_us(1);
    SCLRBIT(IOCLR0, EN);                    // EN = 0 : LCD latches the byte
    delay_ms(2);
}

void CmdLCD(u8 cmd)
{
    SCLRBIT(IOCLR0, RS);                    // RS = 0 : command register
    WriteLCD(cmd);
}

void CharLCD(u8 ascii)
{
    SSETBIT(IOSET0, RS);                    // RS = 1 : data register
    WriteLCD(ascii);
}

void InitLCD(void)
{
    WBYTE(IODIR0, LCD_DATA, 255);           // D0..D7 outputs
    SETBIT(IODIR0, RS);
    SETBIT(IODIR0, EN);                     // FIX: comments said RW=P0.17/EN=P0.18 but
    SETBIT(IODIR0, RW);                     //      the defines are RS=16, EN=17, RW=18
    delay_ms(15);
    CmdLCD(MODE_8BIT_1LINE);
    delay_ms(5);
    CmdLCD(MODE_8BIT_1LINE);
    delay_us(100);
    CmdLCD(MODE_8BIT_1LINE);
    CmdLCD(MODE_8BIT_2LINE);
    CmdLCD(DISP_ON);
    CmdLCD(CLEAR_LCD);
    CmdLCD(SHIFT_CUR_RIGHT);
}

void StrLCD(s8 *p)
{
    while(*p)
        CharLCD(*p++);
}

void U32LCD(u32 n)
{
    u8  a[10];
    s32 i = 0;

    if(n == 0)
    {
        CharLCD('0');
    }
    else
    {
        while(n)
        {
            a[i++] = (n % 10) + '0';
            n /= 10;
        }
        for(--i; i >= 0; i--)
            CharLCD(a[i]);
    }
}

// NEW: print n, then spaces so that exactly 'width' columns are used.
// Fixes leftover digits on the LCD (e.g. 100 -> 95 used to show "955").
void U32LCDW(u32 n, u8 width)
{
    u32 t = n;
    u8  digits = 1;

    while(t >= 10) { t /= 10; digits++; }
    U32LCD(n);
    while(digits++ < width)
        CharLCD(' ');
}

// FIX: the old version printed NOTHING for n >= 0 (the 'else' branch was missing).
void S32LCD(s32 n)
{
    if(n < 0)
    {
        CharLCD('-');
        U32LCD(0u - (u32)n);       // safe also for the most negative value
    }
    else
    {
        U32LCD((u32)n);
    }
}

// FIX: for negative numbers 'n = fnum' converted a negative float to u32 (undefined).
//      Now the sign is printed and the value made positive first.
void F32LCD(f32 fnum, u8 nDP)
{
    u32 n;
    s32 i;

    if(fnum < 0)
    {
        CharLCD('-');
        fnum = -fnum;
    }
    n = (u32)fnum;
    U32LCD(n);
    CharLCD('.');
    for(i = 0; i < nDP; i++)
    {
        fnum = (fnum - n) * 10;
        n = (u32)fnum;
        CharLCD(n + '0');
    }
}

void HexLCD(u32 n)
{
    u8  a[8], rem;
    s32 i = 0;

    if(n == 0)
    {
        CharLCD('0');
    }
    else
    {
        while(n)
        {
            rem = n % 16;
            rem += (rem < 10) ? '0' : ('A' - 10);
            a[i++] = rem;
            n /= 16;
        }
        for(--i; i >= 0; i--)
            CharLCD(a[i]);
    }
}

void OctLCD(u32 n)                 // FIX: new, was declared in the header but missing
{
    u8  a[11];
    s32 i = 0;

    if(n == 0)
    {
        CharLCD('0');
    }
    else
    {
        while(n)
        {
            a[i++] = (n % 8) + '0';
            n /= 8;
        }
        for(--i; i >= 0; i--)
            CharLCD(a[i]);
    }
}

void BinLCD(u32 n, u8 nbd)
{
    s32 i;
    for(i = (nbd - 1); i >= 0; i--)
        CharLCD(((n >> i) & 1) + '0');
}

// Store custom character patterns into CGRAM.
// FIX: 'nb' is now the NUMBER OF BYTES (loop is i < nb). The old loop was i <= nb,
//      so the caller had to pass 15 to write 16 bytes - an off-by-one trap.
//      2 characters x 8 rows = BuildCGRAM(table, 16).
void BuildCGRAM(u8 *p, u8 nb)
{
    u8 i;

    CmdLCD(GOTO_CGRAM);
    for(i = 0; i < nb; i++)
        CharLCD(p[i]);
    CmdLCD(GOTO_LINE1_POS0);       // back to DDRAM (any DDRAM address ends CGRAM mode)
}
