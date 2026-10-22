// delay function declarations
// FIX: include guard added; delay_MS() removed (it differed from delay_ms() only by
//      letter case and was 10x shorter - confusing, and never used).
#ifndef DELAY_MAJOR_H
#define DELAY_MAJOR_H

void delay_us(unsigned int dlyus);
void delay_ms(unsigned int dlyms);
void delay_s(unsigned int dlys);

#endif
