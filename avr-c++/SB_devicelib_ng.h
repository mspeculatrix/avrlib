/* SB_devicelib_ng.h

Common SensorBus device library for Modules and Nodes.

Devices need to use interrupts to respond to incoming communication requests.

The main program needs to define certain ports and pins eg:

// SensorBus ports, pins, ISR vector
#define SB_PORT 		PORTA
#define SB_ACT  		PIN1_bm
#define SB_CLK  		PIN3_bm
#define SB_DATPORT 		PORTA
#define SB_DAT 			PIN2_bm		// for modules only
#define SB_DAT_CTRL		PIN2CTRL	// for modules only
#define SB_DAT_ISR_VEC 	PORTD_PORT_vect

And it should contain this ISR function (replace the `node.` in this
example with the object name):

ISR(DAT_ISR_VECTOR) {
	node.commRequestRcvd = SB_DATPORT.INTFLAGS;	// set to bits triggering interrupt(s)
	SB_DATPORT.INTFLAGS = node.commRequestRcvd;	// clear flags
}
*/

#ifndef __SB_DEVICELIB_NG__
#define __SB_DEVICELIB_NG__

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include <smd_std_macros.h>
#include <SB_lib_defines.h>

namespace SensorBus {

	const uint8_t MSG_BUF_LEN = 16;

	// Transmission delays
	// ACK_PAUSE is used when a receiving device gets an initial strobe from
	// a remote device. It's how long the receiver should wait before sending
	// back an acknowledgement strobe. As both strobes use the same signal
	// (DAT) it's important the strobes don't overlap.
	const uint16_t START_TRANSMISSION_PAUSE = 5; 	// us
	const uint8_t STROBE_DURATION = 10; 			// us
	const uint8_t READ_PAUSE = STROBE_DURATION / 4;	// us
	const uint8_t ACK_PAUSE = STROBE_DURATION + 5;	// us
	const uint8_t BIT_PAUSE = 50; 					// us
	const uint8_t BYTE_PAUSE = 50; 					// us
	const uint8_t SETTLE_DELAY = 5;					// us

	// For timeouts - standard values
	const uint16_t STD_TO_TICKS = 0xFFFF;	// Max number of ticks in each loop
	const uint8_t STD_TO_LOOPS = 10;			// Max number of loops

	typedef enum errcodes {
		ERR_NONE = 0,
		ERR_SENDMODE_ACT_NOT_CLEAR = 10,
		ERR_SENDMODE_NO_ACK_STROBE,
		ERR_GETBYTE_TO_LO = 40,
		ERR_GETBYTE_TO_HI
	} err_code;

	class SB_Device
	{
	public:
		// CONSTRUCTOR
		SB_Device(volatile PORT_t* port, uint8_t clkPin_pm, uint8_t actPin_pm,
			volatile PORT_t* datport);

		const char* errMsg(err_code code);
		volatile int8_t commRequestRcvd = -1;
		uint8_t recvMsgBuf[MSG_BUF_LEN];
		uint8_t sendMsgBuf[MSG_BUF_LEN];

		void printBuf(uint8_t* buf);
		void printMsg(uint8_t* buf);

		err_code recvMessage(uint8_t dat);
		err_code sendMessage(uint8_t dat);

	protected:
		volatile PORT_t* _port;
		volatile uint8_t _clk;
		volatile uint8_t _act;
		volatile PORT_t* _datPort;
		volatile uint8_t* _datPinCtrlBase;

		void _clearBuffer(uint8_t* buf, uint8_t buf_len);
		uint8_t _getByte(uint8_t dat, err_code& error);
		void _setDefaults(void);
		void _setReceiveMode(uint8_t dat);
		err_code _setSendMode(uint8_t dat);
		void _strobeLine(volatile PORT_t* port, uint8_t line);
		void _timeoutCounterInit();
		void _timeoutCounterStart(uint16_t timeoutValue);
		void _timeoutCounterStop();
		bool _waitForState(volatile PORT_t* port, uint8_t pin, uint8_t state);
		bool _waitForState(volatile PORT_t* port, uint8_t pin, uint8_t state,
			uint16_t timeoutTicks, uint8_t maxLoops);
	};

} // namespace SerialBus

#endif
