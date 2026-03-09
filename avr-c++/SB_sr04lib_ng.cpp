/* SB_sr04lib_ng.cpp */

#include "SB_sr04lib_ng.h"

SB_SR04::SB_SR04(void)
	: SB_Module(_SB_PORT, _SB_CLK, _SB_ACT, _SB_DAT, _SB_DATPORT, _SB_DATCTRL) {
	// Configure TCA0 for normal (count‑up) mode, no PWM
	// The following just sets the default, but for the sake of completeness...
	TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_NORMAL_gc;
	// Optional: ensure no compare channels are enabled
	TCA0.SINGLE.CTRLD = 0;
	// Clear the counter and period registers
	TCA0.SINGLE.CNT = 0;      // reset counter
	TCA0.SINGLE.PER = 0xFFFF; // roll over at max 16‑bit value
	// Enable & clock div
	TCA0.SINGLE.CTRLA = TCA_SINGLE_ENABLE_bm | TCA_SINGLE_CLKSEL_DIV1_gc;
	_last_ping = 0;
	_previous_ping = 0;
}

void SB_SR04::begin(void) {
	serial.begin();
	serial.writeln("Servo module initialised.");

	_port->OUTCLR = _trigger_pin; 			// Default to low
	_port->DIRSET = _trigger_pin;			// Set as output
	_port->DIRCLR = _echo_pin;				// Set as input

	_LED_PORT->DIRSET = ALERT_LED | ACT_LED;
	_LED_PORT->OUTCLR = ALERT_LED | ACT_LED;
}

// Flash an LED once.
void SB_SR04::flashLED(uint8_t led) {
	_LED_PORT->OUTSET = led;
	_delay_ms(_DEFAULT_LED_FLASH_DELAY);
	_LED_PORT->OUTCLR = led;
}

// Flash a specified LED a default number of times.
// Wrapper to next function.
void SB_SR04::pulseLED(uint8_t led) {
	pulseLED(led, _DEFAULT_LED_PULSE_CYCLES);
}

// Flash a specified LED a given number of times.
void SB_SR04::pulseLED(uint8_t led, uint8_t numPulses) {
	for (uint8_t i = 0; i < numPulses; i++) {
		_LED_PORT->OUTSET = led;
		_delay_ms(_DEFAULT_LED_PULSE_DELAY);
		_LED_PORT->OUTCLR = led;
		_delay_ms(_DEFAULT_LED_PULSE_DELAY);
	}
}

void SB_SR04::setLED(uint8_t led, uint8_t onOff) {
	if (onOff == ON) {
		_LED_PORT->OUTSET = led;
	} else {
		_LED_PORT->OUTCLR = led;
	}
}

/**
 * @brief Get SensorBus message.
 * Overwrites parent method.
 */
SensorBus::err_code SB_SR04::recvMessage(void) {
	return SB_Module::recvMessage(_SB_DAT);
}

uint16_t SB_SR04::ping(void) {
	cli();					// disable interrupts for now
	uint16_t distance = 0;
	// unsigned long samples[SR04_PING_SAMPLES];

//	for (uint8_t i = 0; i < SR04_PING_SAMPLES; i++) {
	_port->OUTSET = _trigger_pin;		// set trigger pin high
	_delay_us(10);						// wait for min. 10us
	_port->OUTCLR = _trigger_pin;		// take trigger pin low again
	WAIT_UNTIL_HIGH(_port, _echo_pin);
	TCA0.SINGLE.CNT = 0;      			// reset counter
	WAIT_UNTIL_LOW(_port, _echo_pin);
	unsigned long elapsed = TCA0.SINGLE.CNT;
	// samples[i] = TCA0.SINGLE.CNT;
//	}

	// Sort the samples into descending order
	// qsort(samples, SR04_PING_SAMPLES, sizeof(unsigned long), compare_desc);

	// Take the median value as a crude way of eliminating outliers
	// unsigned long elapsed = samples[SR04_PING_SAMPLES / 2];

	// elapsed = (elapsed * SR04_CLK_PRESCALER) / (F_CPU / 1000000.0);	// to scale for clock speed & prescaler
//	if (MIN_ECHO_TIME <= elapsed && elapsed <= MAX_ECHO_TIME) {
	distance = (uint16_t)(elapsed / DIST_FACTOR);
	_previous_ping = _last_ping;
	_last_ping = distance;
	//	}
	sei(); 			// re-enable interrupts
	return distance;
}

// For sorting ping values
// int compare_desc(const void* a, const void* b) {
// 	return (*(unsigned long*)b - *(unsigned long*)a);
// }

/*            *****  PIT TIMER FUNCTIONS  *****

The Periodic Interrupt Timer (PIT) is part of the RTC block.
These functions assume the default 32.768 kHz ULP oscillator for the RTC.

*/

// Initialise the Periodic Interrupt Timer (PIT). This needs to be called
// once during the setup part of main(), before calling sei().
void SB_SR04::PIT_init(void) {
	while (RTC.STATUS > 0);	// Ensure all RTC registers are synchronised
	RTC.CLKSEL = RTC_CLKSEL_INT32K_gc;	// Select internal 32.768kHz oscillator
	while (RTC.STATUS > 0);				// Wait for synchronisation
	// Enable RTC with prescaler (required for PIT to work)
	RTC.CTRLA = RTC_PRESCALER_DIV1_gc | RTC_RTCEN_bm;
	while (RTC.STATUS > 0);				// Wait for synchronisation
	while (RTC.PITSTATUS > 0);			// Wait for PIT registers to be ready
	// Enable PIT interrupt BEFORE enabling PIT
	RTC.PITINTCTRL = RTC_PI_bm;

	// Configure PIT period AND enable it. The following values are options to
	// determine the approximate intervals at which the interrupt fires
	// (other, smaller, values may be available):
	//     RTC_PERIOD_CYC4096_gc 	- 125 ms
	//     RTC_PERIOD_CYC8192_gc	- 250 ms
	//     RTC_PERIOD_CYC16384_gc	- 500 ms
	//     RTC_PERIOD_CYC32768_gc 	- 1 sec
	// The ATtiny1604 can't go beyond this but some microcontrollers, such as
	// the ATmega4809, can use:
	//     RTC_PERIOD_CYC65536_gc	- 2 secs
	RTC.PITCTRLA = RTC_PERIOD_CYC16384_gc | RTC_PITEN_bm; // 0.5 sec interval
}

// Enable the Periodic Interrupt Timer (PIT). Call this AFTER sei().
void SB_SR04::PIT_enable(void) {
	while (RTC.PITSTATUS > 0); 					// Wait for synchronisation
	RTC.PITCTRLA |= RTC_PITEN_bm;
}

// Disable the Periodic Interrupt Timer (PIT)
void SB_SR04::PIT_disable(void) {
	RTC.PITINTCTRL &= ~RTC_PI_bm;				// Disable the interrupt first
	while (RTC.PITSTATUS & RTC_CTRLBUSY_bm);	// Wait for PIT synchronisation
	RTC.PITCTRLA &= ~RTC_PITEN_bm;	    		// Turn off the PIT
}

// Restart PIT after it has been disabled
void SB_SR04::PIT_restart(void) {
	while (RTC.STATUS > 0 || RTC.PITSTATUS > 0);	// Wait for synchronisation
	// Reset the RTC Prescaler by toggling RTCEN
	// This forces the internal prescaler (and thus the PIT) back to 0
	uint8_t temp = RTC.CTRLA;
	RTC.CTRLA = temp & ~RTC_RTCEN_bm;
	while (RTC.STATUS > 0); // Wait for sync
	RTC.CTRLA = temp | RTC_RTCEN_bm;
	while (RTC.STATUS > 0); // Wait for sync

	RTC.PITINTCTRL = RTC_PI_bm;				// Re-enable PIT and its interrupt
	while (RTC.PITSTATUS > 0);				// Wait until ready
	RTC.PITCTRLA |= RTC_PITEN_bm;
}
