/* SB_sr04lib_ng.h

Library for SR04 ultrasonic rangefinder
For use with Modern ATmega and ATtiny microcontrollers (0-, 1- and 2-Series).

Uses timer TCA0 for measuring ping timing.

Requires following ISR:

ISR(PORTA_PORT_vect) {
	sr04.getIntFlags();
}

For PIT timer, requires an ISR along the lines of:

// PIT timer ISR called ~every 0.5 second to initiate ping
ISR(RTC_PIT_vect) {
	pitTickCounter++;
	RTC.PITINTFLAGS = RTC_PI_bm; 		// Clears the interrupt register flag
	if (pitTickCounter == PIT_TICK_COUNT_THRESHOLD) {
		performPing = true;				// Set event flag
		pitTickCounter = 0;
	}
}
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
#include <smd_ng_serial.h>


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
	SB_SR04(void);

	SMD_NG_Serial serial = SMD_NG_Serial(57600, &PORTB, PIN2_bm, PIN3_bm);
	static constexpr uint8_t ALERT_LED = PIN0_bm;
	static constexpr uint8_t ACT_LED = PIN1_bm;

	// METHODS
	void begin(void);
	uint16_t ping(void);
	uint16_t getLast(void) { return _last_ping; }
	uint16_t getPrevious(void) { return _previous_ping; }
	void flashLED(uint8_t led);
	void pulseLED(uint8_t led);
	void pulseLED(uint8_t led, uint8_t numPulses);
	void setLED(uint8_t led, uint8_t onOff);
	SensorBus::err_code recvMessage(void); // overwrites parent func

	void PIT_init(void);
	void PIT_enable(void);
	void PIT_disable(void);
	void PIT_restart(void);

protected:
	// PROPERTIES
	static inline PORT_t* const _port = &PORTA;
	static constexpr uint8_t _trigger_pin = PIN4_bm;
	static constexpr uint8_t _echo_pin = PIN5_bm;
	volatile uint16_t _last_ping;
	volatile uint16_t _previous_ping;

	static inline PORT_t* const _SB_PORT = &PORTA;
	static constexpr uint8_t _SB_CLK = PIN3_bm;
	static constexpr uint8_t _SB_ACT = PIN1_bm;
	static inline PORT_t* const _SB_DATPORT = &PORTA;
	static constexpr uint8_t _SB_DAT = PIN2_bm;
	static inline volatile uint8_t* const _SB_DATCTRL = &PORTA.PIN2CTRL;

	static inline PORT_t* const _LED_PORT = &PORTB;

	static constexpr uint8_t _DEFAULT_LED_FLASH_DELAY = 150;
	static constexpr uint8_t _DEFAULT_LED_PULSE_DELAY = 150;
	static constexpr uint8_t _DEFAULT_LED_PULSE_CYCLES = 3;

};

#endif
