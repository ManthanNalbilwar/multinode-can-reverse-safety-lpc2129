// Common macros used for setting, clearing, reading and modifying
// individual bits, nibbles and bytes in LPC21xx registers.
// FIX: every macro argument is now wrapped in parentheses. Before, a call like
//      SETBIT(x, a+1) or WBYTE(r, p, v|1) expanded with the wrong precedence.
#ifndef DEFINE_H
#define DEFINE_H

#define SETBIT(WORD,BITPOS)    ((WORD) |= (1UL<<(BITPOS)))
#define CLRBIT(WORD,BITPOS)    ((WORD) &= ~(1UL<<(BITPOS)))

// Write (not OR) a 1 at BITPOS. Use with IOSET0 (drive pin high).
#define SSETBIT(WORD,BITPOS)   ((WORD) = (1UL<<(BITPOS)))
// Write a 1 at BITPOS. Use with IOCLR0 (drive pin low). (Old comment said "write a value".)
#define SCLRBIT(WORD,BITPOS)   ((WORD) = (1UL<<(BITPOS)))

#define WBYTE(WORD,SBITPOS,BYTE)     ((WORD) = ((WORD) & ~(0xFFUL<<(SBITPOS))) | ((u32)(BYTE)<<(SBITPOS)))
#define WNIBBLE(WORD,SBITPOS,NIBBLE) ((WORD) = ((WORD) & ~(0x0FUL<<(SBITPOS))) | ((u32)(NIBBLE)<<(SBITPOS)))
#define WBIT(WORD,SBITPOS,BIT)       ((WORD) = ((WORD) & ~(1UL<<(SBITPOS))) | ((u32)(BIT)<<(SBITPOS)))

#define RBIT(WORD,SBITPOS)     (((WORD)>>(SBITPOS))&1)
#define RNIBBLE(WORD,SBITPOS)  (((WORD)>>(SBITPOS))&15)
#define RBYTE(WORD,SBITPOS)    (((WORD)>>(SBITPOS))&255)

#define READWRITERBIT2(DWORD,DBIT,SWORD,SBIT) \
        ((DWORD) = ((DWORD) & ~(1UL<<(DBIT))) | ((((SWORD)>>(SBIT))&1UL)<<(DBIT)))
#define READWRITERBIT(DWORD,DBIT,SBIT) \
        ((DWORD) = ((DWORD) & ~(1UL<<(DBIT))) | ((((DWORD)>>(SBIT))&1UL)<<(DBIT)))

#define CPLBIT(WORD,BITPOS)    ((WORD) ^= (1UL<<(BITPOS)))

#endif
