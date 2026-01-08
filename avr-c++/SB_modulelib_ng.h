/*
 *
 * Main class and library for SensorBus modules.
 *
 * Uses the timeout counter B (TCB).
 *
 * Requires SBlib_defines.h in same dir
 *
 * To use this you need to externally define some stuff, such as:
 *
 * #define SB_PORT PORTD
 * #define SB_DAT PIN0_bm // Chip Enable (Active Low: /CE)
 * #define SB_CLK PIN1_bm // Output Enable (Active Low: /OE)
 * #define SB_ACT PIN2_bm // Output Enable (Active Low: /OE)
 * #define SB_PORT_INT_VEC PORTD_PORT_vect
 * #define SB_DAT_CTRL PIN0CTRL // only for non-class stuff
 *
 * Global var:
 * bool commRequest = false;
*/

#ifndef __SB_MODULELIB_NG__
#define __SB_MODULELIB_NG__

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include <smd_std_macros.h>
#include <SB_lib_defines.h>

#include <SB_devicelib_ng.h>

/*
The main program needs to provide an interrupt service routine (ISR)
suitably configured for the Port/Pin used for the Data line. Eg:

ISR(PORTD_PORT_vect) {
	if (PORTD.INTFLAGS & PIN0_bm) {		// Check if DAT triggered
		PORTD.INTFLAGS = PIN0_bm;		// Clear interrupt flag
		commRequest = true;				// Set event flag
	}
}
*/

using namespace SensorBus;

class SensorBusModule : public SB_Device
{
public:
	SensorBusModule(volatile PORT_t* port,
		uint8_t clkPin_pm, uint8_t actPin_pm, uint8_t datPin_pm,
		volatile uint8_t* datCtrl);
	uint8_t sendMessage(uint8_t* msgBuf);

protected:
	// VARIABLES
	volatile uint8_t _dat;
	volatile uint8_t* _datCtrl;

	// METHODS
	void _setDefaults(); // Overloads parent method
};

#endif