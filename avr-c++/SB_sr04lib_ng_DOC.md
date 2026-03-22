# SB_MOD_SR04_t1604

Firmware for the SR04 ultrasonic rangefinder sensor module using SensorBus.

This is based around the ATtiny1604 microcontroller.

This code depends on a number of my libaries, which you can find [on GitHub](https://github.com/mspeculatrix/avrlib).

The code makes use of all three of the ATtiny1604's timers.

- TCA - used by the sensor code (`SB_sr04lib_avrng`) to time the echoes from pings.
- TCB - used by the SensorBus library (`SBlib_modlib_avrng`) for signal timeouts.
- RTC/PIT - creates an interrupt roughly once per second to tell the module to perform a measurement (ie, a ping).

## POWER

The board is powered at +5V DC via the SensorBus connection.

## ATtiny1604 PIN ASSIGNMENTS

| NAME | No. | Description                |
|:----:|:---:|----------------------------|
| PA0  | 10  | UPDI programming           |
| PA1  | 11  | `SB_ACT` SensorBus signal  |
| PA2  | 12  | `SB_DAT` SensorBus signal  |
| PA3  | 13  | `SB_CLK` SensorBus signal  |
| PA4  | 2   | SR04 sensor Trigger signal |
| PA5  | 3   | SR04 sensor Echo signal    |
| PA6  | 4   | [--reserved--]             |
| PA7  | 5   | [--reserved--]             |
|      |     |                            |
| PB0  | 9   | Alert LED                  |
| PB1  | 8   | Activity LED               |
| PB2  | 7   | Serial TX                  |
| PB2  | 6   | Serial RX                  |

## UPDI PROGRAMMING

Two ports are provided for UPDI programming and debugging.

There is a 6-pin (2x3) IDC header compatible with the Atmel-ICE programmer.

There is also a 3-pin header for serial programming (eg, using a USB-to-serial cable). Connect the signals in the following way:

| Serial cable | Board Header  |
|:------------:|:-------------:|
| RX           | UPDI RX       |
| TX           | TX            |
| GND          | GND           |

Note: there is no crossing over of TX & RX signals as you would with a standard serial connection.

## SERIAL PORT

There's a standard UART-style serial port with TX, RX and GND signals. The signal levels are +5V.

## LEDs

The two LEDs are marked `ACT` (blue) and `ALERT` (red), but these are just suggested uses. There's nothing intrinsic about their functions.
