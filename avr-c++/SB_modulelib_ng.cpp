/* SB_modulelib_ng.cpp */

#include "SB_modulelib_ng.h"

/*------------------------------------------------------------------------------
---- PUBLIC METHODS                                                         ----
------------------------------------------------------------------------------*/
SensorBusModule::SensorBusModule(volatile PORT_t* port,
	uint8_t clkPin_pm, uint8_t actPin_pm,
	uint8_t datPin_pm, PORT_t* datPort, volatile uint8_t* datCtrl)
	: SB_Device(port, clkPin_pm, actPin_pm, datPort), // call parent constructor
	_dat(datPin_pm),
	_datCtrl(datCtrl) {

	_setDefaults();
	_timeoutCounterInit();
}

err_code SensorBusModule::sendMessage() {
	err_code error = SB_Device::sendMessage(_dat);
	return error;
}

/*------------------------------------------------------------------------------
---- PROTECTED METHODS                                                      ----
------------------------------------------------------------------------------*/

/**
 * Overrides parent method
 */
void SensorBusModule::_setDefaults(void) {
	_port->OUTSET = _clk | _act; 	// Set to HIGH
	_port->DIRCLR = _clk | _act; 	// Set to INPUTs
	_datPort->OUTSET = _dat;		// Set HIGH as default
	_datPort->DIRCLR = _dat;		// Set to INPUT
	// Enable interrupts on data line
	_datPinCtrlBase[*_datCtrl] = PORT_PULLUPEN_bm | PORT_ISC_FALLING_gc;
}
