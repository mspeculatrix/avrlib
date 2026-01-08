#include "SB_modulelib_ng.h"

/*------------------------------------------------------------------------------
---- PUBLIC METHODS                                                         ----
------------------------------------------------------------------------------*/
SensorBusModule::SensorBusModule(volatile PORT_t* port,
	uint8_t clkPin_pm, uint8_t actPin_pm,
	uint8_t datPin_pm, volatile uint8_t* datCtrl)
	: SB_Device(port, clkPin_pm, actPin_pm, port), // call parent constructor
	_dat(datPin_pm),
	_datCtrl(datCtrl) {

	_setDefaults();
	_timeoutCounterInit();

}

// This will be called by methods specific to the instance.
uint8_t SensorBusModule::sendMessage(uint8_t* msgBuf) {
	// Assumes all signals start as inputs
	uint8_t error = 0;
	uint8_t msgLen = msgBuf[0];
	// Disable interrupts on DAT
	_datPinCtrlBase[*_datCtrl] &= ~PORT_ISC_gm;  			// Clear ISC bits
	_datPinCtrlBase[*_datCtrl] |= PORT_ISC_INTDISABLE_gc;  // Disable interrupts

	bool bus_inactive = _waitForState(_port, _act, HIGH, STD_TO_TICKS, STD_TO_LOOPS);

	if (bus_inactive) {
		_port->OUTCLR = _dat | _act;				// Set both low
		_port->DIRSET = _dat | _act;				// Set both to outputs
		bool clk_strobed = _waitForState(_port, _clk, LOW, STD_TO_TICKS, STD_TO_LOOPS);
		if (clk_strobed) {
			_port->OUTSET = _dat;				// Take high
			_delay_us(START_TRANSMISSION_PAUSE);
			_port->OUTSET = _clk;				// Take high
			_port->DIRSET = _clk;						// & output

			// EXCHANGE LOOP
			for (uint8_t i = 0; i < msgLen; i++) {
				for (uint8_t bit = 0; bit < 8; bit++) {
					if (msgBuf[i] & (1 << bit)) {
						_port->OUTSET = _dat;
					} else {
						_port->OUTCLR = _dat;
					}
					_delay_us(BIT_PAUSE);
					_port->OUTCLR = _clk;
					_delay_us(BIT_PAUSE);
					_port->OUTSET = _clk;
					// _delay_us(SB_BIT_PAUSE);
				}
				_delay_us(BYTE_PAUSE);
			}
			_port->DIRCLR = _clk;
		} else {
			error = ERR_CLK_STROBE;
		}
	} else {
		error = ERR_TO_BUS;
	}
	// RESET TO DEFAULTS
	_setDefaults();
	return error;
}

/*------------------------------------------------------------------------------
---- PROTECTED METHODS                                                      ----
------------------------------------------------------------------------------*/

/**
 * Overloads parent method
 */
void SensorBusModule::_setDefaults(void) {
	_port->DIRSET = _clk | _dat | _act; // all lines to HIGH
	_port->DIRCLR = _clk | _dat | _act; // all lines to inputs
	// Enable interrupts on data line
	_datPinCtrlBase[*_datCtrl] = PORT_PULLUPEN_bm | PORT_ISC_FALLING_gc;
}
