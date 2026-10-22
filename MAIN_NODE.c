// main_node.c
// Main Node of the CAN-Based Vehicle Safety & Monitoring System.
// Engine temperature, indicator control, CAN communication, mode display, reverse alert.
//
// CAN message map (125 kbps, 11-bit IDs)
//   0x101  Main    -> Indicator : DLC 1, byte0 = 'L' / 'R' / 'S'
//   0x102  Main    -> Reverse   : DLC 1, byte0 = mode (0 Forward, 1 Reverse)
//   0x201  Reverse -> Main      : DLC 3, byte0..1 = distance in cm (little endian),
//                                        byte2    = alert (0 safe, 1 too close, 2 sensor error)
//
// Main-loop design (FIX): the loop never blocks for long. The DS18B20 conversion (750 ms)
// is started and collected later (non-blocking), CAN is drained completely each pass,
// and the LCD is rewritten with fixed-width text (no "clear then redraw" flicker).

#include <LPC21xx.h>
#include "major_type.h"
#include "major_lcd.h"
#include "major_lcd_defines.h"
#include "DELAY_MAJOR.h"
#include "DS18B20_MAJOR.h"
#include "can_major.h"
#include "can_defines_major.h"
#include "buzzer_major.h"
#include "interrupt_major.h"

#define ID_INDICATOR   0x101
#define ID_MODE        0x102
#define ID_REVERSE     0x201

#define ALERT_DIST_CM  20
#define RX_TIMEOUT_MS  2000UL     // Reverse node silent for this long -> "NO DATA"
#define TEMP_PERIOD_MS 1000UL     // start a new temperature conversion every second
#define TEMP_CONV_MS   800UL      // DS18B20 needs <= 750 ms

/* ---- variables shared with the interrupt routines (volatile) ---- */
volatile u8 mode          = 0;     // 0 = Forward, 1 = Reverse
volatile u8 left_flag     = 0;
volatile u8 right_flag    = 0;
volatile u8 tx_ind_flag   = 0;     // set by EINT1/EINT2 -> send 0x101
volatile u8 tx_mode_flag  = 0;     // set by EINT0       -> send 0x102
volatile u8 indicator_cmd = 'S';   // 'L', 'R', 'S'

/* ---- data received from the Reverse Node ---- */
static u32 dist          = 0;      // cm
static u8  reverse_alert = 0;      // 0 safe, 1 too close, 2 sensor error
static u8  rx_valid      = 0;      // 1 = at least one 0x201 frame since entering Reverse
static u32 rx_time       = 0;      // millis() of the last 0x201 frame

/* ---- temperature ---- */
static int temp_raw   = 0;         // 1 LSB = 1/16 C
static u8  temp_state = 0;         // 0 = no reading yet, 1 = valid, 2 = sensor fail

static u8 blink = 0;

// Two custom 5x8 characters (indicator arrow halves) -> CGRAM chars 0 and 1
u8 indicator_lut[16] =
{
    0x01,0x03,0x07,0x0F,0x1F,0x0F,0x07,0x03,
    0x10,0x18,0x1C,0x1E,0x1F,0x1E,0x1C,0x18
};

static void Pad(u8 n)              // print n spaces (clears leftovers of the previous text)
{
    while(n--) CharLCD(' ');
}

// Read every pending CAN frame (FIX: old code read only ONE frame per 1.2 s loop, so
// the 0x201 frames piled up and the displayed distance was seconds old).
static void can_service(void)
{
    struct CAN_Frame rx;

    while(C1GSR & RBS_BIT_READ)
    {
        CAN1_Rx(&rx);
        if(rx.ID == ID_REVERSE)
        {
            // FIX: the old protocol (DLC 2, alert in Data2) never delivered Data2:
            //      only DLC bytes are transmitted. Alert now travels in byte 2 of Data1.
            dist          = rx.Data1 & 0xFFFF;
            reverse_alert = (u8)((rx.Data1 >> 16) & 0xFF);
            rx_valid      = 1;
            rx_time       = millis();
        }
    }
}

static void can_send_pending(void)
{
    struct CAN_Frame tx;

    tx.vbf.RTR = 0;
    tx.vbf.DLC = 1;
    tx.Data2   = 0;                 // FIX: was uninitialised stack data

    if(tx_ind_flag)
    {
        tx_ind_flag = 0;
        tx.ID    = ID_INDICATOR;
        tx.Data1 = indicator_cmd;
        if(!CAN1_Tx(tx))
            tx_ind_flag = 1;        // not delivered (no ACK) -> retry next pass
    }

    if(tx_mode_flag)
    {
        tx_mode_flag = 0;
        rx_valid = 0;               // old distance is meaningless after a mode change
        tx.ID    = ID_MODE;
        tx.Data1 = mode;
        if(!CAN1_Tx(tx))
            tx_mode_flag = 1;
    }
}

static void show_temperature(void)
{
    int  t;
    u8   neg;

    CmdLCD(GOTO_LINE1_POS0);

    if(temp_state == 2)
    {
        StrLCD("SENSOR FAIL         ");
    }
    else if(temp_state == 0)
    {
        StrLCD("ENG TEMP: --        ");
    }
    else
    {
        t   = temp_raw;
        neg = (t < 0);
        if(neg) t = -t;                         // FIX: negative temperatures were shown as ~4000 C

        StrLCD("ENG TEMP:");
        if(neg) CharLCD('-');
        U32LCD(t >> 4);                         // integer part
        CharLCD('.');
        CharLCD('0' + (((t & 0x0F) * 10) >> 4));// FIX: real tenths digit (old code only showed .0 or .5)
        CharLCD(223);                           // degree symbol
        CharLCD('C');
        Pad(4);
    }
}

static void show_indicator(void)
{
    CmdLCD(GOTO_LINE2_POS0);

    if(indicator_cmd == 'L')
    {
        CharLCD(blink ? 0 : ' ');
        CharLCD(1);
        StrLCD(" LEFT IND");  Pad(9);
    }
    else if(indicator_cmd == 'R')
    {
        CharLCD(0);
        CharLCD(blink ? 1 : ' ');
        StrLCD(" RIGHT IND"); Pad(8);
    }
    else
    {
        CharLCD(0);
        CharLCD(1);
        StrLCD(" IND OFF");   Pad(10);
    }
}

static void show_mode_and_distance(void)
{
    u8 buzz = 0;

    CmdLCD(GOTO_LINE3_POS0);
    StrLCD(mode ? "MODE: REVERSE      " : "MODE: FORWARD      ");

    CmdLCD(GOTO_LINE4_POS0);
    if(!mode)
    {
        StrLCD("FORWARD DRIVE MODE  ");
    }
    else if(!rx_valid)
    {
        StrLCD("DIST:--- WAITING    ");
    }
    else if((millis() - rx_time) > RX_TIMEOUT_MS)
    {
        StrLCD("NO DATA FROM SENSOR ");   // FIX: lost CAN link used to leave a stale "SAFE" on screen
        buzz = 1;
    }
    else
    {
        StrLCD("DIST:");
        U32LCDW(dist, 3);                  // FIX: fixed width -> no leftover digits
        StrLCD("cm ");
        if(reverse_alert == 1)      { StrLCD("ALERT   "); buzz = 1; }
        else if(reverse_alert == 2) { StrLCD("SENS ERR"); buzz = 1; }  // FIX: failed sensor (999) used to display "SAFE"
        else                        { StrLCD("SAFE    "); }
        Pad(1);
    }

    if(buzz) Buzzer_ON(); else Buzzer_OFF();
}

int main(void)
{
    u32 t_last_start = 0, t_conv_start = 0;
    u8  converting = 0;
    int r;

    InitLCD();
    Init_CAN1();
    Buzzer_init();
    interrupt_enable();
    BuildCGRAM(indicator_lut, 16);    // 2 chars x 8 rows = 16 bytes

    CmdLCD(GOTO_LINE1_POS0);  StrLCD("Vehicle Safety And");
    CmdLCD(GOTO_LINE2_POS0);  StrLCD("Monitoring System ");
    delay_ms(2000);
    CmdLCD(CLEAR_LCD);

    while(1)
    {
        /* 1. CAN */
        can_service();
        can_send_pending();

        /* 2. Temperature: start a conversion, collect it ~800 ms later (never blocks) */
        if(!converting)
        {
            if((millis() - t_last_start) >= TEMP_PERIOD_MS)
            {
                t_last_start = millis();
                if(DS18B20_Start()) { converting = 1; t_conv_start = millis(); }
                else                { temp_state = 2; }
            }
        }
        else if((millis() - t_conv_start) >= TEMP_CONV_MS)
        {
            r = DS18B20_Read();
            converting = 0;
            if(r == DS18B20_ERROR) temp_state = 2;
            else                   { temp_raw = r; temp_state = 1; }
        }

        /* 3. Display */
        blink = !blink;
        show_temperature();
        show_indicator();
        can_service();                       // LCD writes are slow: drain CAN again
        show_mode_and_distance();

        delay_ms(50);
    }
}
