#include "SB_sr04lib_ng.h"


SB_SR04::SB_SR04(PORT_t* port, uint8_t trigger, uint8_t echo)
	: _port(port), _trigger_pin(trigger), _echo_pin(echo) {
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
	_delta = distance - _last_ping;
	_last_ping = distance;
	//	}
	sei(); 			// re-enable interrupts
	return distance;
}




// For sorting ping values
// int compare_desc(const void* a, const void* b) {
// 	return (*(unsigned long*)b - *(unsigned long*)a);
// }
