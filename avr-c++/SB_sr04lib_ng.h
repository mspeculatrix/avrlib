/* SB_sr04lib_ng.h

Library for SR04 ultrasonic rangefinder
For use with Modern ATmega and ATtiny microcontrollers (0-, 1- and 2-Series).

Uses timer TCA0 for measuring ping timing.

*/

#ifndef __SB_SR04LIB_NG__
#define __SB_SR04LIB_NG__

// Ensure we have io/sfr/pindefs loaded
#ifndef   _AVR_IO_H_
#include  <avr/io.h>
#endif

#include <stdlib.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <SB_lib_defines.h>
#include <SB_modulelib_ng.h>

#define MIN_ECHO_TIME 400
#define MAX_ECHO_TIME 23200
// #define SR04_PING_SAMPLES 3 	// Must be an odd number
#define DIST_FACTOR 100
#define SR04_CLK_PRESCALER 1

#define WAIT_UNTIL_HIGH(port, pin) do { } while (!(port->IN & (pin)))
#define WAIT_UNTIL_LOW(port, pin) do { } while (port->IN & (pin))


class SB_SR04 : public SB_Module {

public:
	// CONSTRUCTOR
	SB_SR04(PORT_t* port, uint8_t trigger, uint8_t echo,
		PORT_t* sbPort, uint8_t sbClk, uint8_t sbAct, uint8_t sbDat,
		PORT_t* datPort, volatile uint8_t* sbDatCtrl);

	// PROPERTIES
	uint16_t ping(void);
	uint16_t getLast(void) { return _last_ping; }
	uint16_t getPrevious(void) { return _previous_ping; }

protected:
	// PROPERTIES
	volatile PORT_t* _port;
	volatile uint8_t _trigger_pin;
	volatile uint8_t _echo_pin;
	volatile uint16_t _last_ping;
	volatile uint16_t _previous_ping;
};

#endif
