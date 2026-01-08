#include "SB_devicelib_ng.h"

using namespace SensorBus;

/*******************************************************************************
*****  PUBLIC                                                              *****
*******************************************************************************/

// CONSTRUCTOR
SB_Device::SB_Device(volatile PORT_t* port, uint8_t clkPin_pm,
	uint8_t actPin_pm, volatile PORT_t* datport)
	: _port(port), _clk(clkPin_pm), _act(actPin_pm), _dat_port(datport) {

	// base address of PIN0CTRL, from which we offset to desired pin
	_datPinCtrlBase = &(_dat_port->PIN0CTRL);
}

uint8_t SB_Device::recvMessage(uint8_t* msgbuf, uint8_t dat) {
	_setReceiveMode(dat);
	uint8_t error = 0;
	uint8_t buf_idx = 0;
	msgbuf[0] = _getByte(dat, &error);
	if (error == 0) {
		buf_idx++;
		// The first received byte denotes the entire message length
		while (buf_idx < msgbuf[0]) {
			msgbuf[buf_idx] = _getByte(dat, &error);
			if (error > 0) break;
			buf_idx++;
		}
	}

	_setDefaults();
	return error;
}

uint8_t SB_Device::sendMessage(uint8_t* msgbuf, uint8_t dat) {
	uint8_t error = 0;
	_setSendMode(dat);



	_setDefaults();
	return error;
}


/*******************************************************************************
*****  PROTECTED                                                           *****
*******************************************************************************/

uint8_t SB_Device::_getByte(uint8_t dat, uint8_t* error) {
	uint8_t byte_val = 0;
	for (uint8_t i = 0; i < 8; i++) {
		bool clk_low_OK = _waitForState(_port, _clk, LOW, STD_TO_TICKS, STD_TO_LOOPS);
		if (clk_low_OK) {
			uint8_t bit_val = _dat_port->IN & (1 << dat);
			byte_val |= (bit_val << i);
			bool clk_hi_OK = _waitForState(_port, _clk, HIGH, STD_TO_TICKS, STD_TO_LOOPS);
			if (!clk_hi_OK) {
				*error = (uint8_t)ERR_GETBYTE_TO_HI;
				break;
			}
		} else {
			*error = (uint8_t)ERR_GETBYTE_TO_LO;
			break;
		}
	}
	return byte_val;
}

void SB_Device::_setReceiveMode(uint8_t dat) {
	cli();						// Disable interrupts
	_port->OUTSET = _act;		// Set SB_ACT to HIGH
	_port->DIRSET = _act;		// Set SB_ACT to OUTPUT
	_port->OUTCLR = _act;		// Set SB_ACT to LOW
	_dat_port->OUTSET = dat;	// Set dat pin HIGH
	_dat_port->DIRSET = dat;	// Set dat pin to OUTPUT
	_strobeLine(_dat_port, dat);
	_dat_port->DIRCLR = dat;	// Set dat pin to INPUT
}

uint8_t SB_Device::_setSendMode(uint8_t dat) {
	uint8_t error = 0;
	bool clearToSend = _waitForState(_port, _act, LOW,
		STD_TO_TICKS, STD_TO_LOOPS);
	if (clearToSend) {
		cli();		// Disable interrupts
		_port->OUTSET = _clk;
		_port->DIRSET = _clk;
		_dat_port->OUTSET = dat;
		_dat_port->DIRSET = dat;
		_strobeLine(_dat_port, dat);
		_dat_port->DIRCLR = dat;
		bool dat_low = _waitForState(_dat_port, dat, LOW,
			STD_TO_TICKS, STD_TO_LOOPS);
		if (!dat_low) {
			error = ERR_DAT_LOW;
		}

	} else {
		error = ERR_ACT_CLEAR;
	}
	return error;
}

void SB_Device::_strobeLine(volatile PORT_t* port, uint8_t line) {
	port->OUTCLR = line;
	_delay_us(STROBE_DURATION);
	port->OUTSET = line;
}

void SB_Device::_timeoutCounterInit(void) {
	TCB0.CTRLA = 0;	// Disable timer before configuration
	TCB0.CTRLA = TCB_CLKSEL_CLKDIV2_gc;	// Select clock source
	TCB0.CTRLB = TCB_CNTMODE_INT_gc;	// periodic interrupts, ints disabled
	TCB0.INTFLAGS = TCB_CAPT_bm;		// Clear any pending interrupt flag
}

void SB_Device::_timeoutCounterStart(uint16_t timeoutValue) {
	TCB0.CNT = 0;     // Reset count
	TCB0.CCMP = timeoutValue;			// Load rollover value
	TCB0.CTRLA |= TCB_ENABLE_bm; 		// Enable the timer
}

void SB_Device::_timeoutCounterStop(void) {
	TCB0.INTFLAGS = TCB_CAPT_bm;
	TCB0.CTRLA &= ~TCB_ENABLE_bm;
}

bool SB_Device::_waitForState(volatile PORT_t* port, uint8_t pin,
	uint8_t state, uint16_t timeoutTicks, uint8_t max_loops) {
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

/**
 * Will get overloaded by child class.
 */
void SB_Device::_setDefaults(void) {
	_port->DIRCLR = _clk | _act; // set to inputs
}
