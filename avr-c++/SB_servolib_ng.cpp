/* SB_servolib_ng.cpp */

#include "SB_servolib_ng.h"

// CONSTRUCTOR
SB_Servo::SB_Servo(PORT_t* sbPort, uint8_t sbClk, uint8_t sbAct, uint8_t sbDat,
	PORT_t* datPort, volatile uint8_t* sbDatCtrl)
	: SensorBusModule(sbPort, sbClk, sbAct, sbDat, datPort, sbDatCtrl) {

	_servoPort = &PORTA;
	_servoPin = PIN3_bm;
}

void SB_Servo::begin(void) {
	// 1. Set PA3 as output (TCA0 WO3)
	_servoPort->DIRSET = _servoPin;

	// 2. Set TCA0 to Normal (Single) Mode
	TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_SINGLESLOPE_gc | TCA_SINGLE_CMP0EN_bm;

	// 3. Set the Period (16-bit register)
	TCA0.SINGLE.PER = PERIOD_20MS;

	// 4. Set Clock Prescaler to 1024 and Enable
	TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1024_gc | TCA_SINGLE_ENABLE_bm;
}

/**
 * @param angle 0 to 180
 */
void SB_Servo::write(uint8_t angle) {
	if (angle < 0) angle = 0;
	if (angle > 180) angle = 180;

	// Linear mapping from angle to pulse width
	// Using 32-bit math for the intermediate step to prevent overflow
	uint16_t duty = PULSE_MIN + ((uint32_t)(PULSE_MAX - PULSE_MIN) * angle) / 180;

	// Update Compare Register 0 (Maps to WO0 / PA3 in Single Mode)
	TCA0.SINGLE.CMP0 = duty;
}
