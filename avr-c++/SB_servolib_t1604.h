/**

This library is entirely oriented towards my SensorBus Servo board which
uses the ATtiny1604 running at 20MHz.

It's designed to work with servos operating on a 50Hz frequency (20ms period)
with a 180 degree range.

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

namespace SB_Servo {

	const uint8_t SB_SERVO_A = 0;
	const uint8_t SB_SERVO_B = 1;

	/**
	 * @brief Class for controlling the SensorBus Servo board.
	 *
	 * Child class of SB_Module.
	 * The board can control two servos.
	 */
	class SB_Servo_t1604 : public SB_Module {

	public:
		// CONSTRUCTOR
		SB_Servo_t1604(PORT_t* sbPort, uint8_t sbClk, uint8_t sbAct, uint8_t sbDat,
			PORT_t* datPort, volatile uint8_t* sbDatCtrl);

		SB_Servo_t1604(uint16_t minA, uint16_t maxA,
			PORT_t* sbPort, uint8_t sbClk, uint8_t sbAct, uint8_t sbDat,
			PORT_t* datPort, volatile uint8_t* sbDatCtrl);

		SB_Servo_t1604(uint16_t minA, uint16_t maxA,
			uint16_t minB, uint16_t maxB,
			PORT_t* sbPort, uint8_t sbClk, uint8_t sbAct, uint8_t sbDat,
			PORT_t* datPort, volatile uint8_t* sbDatCtrl);

		void begin(void);
		void setAngle(uint8_t pin, uint8_t angle);
		void setMinAngle(uint8_t servo, uint8_t angle);
		void setMaxAngle(uint8_t servo, uint8_t angle);

	protected:

		// Calculation for 20MHz clock with 64 Prescaler:
		// Period = 20,000,000 / 64 / 50Hz = 6250
		const uint16_t _PERIOD_20MS = 6250;
		// Pulse widths in number of ticks - default values
		const uint16_t _TICKS_MIN_DFL = 170; // ~0.55ms (0 degrees)
		const uint16_t _TICKS_MAX_DFL = 745; // ~2.45ms (180 degrees)

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
