#include "SB_modlib_avrng.h"

/*------------------------------------------------------------------------------
---- PUBLIC METHODS                                                         ----
------------------------------------------------------------------------------*/
SensorBusModule::SensorBusModule(volatile PORT_t* port,
	uint8_t datPin_pm, uint8_t clkPin_pm, uint8_t actPin_pm,
	volatile uint8_t* datCtrl, uint8_t msgBufLen)
	: _port(port), _dat(datPin_pm), _clk(clkPin_pm),
	_act(actPin_pm), _datCtrl(datCtrl), _msgBufLen(msgBufLen) {
	init();
}

void SensorBusModule::init() {
	// Initial configuration
	// base address of PIN0CTRL, from which we offset to desired pin
	_pinCtrlBase = &(_port->PIN0CTRL);
	_setDefaults();
	_timeoutCounterInit();
}

// This will be called by methods specific to the instance.
uint8_t SensorBusModule::sendMessage(uint8_t* msgBuf) {
	// Assumes all signals start as inputs
	uint8_t error = 0;
	uint8_t msgLen = msgBuf[0];
	// Disable interrupts on DAT
	_pinCtrlBase[*_datCtrl] &= ~PORT_ISC_gm;  			// Clear ISC bits
	_pinCtrlBase[*_datCtrl] |= PORT_ISC_INTDISABLE_gc;  // Disable interrupts

	bool bus_inactive = _waitForState(_port, _act, HIGH, 20000, SB_MAX_TO_LOOP_COUNT);

	if (bus_inactive) {
		_port->OUTCLR = _dat | _act;				// Set both low
		_port->DIRSET = _dat | _act;				// Set both to outputs
		bool clk_strobed = _waitForState(_port, _clk, LOW, 0xFFFF, 10);
		if (clk_strobed) {
			_port->OUTSET = _dat;				// Take high
			_delay_us(SB_START_TRANSMISSION_PAUSE);
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
					_delay_us(SB_BIT_PAUSE);
					_port->OUTCLR = _clk;
					_delay_us(SB_BIT_PAUSE);
					_port->OUTSET = _clk;
					// _delay_us(SB_BIT_PAUSE);
				}
				_delay_us(SB_BYTE_PAUSE);
			}
			_port->DIRCLR = _clk;
		} else {
			error = SB_ERR_CLK_STROBE;
		}
	} else {
		error = SB_ERR_TO_BUS;
	}
	// RESET TO DEFAULTS
	_setDefaults();
	return error;
}

void SensorBusModule::setReceiveMode(void) {

}

void SensorBusModule::strobeClk(void) {
	_port->DIRSET = _clk;
	_port->OUTCLR = _clk;
	_delay_us(SB_CLK_STROBE_DURATION);
	_port->OUTSET = _clk;
	_port->DIRCLR = _clk;
}

/*------------------------------------------------------------------------------
---- PROTECTED METHODS                                                      ----
------------------------------------------------------------------------------*/

void SensorBusModule::_setDefaults(void) {
	_port->DIRCLR = _clk | _dat | _act; // all lines to inputs
	// Enable interrupts on data line
	_pinCtrlBase[*_datCtrl] = PORT_PULLUPEN_bm | PORT_ISC_FALLING_gc;
}

void SensorBusModule::_timeoutCounterInit(void) {
	TCB0.CTRLA = 0;	// Disable timer before configuration
	TCB0.CTRLA = TCB_CLKSEL_CLKDIV2_gc;	// Select clock source
	TCB0.CTRLB = TCB_CNTMODE_INT_gc;	// periodic interrupts, ints disabled
	TCB0.INTFLAGS = TCB_CAPT_bm;		// Clear any pending interrupt flag
}

void SensorBusModule::_timeoutCounterStart(uint16_t timeoutValue) {
	TCB0.CNT = 0;     // Reset count
	TCB0.CCMP = timeoutValue;			// Load rollover value
	TCB0.CTRLA |= TCB_ENABLE_bm; 		// Enable the timer
}

void SensorBusModule::_timeoutCounterStop(void) {
	TCB0.INTFLAGS = TCB_CAPT_bm;
	TCB0.CTRLA &= ~TCB_ENABLE_bm;
}

bool SensorBusModule::_waitForState(volatile PORT_t* port, uint8_t pin, uint8_t state, uint16_t timeoutTicks, uint8_t max_loops) {
	bool stateAchieved = false;
	bool max_loop_limit = false;
	uint8_t loop_count = 0;
	_timeoutCounterStart(timeoutTicks);
	while (!stateAchieved && !max_loop_limit) {
		bool timed_out = false;
		while (!stateAchieved && !timed_out) {
			uint8_t pinState = port->IN & pin;
			if ((state == HIGH && pinState) || (state == LOW && !pinState)) {
				stateAchieved = true;
			} else if (TCB0.INTFLAGS & TCB_CAPT_bm) {	// overflow happened
				TCB0.INTFLAGS = TCB_CAPT_bm;			// clear flag
				timed_out = true;
			}
		}
		if (!stateAchieved) {
			if (loop_count >= max_loops) {
				max_loop_limit = true;
			} else {
				loop_count++;
			}
		}
	}
	_timeoutCounterStop();
	return stateAchieved;
}
