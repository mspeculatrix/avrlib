/* SB_nodelib_ng.cpp */

#include "SB_nodelib_ng.h"

SB_Node::SB_Node(volatile PORT_t* port,
	uint8_t clkPin_pm, uint8_t actPin_pm, PORT_t* datPort)
	: SB_Device(port, clkPin_pm, actPin_pm, datPort) // call parent constructor
{
	_setDefaults();
	_timeoutCounterInit();
}

/**
 * Overrides parent method
 */
void SB_Node::_setDefaults(void) {
	_port->OUTSET = _clk | _act; 		// set to HIGH as default state
	_port->DIRCLR = _clk | _act; 		// set to INPUTS
	_datPort->OUTSET = 0xFF;			// Set all DAT pins high
	_datPort->DIRCLR = 0xFF;			// All DAT pins to inputs
	// Set pullups and falling edge interrupts on DAT pins
	for (uint8_t i = 0; i < 8; i++) {
		*(_datPinCtrlBase + i) = PORT_PULLUPEN_bm | PORT_ISC_FALLING_gc;
	}
}
