# CAN-Based Vehicle Safety & Monitoring System (LPC2129)

A three-node vehicle system built on **LPC2129 (ARM7)** boards that talk over a **CAN bus** (125 kbps, MCP2551 transceivers).
It monitors engine temperature, controls turn indicators and warns the driver about obstacles while reversing.

## Nodes
| Node | What it does |
|---|---|
| **Main Node** | Reads engine temperature (DS18B20), shows everything on a 20x4 LCD, reads the three switches (mode / left / right), drives the buzzer, sends commands over CAN |
| **Indicator Node** | Receives the indicator command and runs 8 LEDs as a left / right sweeping turn indicator |
| **Reverse Node** | In reverse mode measures distance with an ultrasonic sensor and sends distance + alert to the Main Node |

## How it works
```
Switch press -> Main Node (EINT ISR sets a flag)
   |-- 0x101 'L'/'R'/'S' --> Indicator Node -> LEDs sweep / off
   '-- 0x102 mode (0/1)  --> Reverse Node   -> starts measuring in reverse mode
Reverse Node -- 0x201 distance + alert --> Main Node -> LCD + buzzer
```

### CAN messages (11-bit IDs, 125 kbps)
| ID | From -> To | DLC | Data |
|---|---|---|---|
| `0x101` | Main -> Indicator | 1 | byte0 = `'L'` left, `'R'` right, `'S'` off |
| `0x102` | Main -> Reverse | 1 | byte0 = mode (0 forward, 1 reverse) |
| `0x201` | Reverse -> Main | 3 | byte0..1 = distance in cm (little endian), byte2 = alert: 0 safe, 1 closer than 20 cm, 2 sensor error |

## Hardware
LPC2129 board x3 (Vector LPC2129 CAN development board), MCP2551 CAN transceiver x3, 20x4 character LCD, DS18B20, HC-SR04 / HC-SR05 ultrasonic sensor, buzzer, 8 LEDs, 3 push buttons, USB-UART (Flash Magic).

### Pin map
**Main Node**
| Function | Pin |
|---|---|
| LCD D0-D7 | P0.0 - P0.7 |
| LCD RW / RS / EN | P0.8 / P0.9 / P0.10 |
| Mode switch (EINT0) | P0.16 |
| Left switch (EINT1) | P0.14 |
| Right switch (EINT2) | P0.15 |
| DS18B20 DQ (4.7k pull-up to 3.3 V) | P0.19 |
| Buzzer (active-low) | P0.20 |
| CAN1 | TD1, P0.25 (RD1) |

**Indicator Node:** 8 LEDs (active-low) on P0.0 - P0.7, CAN1 on TD1 / P0.25.
**Reverse Node:** ultrasonic Trig P0.20, Echo P0.19, CAN1 on TD1 / P0.25.

### Bus wiring
CANH-CANH, CANL-CANL between the boards, common GND, 120 ohm termination only at the two ends of the bus.
Board jumpers: TD1 and RD1 ON, P0.25 ON, P0.8 / P0.9 UART1 jumpers removed (used for LCD RW / RS on the Main Node), P0.0 / P0.1 UART0 jumpers only while flashing, ISP switch on EXE to run.

## Build and flash
1. Create one Keil uVision project per node (target LPC2129) with the board's `Startup.s`.
2. Add all files from that node's `src/` and set `inc/` as include path.
3. Rebuild all, flash the `.hex` with Flash Magic (ISP switch on LOAD), then switch to EXE and reset.

`clock_major.h` assumes CCLK = 60 MHz; PCLK is read from `VPBDIV` at run time. If your CCLK differs, change `CCLK` there and recalibrate the delay loops in `delay_MAJOR.c`.

## Repository layout
```
MainNode/       inc/  src/
IndicatorNode/  inc/  src/
ReverseNode/    inc/  src/
docs/FIXES.md   list of bugs found and fixed compared to the reference
```
The CAN driver, delay and type files are identical in all three nodes.

## Credits
Based on the reference project from Vector India:
https://github.com/varalakshmi68/CAN-BASED-VEHICLE-SAFETY-MONITORING-SYSTEM

I reviewed that code, fixed the bugs listed in [`docs/FIXES.md`](docs/FIXES.md) (for example the inverted DS18B20 conversion wait, the CAN frame whose alert byte was never transmitted, missing switch debounce, blocking CAN transmit) and moved the Main Node pins so the LCD data bus sits on contiguous pins.

## Author
Manthan
