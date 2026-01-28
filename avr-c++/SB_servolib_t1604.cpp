/* SB_servolib_ng.cpp */

#include "SB_servolib_t1604.h"

using namespace SB_Servo;

/**
 * @brief CONSTRUCTOR
 *
 * Currently just calls the parent class' constructor.
 */
SB_Servo_t1604::SB_Servo_t1604(PORT_t* sbPort, uint8_t sbClk, uint8_t sbAct, uint8_t sbDat,
	PORT_t* datPort, volatile uint8_t* sbDatCtrl)
	: SB_Module(sbPort, sbClk, sbAct, sbDat, datPort, sbDatCtrl) {}

/**
 * @brief Initialise servo board and motor(s).
 *
 * Call once in main() before entering the main loop.
 */
void SB_Servo_t1604::begin(void) {

	PORTA.DIRSET = PIN4_bm | PIN7_bm;	// Set LED pins as outputs
	PORTB.DIRSET = PIN0_bm | PIN1_bm;	// Set servo signal pins as outputs

	// Set TCA0 to Normal (single slope) mode and enable WO0 and W01
	// Not using WO2 as this clashes with the USART
	TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_SINGLESLOPE_gc
		| TCA_SINGLE_CMP0EN_bm		// enable WO0 on PB0
		| TCA_SINGLE_CMP1EN_bm;		// enable WO1 on PB1

	// Set the period (16-bit register)
	TCA0.SINGLE.PER = _PERIOD_20MS; 	// 50Hz fequency

	// Set the clock prescaler to 64 and enable the timer
	TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV64_gc | TCA_SINGLE_ENABLE_bm;
}

/**
 * @brief Set the servo angle (0-180)
 * @param pin uint8_t SB_SERVO_A (0) or SB_SERVO_B (1)
 * @param angle uijnt8_t 0 to 180
 */
void SB_Servo_t1604::setAngle(uint8_t pin, uint8_t angle) {
	if (angle < 0) angle = 0;
	if (angle > 180) angle = 180;

	// Linear mapping from angle to pulse width
	// Using 32-bit math for the intermediate step to prevent overflow
	uint16_t duty = _TICKS_MIN +
		((uint32_t)(_TICKS_MAX - _TICKS_MIN) * angle) / 180;

	// Update Compare Register
	switch (pin) {
		case SB_SERVO_A:
			TCA0.SINGLE.CMP0 = duty;
			break;
		case SB_SERVO_B:
			TCA0.SINGLE.CMP1 = duty;
			break;
	}
}
