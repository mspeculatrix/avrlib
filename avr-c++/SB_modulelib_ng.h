/*
 *
 * Main class and library for SensorBus modules.
 *
 * Uses the timeout counter B (TCB).
 *
 * Requires SBlib_defines.h in same dir
 *
 * To use this you need to externally define some stuff, such as:
 *
 * #define SB_PORT PORTD
 * #define SB_DAT PIN0_bm // Chip Enable (Active Low: /CE)
 * #define SB_CLK PIN1_bm // Output Enable (Active Low: /OE)
 * #define SB_ACT PIN2_bm // Output Enable (Active Low: /OE)
 * #define SB_PORT_INT_VEC PORTD_PORT_vect
 * #define SB_DAT_CTRL PIN0CTRL // only for non-class stuff
 *
 * Global var:
 * bool commRequest = false;
*/

#ifndef __SB_MODULELIB_NG__
#define __SB_MODULELIB_NG__

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include <smd_std_macros.h>
#include <SB_lib_defines.h>

#define SB_MAX_TO_LOOP_COUNT 6
// Transmission delays
#define SB_START_TRANSMISSION_PAUSE 500 	// us
#define SB_BIT_PAUSE 50 					// us
#define SB_BYTE_PAUSE 50 					// us
#define SB_PULSE_LENGTH 500 	// us

#define SB_CLK_STROBE_DURATION 50 	// us

#define SB_ERR_CLK_STROBE 10
#define SB_ERR_TO_BUS 20

/*
The main program needs to provide an interrupt service routine (ISR)
suitably configured for the Port/Pin used for the Data line. Eg:

ISR(PORTD_PORT_vect) {
	if (PORTD.INTFLAGS & PIN0_bm) {		// Check if DAT triggered
		PORTD.INTFLAGS = PIN0_bm;		// Clear interrupt flag
		commRequest = true;				// Set event flag
	}
}
*/

class SensorBusModule {
public:
	SensorBusModule(volatile PORT_t* port,
		uint8_t datPin_pm, uint8_t clkPin_pm, uint8_t actPin_pm,
		volatile uint8_t* datCtrl, uint8_t msgBufLen);
	void init(void);
	uint8_t sendMessage(uint8_t* msgBuf);
	void setReceiveMode(void);
	void strobeClk(void);

protected:
	// VARIABLES
	volatile PORT_t* _port;
	volatile uint8_t _dat;
	volatile uint8_t _clk;
	volatile uint8_t _act;
	volatile uint8_t* _datCtrl;
	volatile uint8_t* _pinCtrlBase;
	uint8_t _msgBufLen;

	// METHODS
	void _setDefaults();
	void _timeoutCounterInit();
	void _timeoutCounterStart(uint16_t timeoutValue);
	void _timeoutCounterStop();
	bool _waitForState(volatile PORT_t* port, uint8_t pin, uint8_t state, uint16_t timeoutTicks, uint8_t max_loops);
};

#endif