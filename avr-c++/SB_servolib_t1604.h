/**

This library is entirely oriented towards my SensorBus Servo board which
uses the ATtiny1604.

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

		void begin(void);
		void setAngle(uint8_t pin, uint8_t angle);

	protected:

		// Calculation for 20MHz clock with 64 Prescaler:
		// Period = 20,000,000 / 64 / 50Hz = 6250
		const uint16_t _PERIOD_20MS = 6250;

		// Pulse widths in number of ticks
		const uint16_t _TICKS_MIN = 170; // ~0.55ms (0 degrees)
		const uint16_t _TICKS_MAX = 745; // ~2.45ms (180 degrees)

	};

} // namespace

#endif
