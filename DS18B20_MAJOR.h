// DS18B20 (1-Wire) temperature sensor driver - data line on P0.19.
#ifndef DS18B20_H
#define DS18B20_H

// Returned instead of a temperature when the sensor does not answer.
// FIX: the old code used -1, but a raw reading of 0xFFFF (-0.0625 C) is also "-1".
#define DS18B20_ERROR 0x7FFF

unsigned char ResetDS18b20(void);    // 1 = sensor present, 0 = no sensor   (FIX: meaning is now the natural one)
unsigned char ReadBit(void);
void          WriteBit(unsigned char);
unsigned char ReadByte(void);
void          WriteByte(unsigned char);

// Non-blocking pair (NEW) - lets the main loop keep servicing CAN while the sensor converts (~750 ms):
unsigned char DS18B20_Start(void);   // 1 = conversion started, 0 = sensor missing
int           DS18B20_Read(void);    // raw 16-bit signed value (1 LSB = 1/16 C) or DS18B20_ERROR

// Blocking version (starts, waits, reads). Takes up to ~1 s.
int           ReadTemp(void);

#endif
