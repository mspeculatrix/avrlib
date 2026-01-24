#ifndef __SB_SERVOLIB_NG__
#define __SB_SERVOLIB_NG__

/* *** WORK IN PROGRESS *** */

// Ensure we have io/sfr/pindefs loaded
#ifndef   _AVR_IO_H_
#include  <avr/io.h>
#endif

// #include <stdlib.h>
// #include <avr/interrupt.h>
// #include <util/delay.h>
#include <SB_lib_defines.h>
#include <SB_modulelib_ng.h>

class SB_Servo : public SensorBusModule {

public:
	// CONSTRUCTOR
	SB_Servo(PORT_t* sbPort, uint8_t sbClk, uint8_t sbAct, uint8_t sbDat,
		PORT_t* datPort, volatile uint8_t* sbDatCtrl);

	void begin(void);
	void write(uint8_t angle);

protected:
	// PROPERTIES
	volatile PORT_t* _servoPort;
	volatile uint8_t _servoPin;

	// Calculations for 20MHz with 1024 Prescaler:
	// Period = 20,000,000 / 1024 / 50Hz = 390
	const uint16_t PERIOD_20MS = 390;

	// Pulse widths (Period * time_in_ms / 20ms)
	const uint16_t PULSE_MIN = 20;  // ~1.0ms (0 degrees)
	const uint16_t PULSE_MAX = 40;  // ~2.0ms (180 degrees)

};


#endif
