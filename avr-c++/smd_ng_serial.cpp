#include "smd_ng_serial.h"

// Initialize the static array to nulls
SMD_NG_Serial* SMD_NG_Serial::instances[3] = { nullptr, nullptr, nullptr };

// The generic interrupt handler
void SMD_NG_Serial::handle_interrupt(uint8_t index) {
	if (instances[index] != nullptr) {
		SMD_NG_Serial* obj = instances[index];

		// Read data from the hardware that triggered the interrupt
		uint8_t data = obj->_hw->RXDATAL;

		// Calculate next write position
		uint8_t next_idx = (obj->_recvbuf_write_idx + 1) % SER_RECV_BUF_SZ;

		// If buffer isn't full, store the byte
		if (next_idx != obj->_recvbuf_read_idx) {
			obj->_recvbuf[obj->_recvbuf_write_idx] = data;
			obj->_recvbuf_write_idx = next_idx;
		}
	}
}

// ISR vector redirects
ISR(USART0_RXC_vect) { SMD_NG_Serial::handle_interrupt(0); }
ISR(USART1_RXC_vect) { SMD_NG_Serial::handle_interrupt(1); }
ISR(USART2_RXC_vect) { SMD_NG_Serial::handle_interrupt(2); }



// -------------------------------------------------------------------------
// -----  CONSTRUCTORS                                                 -----
// -------------------------------------------------------------------------
SMD_NG_Serial::SMD_NG_Serial(void) {
	_init(19200, SER_DATA_BITS8, SER_STOP_BITS1, SER_PARITY_NONE, &PORTA, PIN0_bm, PIN1_bm);
}

// Instantiate with default baudrate, 8 data bits, 1 stop bit
SMD_NG_Serial::SMD_NG_Serial(volatile PORT_t* port,
	uint8_t tx_pin_bm, uint8_t rx_pin_bm) {
	_init(19200, SER_DATA_BITS8, SER_STOP_BITS1, SER_PARITY_NONE, port, tx_pin_bm, rx_pin_bm);
}

// Instantiate with definable baudrate, 8 data bits, 1 stop bit
SMD_NG_Serial::SMD_NG_Serial(uint32_t baudrate, volatile PORT_t* port, uint8_t tx_pin_bm, uint8_t rx_pin_bm) {
	_init(baudrate, SER_DATA_BITS8, SER_STOP_BITS1, SER_PARITY_NONE, port, tx_pin_bm, rx_pin_bm);
}

SMD_NG_Serial::SMD_NG_Serial(uint32_t baudrate, uint8_t dataBits, uint8_t stopBits,
	volatile PORT_t* port, uint8_t tx_pin_bm, uint8_t rx_pin_bm) {
	_init(baudrate, dataBits, stopBits, SER_PARITY_NONE, port, tx_pin_bm, rx_pin_bm);
}

void SMD_NG_Serial::_init(uint32_t baudrate, uint8_t dataBits, uint8_t stopBits, uint8_t parity,
	volatile PORT_t* port, uint8_t tx_pin_bm, uint8_t rx_pin_bm) {
	_baud = baudrate;
	_dataBits = dataBits;
	_stopBits = stopBits;
	_parity = parity;
	_port = port;
	_tx_pin_bm = tx_pin_bm;
	_rx_pin_bm = rx_pin_bm;
	_started = false;
	_useCR = false;
	_sendNullTerminator = false;
	_recvbuf_read_idx = 0;
	_recvbuf_write_idx = 0;
	_hw = nullptr;
}

// -------------------------------------------------------------------------
// -----  METHODS                                                      -----
// -------------------------------------------------------------------------

uint8_t SMD_NG_Serial::begin(void) {
	return begin(&USART0); // Default to USART0 for backward compatibility
}

uint8_t SMD_NG_Serial::begin(USART_t* usart_hw) {
	_hw = usart_hw;

	// Register this instance for the ISRs
	if (_hw == &USART0) instances[0] = this;
	else if (_hw == &USART1) instances[1] = this;
	else if (_hw == &USART2) instances[2] = this;

	cli();

	// Calculate Baud Rate
	uint16_t baud_setting = (64 * F_CPU + ((16UL * _baud) / 2)) / (16UL * _baud);
	_hw->BAUD = baud_setting;

	// Frame Format
	uint8_t ctrlc = 0;
	ctrlc |= (_parity << 4);
	ctrlc |= (_stopBits << 3);
	ctrlc |= (_dataBits);
	_hw->CTRLC = ctrlc;

	// Pin Configuration
	_port->DIRSET = _tx_pin_bm;
	_port->DIRCLR = _rx_pin_bm;

	// Enable Hardware
	_hw->CTRLB = USART_RXEN_bm | USART_TXEN_bm;
	_hw->CTRLA = USART_RXCIE_bm; // Enable Receive Complete Interrupt

	_started = true;
	clearInputBuffer();
	sei();

	return 0;
}

bool SMD_NG_Serial::started(void) {
	return _started;
}

void SMD_NG_Serial::clearInputBuffer(void) {
	_recvbuf_read_idx = 0;
	_recvbuf_write_idx = 0;
}

// -------------------------------------------------------------------------
// -----  RECEIVING                                                    -----
// -------------------------------------------------------------------------

bool SMD_NG_Serial::inWaiting(void) {
	return _recvbuf_write_idx != _recvbuf_read_idx;
}

uint8_t SMD_NG_Serial::getByte(void) {
	uint8_t byteVal = _recvbuf[_recvbuf_read_idx];
	_recvbuf_read_idx = (_recvbuf_read_idx + 1) % SER_RECV_BUF_SZ;
	return byteVal;
}

bool SMD_NG_Serial::readByte(uint8_t* byteVal) {
	if (inWaiting()) {
		*byteVal = getByte();
		return true;
	}
	return false;
}

uint8_t SMD_NG_Serial::readBytes(uint8_t* buf, uint8_t numToRead) {
	uint8_t counter = 0;
	uint8_t inByte = 0;
	while (readByte(&inByte) && counter < numToRead) {
		buf[counter] = inByte;
		counter++;
	}
	return counter;
}

uint8_t SMD_NG_Serial::readLine(char* buffer, size_t bufferSize, bool preserveNewline) {
	uint8_t error = 0;
	if (bufferSize < 2) return SER_ERR_READLINE_BUFFER_TOO_SMALL;

	if (bufferSize > SER_READLINE_BUFFER_MAX) bufferSize = SER_READLINE_BUFFER_MAX;

	bool ended = false;
	size_t index = 0;
	uint8_t inByte = 0;

	do {
		if (readByte(&inByte)) {
			if (inByte == 0) {
				buffer[index] = 0;
				ended = true;
			} else if (inByte == SER_NL) {
				if (preserveNewline && index < bufferSize - 1) {
					buffer[index++] = SER_NL;
				}
				buffer[index] = 0;
				ended = true;
			} else if (inByte == SER_CR) {
				// Skip CR
			} else if (index >= bufferSize - 1) {
				buffer[index] = 0;
				ended = true;
			} else {
				buffer[index++] = inByte;
			}
		}
	} while (!ended);

	return error;
}

// -------------------------------------------------------------------------
// -----  TRANSMITTING                                                 -----
// -------------------------------------------------------------------------

bool SMD_NG_Serial::sendByte(uint8_t byteVal) {
	// Wait until data register is empty on the assigned hardware
	while (!(_hw->STATUS & USART_DREIF_bm)) {};
	_hw->TXDATAL = byteVal;

	// Note: DEF_SEND_CHAR_DELAY might not be needed with DREIF check,
	// but kept for consistency with your original code.
	_delay_ms(DEF_SEND_CHAR_DELAY);
	return false;
}

uint8_t SMD_NG_Serial::write(const char* string) { return _writeStr(string, false); }
uint8_t SMD_NG_Serial::write(const double fnum) { return _writeDouble(fnum, false); }
uint8_t SMD_NG_Serial::write(const int twoByteInt) { return _writeInt16(twoByteInt, false); }
uint8_t SMD_NG_Serial::write(const long longInt) { return _writeLongInt(longInt, false); }

uint8_t SMD_NG_Serial::writeChar(const char ch) {
	sendByte((uint8_t)ch);
	return 0;
}

uint8_t SMD_NG_Serial::writeln(const char* string) { return _writeStr(string, true); }
uint8_t SMD_NG_Serial::writeln(const int twoByteInt) { return _writeInt16(twoByteInt, true); }
uint8_t SMD_NG_Serial::writeln(const long longInt) { return _writeLongInt(longInt, true); }
uint8_t SMD_NG_Serial::writeln(const double fnum) { return _writeDouble(fnum, true); }

uint8_t SMD_NG_Serial::_writeDouble(const double fnum, bool addReturn) {
	char numStr[30];
	dtostrf(fnum, 3, 5, numStr);
	return _writeStr(numStr, addReturn);
}

uint8_t SMD_NG_Serial::_writeInt16(const int twoByteInt, bool addReturn) {
	char numStr[20];
	itoa(twoByteInt, numStr, 10);
	return _writeStr(numStr, addReturn);
}

uint8_t SMD_NG_Serial::_writeLongInt(const long longInt, bool addReturn) {
	char numStr[30];
	ltoa(longInt, numStr, 10);
	return _writeStr(numStr, addReturn);
}

uint8_t SMD_NG_Serial::_writeStr(const char* string, bool addReturn) {
	if (string[0] == 0) return SER_RES_EMPTY_STRING;

	for (size_t i = 0; string[i] != 0; i++) {
		sendByte(string[i]);
	}

	if (addReturn) {
		if (_useCR) sendByte(SER_CR);
		sendByte(SER_NL);
	}
	if (_sendNullTerminator) sendByte(SER_NUL);

	return 0;
}
