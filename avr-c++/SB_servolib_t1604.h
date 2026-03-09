/**

This library is entirely oriented towards my SensorBus Servo board which
uses the ATtiny1604 running at 20MHz.

It's designed to work with servos operating on a 50Hz frequency (20ms period)
with a 180 degree range.

A message of type SBMSG_SET_PARAM has two params - the angles for Servo A and
Servo B respectively. If the value is 255, it means 'no change'.

*/

#ifndef __SB_SERVOLIB_T1604__
#define __SB_SERVOLIB_T1604__

// Ensure we have io/sfr/pindefs loaded
#ifndef   _AVR_IO_H_
#include  <avr/io.h>
#endif

// #include <stdlib.h>
// #include <avr/interrupt.h>
// #include <util/delay.h>
#include <SB_lib_defines.h>
#include <SB_modulelib_ng.h>
#include <smd_ng_serial.h>

namespace SB_Servo {

	/**
	 * @brief Class for controlling the SensorBus Servo board.
	 *
	 * Child class of SB_Module.
	 * The board can control two servos.
	 */
	class SB_Servo_t1604 : public SB_Module {

	public:
		static constexpr uint8_t SERVO_A = 0;
		static constexpr uint8_t SERVO_B = 1;
		static constexpr uint8_t LED_A = 0;
		static constexpr uint8_t LED_B = 1;

		SMD_NG_Serial serial = SMD_NG_Serial(57600, &PORTB, PIN2_bm, PIN3_bm);

		// CONSTRUCTORS
		SB_Servo_t1604(void);
		SB_Servo_t1604(uint16_t minA, uint16_t maxA);
		SB_Servo_t1604(uint16_t minA, uint16_t maxA,
			uint16_t minB, uint16_t maxB);

		// METHODS
		void begin(void);
		// void printBuf(uint8_t* buf);
		void printMsg(uint8_t* buf);
		SensorBus::err_code recvMessage(void); // overwrites parent func
		void respondToMessage(void);
		void setAngle(uint8_t pin, uint8_t angle);
		void setLED(uint8_t led, uint8_t state);
		void setMinAngle(uint8_t servo, uint8_t angle);
		void setMaxAngle(uint8_t servo, uint8_t angle);
		void toggleLED(uint8_t led);

	protected:

		static inline PORT_t* const _SB_PORT = &PORTA;
		static constexpr uint8_t _SB_CLK = PIN3_bm;
		static constexpr uint8_t _SB_ACT = PIN2_bm;
		static inline PORT_t* const _SB_DATPORT = &PORTA;
		static constexpr uint8_t _SB_DAT = PIN1_bm;
		static inline volatile uint8_t* const _SB_DATCTRL = &PORTA.PIN1CTRL;
		// static constexpr uint8_t _SB_DAT_ISR_VEC = PORTA_PORT_vect;

		static constexpr uint8_t _SERVO_A_PIN = PIN0_bm;
		static constexpr uint8_t _SERVO_B_PIN = PIN1_bm;
		static constexpr uint8_t _LED_A_PIN = PIN4_bm;
		static constexpr uint8_t _LED_B_PIN = PIN7_bm;
		static constexpr uint8_t _LED_PINS[2] = { _LED_A_PIN, _LED_B_PIN };

		// Calculation for 20MHz clock with 64 Prescaler:
		// Period = 20,000,000 / 64 / 50Hz = 6250
		static constexpr uint16_t _PERIOD_20MS = 6250;
		// Pulse widths in number of ticks - default values for most common
		// 180deg servos.
		static constexpr uint16_t _TICKS_MIN_DFL = 313; // ~1ms (0 degrees)
		static constexpr uint16_t _TICKS_MAX_DFL = 625; // ~2ms (180 degrees)

		// The minimum and maximum acceptable values for the angle. We're
		// assuming a standard servo capable of 180 degrees, but it might be
		// useful to restrict the available range. You do this using the
		// setMinAngle() and setMaxAngle() methods.
		volatile uint8_t _angleMin_A = 0;
		volatile uint8_t _angleMax_A = 180;
		volatile uint8_t _angleMin_B = 0;
		volatile uint8_t _angleMax_B = 180;

		// These values are used in determining the duty cycle for a given
		// angle. We have the default values, but many servos will need to be
		// adjusted (using params in the constructors).
		volatile uint16_t _ticksMin_A = _TICKS_MIN_DFL;
		volatile uint16_t _ticksMax_A = _TICKS_MAX_DFL;
		volatile uint16_t _ticksMin_B = _TICKS_MIN_DFL;
		volatile uint16_t _ticksMax_B = _TICKS_MAX_DFL;

	};

} // namespace

#endif
