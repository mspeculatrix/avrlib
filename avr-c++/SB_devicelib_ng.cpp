/* SB_devicelib_ng.cpp */

#include "SB_devicelib_ng.h"
#include "smd_ng_serial.h"

using namespace SensorBus;

extern SMD_NG_Serial serial;	// Intended for debugging only

/*******************************************************************************
*****  PUBLIC                                                              *****
*******************************************************************************/

// CONSTRUCTOR
SB_Device::SB_Device(volatile PORT_t* port, uint8_t clkPin_pm,
	uint8_t actPin_pm, volatile PORT_t* datport)
	: _port(port), _clk(clkPin_pm), _act(actPin_pm), _datPort(datport) {

	// base address of PIN0CTRL, from which we offset to desired pin
	_datPinCtrlBase = (volatile uint8_t*)&(_datPort->PIN0CTRL);
	_maxSendRetries = MAX_SEND_RETRIES_DFL;
	_clearBuffer(recvMsgBuf, MSG_BUF_LEN);
	_clearBuffer(sendMsgBuf, MSG_BUF_LEN);
}

const char* SB_Device::errMsg(err_code code) {
	switch (code) {
		case UNDEFINED:						return "Undefined state";
		case ERR_NONE:						return "OK";
		case ERR_SENDMODE_NO_ACK_STROBE:	return "Set send: no ack";
		case ERR_SENDMODE_ACT_NOT_CLEAR:	return "Set send: Bus busy";
		case ERR_GETBYTE_TO_LO:				return "_getByte TO low";
		case ERR_GETBYTE_TO_HI:				return "_getByte TO high";
		case ERR_UNKNOWN_DEVICE:			return "Unknown device";
		default:							return "Undefined error";
	}
}

err_code SB_Device::recvMessage(uint8_t dat) {
	_setReceiveMode(dat);			// Sends acknowledge strobe & configs pins
	err_code error = ERR_NONE;
	uint8_t bufIdx = 0;
	// The first received byte denotes the entire message length
	recvMsgBuf[0] = _getByte(dat, error);
	if (error == ERR_NONE) {
		bufIdx++;
		while (bufIdx < recvMsgBuf[0]) {
			recvMsgBuf[bufIdx] = _getByte(dat, error);
			if (error > ERR_NONE) break;
			bufIdx++;
		}
	}
	// printBuf(recvMsgBuf);
	_delay_us(SETTLE_DELAY); 	// Let bus settle, interrupts re-arm
	_setDefaults();
	return error;
}

err_code SB_Device::sendMessage(uint8_t dat) {
	uint8_t tries = 0;
	err_code error = UNDEFINED;
	while (tries < _maxSendRetries && error != ERR_NONE) {
		serial.write(".");
		error = _setSendMode(dat);
		if (error == 0) {
			uint8_t msgLen = sendMsgBuf[0];
			_delay_us(START_TRANSMISSION_PAUSE);
			// EXCHANGE LOOP
			for (uint8_t i = 0; i < msgLen; i++) { 			// loop through bytes
				// serial.writeln(sendMsgBuf[i]);
				for (uint8_t bit = 0; bit < 8; bit++) { 	// loop through bits
					if (sendMsgBuf[i] & (1 << bit)) {
						_datPort->OUTSET = dat;
					} else {
						_datPort->OUTCLR = dat;
					}
					_delay_us(BIT_PAUSE);
					_port->OUTCLR = _clk;	// Take clock low
					_delay_us(BIT_PAUSE);
					_port->OUTSET = _clk;	// Take clock high
				}
				_delay_us(BYTE_PAUSE);
			}
			// Clean up
			_datPort->DIRCLR = dat; 	// Release to input
			_delay_us(SETTLE_DELAY);	// Settle time
		}
		tries++;
		_setDefaults();
	}
	return error;
}

void SB_Device::setMaxSendRetries(uint8_t retries) {
	_maxSendRetries = retries;
}

/*******************************************************************************
*****  PROTECTED                                                           *****
*******************************************************************************/

void SB_Device::_clearBuffer(uint8_t* buf, uint8_t buf_len) {
	for (uint8_t i = 0; i < buf_len; i++) {
		buf[i] = 0;
	}
}

// Receives 8 bits of data, LSB first.
uint8_t SB_Device::_getByte(uint8_t dat, err_code& error) {
	uint8_t byte_val = 0;
	for (uint8_t i = 0; i < 8; i++) {
		bool clk_low_OK = _waitForState(_port, _clk, LOW);
		// Very slight pause to ensure we're not reading right on the edge
		_delay_us(READ_PAUSE);
		if (clk_low_OK) {
			if (_datPort->IN & dat) {
				byte_val |= (1 << i);
			}
			bool clk_hi_OK = _waitForState(_port, _clk, HIGH);
			if (!clk_hi_OK) {
				error = ERR_GETBYTE_TO_HI;
				break;
			}
		} else {
			error = ERR_GETBYTE_TO_LO;
			break;
		}
	}
	return byte_val;
}

void SB_Device::_setReceiveMode(uint8_t dat) {
	// Wait for the dat signal to be released by remote device
	_waitForState(_datPort, dat, HIGH, STD_TO_TICKS, STD_TO_LOOPS);
	_datPort->OUTSET = dat;		// Ensure dat pin HIGH when switched to OUTPUT
	_datPort->DIRSET = dat;		// Set dat pin to OUTPUT
	_delay_us(ACK_PAUSE);
	_strobeLine(_datPort, dat);
	_datPort->DIRCLR = dat;				// Set dat pin to INPUT
}

err_code SB_Device::_setSendMode(uint8_t dat) {
	err_code error = ERR_NONE;
	bool clearToSend = _waitForState(_port, _act, HIGH);
	if (clearToSend) {
		_port->OUTSET = _clk;			// Set SB_CLK to HIGH
		_port->OUTCLR = _act;			// Set SB_ACT to LOW
		_port->DIRSET = _clk | _act;	// Set SB_CLK & SB_ACT as OUTPUTS
		_datPort->OUTSET = dat;			// Set DAT to HIGH
		_datPort->DIRSET = dat;			// Set DAT to OUTPUT
		_strobeLine(_datPort, dat);		// Strobe DAT line
		_datPort->DIRCLR = dat;			// Set DAT to INPUT
		bool dat_ack = _waitForState(_datPort, dat, LOW); // Wait for ACK strobe
		if (!dat_ack) {
			error = ERR_SENDMODE_NO_ACK_STROBE;
		} else {
			_datPort->OUTSET = dat;		// Ensure DAT is HIGH
			_datPort->DIRSET = dat;		// Set DAT to OUTPUT
		}
	} else {
		error = ERR_SENDMODE_ACT_NOT_CLEAR;
	}
	return error;
}

void SB_Device::_strobeLine(volatile PORT_t* port, uint8_t line) {
	port->OUTCLR = line;				// Take line LOW
	_delay_us(STROBE_DURATION);			// Hold for a moment
	port->OUTSET = line;				// Take line high
}

void SB_Device::_timeoutCounterInit(void) {
	TCB0.CTRLA = 0;						// Disable timer before configuration
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

// Wrapper that calls the main function with the default values for
// timeoutTicks and maxLoops
bool SB_Device::_waitForState(volatile PORT_t* port, uint8_t pin, uint8_t state) {
	return _waitForState(port, pin, state, STD_TO_TICKS, STD_TO_LOOPS);
}

bool SB_Device::_waitForState(volatile PORT_t* port, uint8_t pin,
	uint8_t state, uint16_t timeoutTicks, uint8_t maxLoops) {
	bool stateAchieved = false;
	bool maxLoopLimit = false;
	uint8_t loopCount = 0;
	_timeoutCounterStart(timeoutTicks);
	while (!stateAchieved && !maxLoopLimit) {
		bool timedOut = false;
		while (!stateAchieved && !timedOut) {
			uint8_t pinState = port->IN & pin;
			if ((state == HIGH && pinState) || (state == LOW && !pinState)) {
				stateAchieved = true;
			} else if (TCB0.INTFLAGS & TCB_CAPT_bm) {	// overflow happened
				TCB0.INTFLAGS = TCB_CAPT_bm;			// clear flag
				timedOut = true;
			}
		}
		if (!stateAchieved) {
			if (loopCount >= maxLoops) {
				maxLoopLimit = true;
			} else {
				loopCount++;
			}
		}
	}
	_timeoutCounterStop();
	return stateAchieved;
}

/**
 * Will get overridden by child classes, probably. We'll do the minimum
 * necessary here as a placeholder.
 */
void SB_Device::_setDefaults(void) {
	_port->OUTSET = _clk | _act;	// Pull high by default
	_port->DIRCLR = _clk | _act; 	// Set to inputs
}


/* ***** FOR DEBUGGING ***** */
// Intended for debugging only
void SB_Device::printBuf(uint8_t* buf) {
	for (uint8_t i = 0; i < MSG_BUF_LEN; i++) {
		serial.write(buf[i]);
		serial.write(" ");
	}
	serial.writeln(" ");
}

void SB_Device::printMsg(uint8_t* buf) {
	for (uint8_t i = 0; i < buf[0]; i++) {
		serial.write(buf[i]);
		serial.write(" ");
	}
	serial.writeln(" ");
}
