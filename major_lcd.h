// Function declarations used to control the LCD display.
#ifndef LCD_H
#define LCD_H

void WriteLCD(unsigned char data);
void CmdLCD(unsigned char cmd);
void CharLCD(unsigned char ascii);
void InitLCD(void);
void StrLCD(char *p);
void U32LCD(unsigned int n);
void U32LCDW(unsigned int n, unsigned char width); // NEW: number, right-padded with spaces to 'width' (no leftover digits)
void F32LCD(float fnum, unsigned char ndp);
void S32LCD(int n);
void HexLCD(unsigned int n);
void BinLCD(unsigned int n, unsigned char nbd);
void OctLCD(unsigned int n);                       // FIX: was declared but never defined
void BuildCGRAM(unsigned char *p, unsigned char nb); // nb = number of BYTES to write (FIX: see lcd_MAJOR.c)

#endif
