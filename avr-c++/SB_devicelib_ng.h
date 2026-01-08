/*
Common library for both Modules and Nodes.
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
	const uint16_t START_TRANSMISSION_PAUSE = 500; 	// us
	const uint8_t BIT_PAUSE = 50; 					// us
	const uint8_t BYTE_PAUSE = 50; 					// us
	const uint16_t PULSE_LENGTH = 500; 	// us

	const uint8_t STROBE_DURATION = 50; 	// us

	const uint16_t STD_TO_TICKS = 0xFFFF;
	const uint8_t STD_TO_LOOPS = 6;

	const uint8_t ERR_CLK_STROBE = 10;
	const uint8_t ERR_TO_BUS = 20;
	const uint8_t ERR_DAT_LOW = 25;
	const uint8_t ERR_ACT_CLEAR = 30;
	const uint8_t ERR_GETBYTE_TO_LO = 40;
	const uint8_t ERR_GETBYTE_TO_HI = 41;

	class SB_Device
	{
	public:
		// CONSTRUCTOR
		SB_Device(volatile PORT_t* port, uint8_t clkPin_pm, uint8_t actPin_pm,
			volatile PORT_t* datport);
		uint8_t recvMessage(uint8_t* msgbuf, uint8_t dat);
		uint8_t sendMessage(uint8_t* msgbuf, uint8_t dat);

	protected:
		volatile PORT_t* _port;
		volatile uint8_t _clk;
		volatile uint8_t _act;
		volatile PORT_t* _dat_port;
		volatile uint8_t* _datPinCtrlBase;

		uint8_t _getByte(uint8_t dat, uint8_t* error);
		void _setDefaults(void);
		void _setReceiveMode(uint8_t dat);
		uint8_t _setSendMode(uint8_t dat);
		void _strobeLine(volatile PORT_t* port, uint8_t line);
		void _timeoutCounterInit();
		void _timeoutCounterStart(uint16_t timeoutValue);
		void _timeoutCounterStop();
		bool _waitForState(volatile PORT_t* port, uint8_t pin, uint8_t state,
			uint16_t timeoutTicks,
			uint8_t max_loops);
	};

} // namespace SB

#endif
