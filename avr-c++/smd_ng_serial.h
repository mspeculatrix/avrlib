/**
 * @file smd_ng_serial.h
 *
 * C++ Serial Library for modern AVR microcontrollers
 * (Series-0, Series-1, Series-2)
 *
 * *** IMPORTANT - F_CPU must be set. ***
 *
 * Each instance has its own receive buffer.
 *
 * Typical use:
 * SMD_NG_Serial serial = SMD_NG_Serial(19200, &PORTA, PIN0_bm, PIN1_bm);
 * uint8_t error = serial.begin();
 *
 * TO DO:
 * - Need to do more with receive buffer.
 * - Not quite sure if all the read functions use the buffer or read directly.
*/

#ifndef __SMD_NG_SERIAL_H__
#define __SMD_NG_SERIAL_H__
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/cpufunc.h> // Required for ccp_write_io
#include <util/delay.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define SER_RECV_BUF_SZ 64

#define SER_NUL 0x00  // 0  - NULL - often used for string termination
// reserving 1-3 for special uses
#define SER_EOT 0x04  // 4  - end of transmission
#define SER_ENQ 0x05  // 5  - enquiry - might come in handy one day
#define SER_BEL 0x07  // 7  - bell
#define SER_NL  0x0A  // 10 - newline/line feed
#define SER_CR  0x0D  // 13 - carriage return
#define SER_ESC 0x1B  // 27 - escape
#define SER_SPC 0x20  // 32 - space
#define NO_DATA 0x80  // 128 use this as a flag to indicate no data waiting in
					  // byte buffer. It's the high-order version of null

#define SER_READLINE_BUFFER_MAX 255

#define DEF_SEND_CHAR_DELAY 5

#define SER_PARITY_NONE 0b00
#define SER_PARITY_EVEN 0b10
#define SER_PARITY_ODD  0b11

#define SER_STOP_BITS1  0
#define SER_STOP_BITS2  1

#define SER_DATA_BITS7 0b010
#define SER_DATA_BITS8 0b011


// ERROR & RESULT CODES
//#define SER_RES_INT_TOO_LARGE 1
#define SER_ERR_INCORRECT_DATA_BITS 1
#define SER_ERR_INCORRECT_STOP_BITS 2
#define SER_ERR_BAUD_TOO_LOW 3
#define SER_ERR_BAUD_TOO_HIGH 4
#define SER_ERR_UBRR_OVERFLOW 5
#define SER_RES_EMPTY_STRING 6
#define SER_ERR_READLINE_BUFFER_TOO_SMALL 7

namespace smd_ng_serial {}

class SMD_NG_Serial {
public:
	// Constructors
	SMD_NG_Serial(void);
	SMD_NG_Serial(volatile PORT_t* port,
		uint8_t tx_pin_bm, uint8_t rx_pin_bm);
	SMD_NG_Serial(uint32_t baudrate, volatile PORT_t* port,
		uint8_t tx_pin_bm, uint8_t rx_pin_bm);
	SMD_NG_Serial(uint32_t baudrate, uint8_t dataBits,
		uint8_t stopBits, volatile PORT_t* port,
		uint8_t tx_pin_bm, uint8_t rx_pin_bm);

	// Methods
	uint8_t begin(void);	// init with specific USART
	uint8_t begin(USART_t* usart_hw);	// init with specific USART
	bool started(void);
	void clearInputBuffer(void);
	void useNullTerminator(bool sendNull) { _sendNullTerminator = sendNull; }
	void addCarriageReturn(bool addCR) { _useCR = addCR; }

	// Static method called by the ISRs
	static void handle_interrupt(uint8_t index);

	// Receiving
	uint8_t getByte(void);            // read a byte
	bool inWaiting(void);             // is there a byte waiting in the buffer?
	bool readByte(uint8_t* byteVal);  // read a byte from buffer
	uint8_t readBytes(char* buf, uint8_t numToRead);
	uint8_t readBytes(uint8_t* buf, uint8_t numToRead);
	uint8_t readLine(char* buffer, size_t bufferSize, bool preserveNewline);

	// Transmitting
	void sendByte(uint8_t byteVal); 		// send single byte

	uint8_t write(const char* string);
	uint8_t write(const int twoByteInt);	// max value 32767
	uint8_t write(const uint16_t word);
	uint8_t write(const long longInt);
	uint8_t write(const double fnum);

	uint8_t writeChar(const char ch);

	uint8_t writeln(const char* string);
	uint8_t writeln(const int twoByteInt);
	uint8_t writeln(const uint16_t word);
	uint8_t writeln(const long longInt);
	uint8_t writeln(const double fnum);

protected:
	USART_t* _hw;				// Pointer to the specific USART

	uint8_t _instance_idx; // 0 for USART0, 1 for USART1, etc.

	// Receive buffers
	volatile uint8_t _recvbuf[SER_RECV_BUF_SZ];
	volatile uint8_t _recvbuf_write_idx;
	volatile uint8_t _recvbuf_read_idx;

	// Static array to keep track of created instances
	static SMD_NG_Serial* instances[3];

	uint16_t _baud;
	uint8_t _dataBits;
	bool _echo;
	uint8_t _stopBits;
	uint8_t _parity;
	volatile PORT_t* _port;
	uint8_t _rx_pin_bm;
	uint8_t _tx_pin_bm;
	bool _started;
	bool _sendNullTerminator;	// add null terminator 0 to end of all sends?
	bool _useCR;

	void _init(uint32_t baudrate, uint8_t dataBits, uint8_t stopBits,
		uint8_t parity, volatile PORT_t* port,
		uint8_t tx_pin_bm, uint8_t rx_pin_bm);

	uint8_t _writeInt16(const int twoByteInt, bool addReturn);	// max 32767
	uint8_t _writeLongInt(const long longInt, bool addReturn);
	uint8_t _writeDouble(const double fnum, bool addReturn);
	uint8_t _writeStr(const char* string, bool addReturn);
};

#endif
