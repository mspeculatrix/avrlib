# SB_SERVOLIB_T1604

SensorBus servo module for controlling the pan and tilt servos on the sensor head.

The tilt sensor has only about a 90-degree range, so the constructor values reflect this.

SensorBus messages should have the following format:

| Byte | Value                             |
|:----:|-----------------------------------|
| 0    | 4                                 |
| 1    | SBMSG_SET_PARAM                   |
| 2    | Angle for Servo A (PAN) - 0-180   |
| 3    | Angle for Servo B (TILT) - 0-180  |

## ATtiny1604 PINOUT

| FUNC    | MCU PIN | IC PIN | - | IC PIN | MCU PIN | FUNC    |
|:-------:|:-------:|:------:|:-:|:------:|:-------:|:-------:|
| VCC     | VCC     | 1      | - | 14     | GND     | GND     |
| LED_A   | PA4     | 2      | - | 13     | PA3     | SB_CLK  |
| USER IO | PA5     | 3      | - | 12     | PA2     | SB_ACT  |
| USER_IO | PA6     | 4      | - | 11     | PA1     | SB_DAT  |
| LED_B   | PA7     | 5      | - | 10     | PA0     | UPDI    |
| TX      | PB3     | 6      | - | 9      | PB0     | SERVO_A |
| RX      | PB2     | 7      | - | 8      | PB1     | SERVO_B |

## SERIAL PORT

There's a standard UART-style serial port with TX, RX and GND signals. The signal levels are +5V.
