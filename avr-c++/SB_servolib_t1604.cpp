/* SB_servolib_ng.cpp */

#include "SB_servolib_t1604.h"

using namespace SB_Servo;

/**
 * @brief CONSTRUCTORS
 */

 // Version using all default values
SB_Servo_t1604::SB_Servo_t1604(PORT_t* sbPort, uint8_t sbClk, uint8_t sbAct,
	uint8_t sbDat, PORT_t* datPort, volatile uint8_t* sbDatCtrl)
	: SB_Module(sbPort, sbClk, sbAct, sbDat, datPort, sbDatCtrl) {
	_ticksMin_A = _TICKS_MIN_DFL;
	_ticksMax_A = _TICKS_MAX_DFL;
	_ticksMin_B = _TICKS_MIN_DFL;
	_ticksMax_B = _TICKS_MAX_DFL;
}

// Version using passed values for Servo A, defaults for Servo B
SB_Servo_t1604::SB_Servo_t1604(uint16_t minA, uint16_t maxA,
	PORT_t* sbPort, uint8_t sbClk, uint8_t sbAct, uint8_t sbDat,
	PORT_t* datPort, volatile uint8_t* sbDatCtrl)
	: SB_Module(sbPort, sbClk, sbAct, sbDat, datPort, sbDatCtrl) {
	_ticksMin_A = minA;
	_ticksMax_A = maxA;
	_ticksMin_B = _TICKS_MIN_DFL;
	_ticksMax_B = _TICKS_MAX_DFL;
}

// Version using passed values for both Servo A and Servo B
SB_Servo_t1604::SB_Servo_t1604(uint16_t minA, uint16_t maxA,
	uint16_t minB, uint16_t maxB,
	PORT_t* sbPort, uint8_t sbClk, uint8_t sbAct, uint8_t sbDat,
	PORT_t* datPort, volatile uint8_t* sbDatCtrl)
	: SB_Module(sbPort, sbClk, sbAct, sbDat, datPort, sbDatCtrl) {
	_ticksMin_A = minA;
	_ticksMax_A = maxA;
	_ticksMin_B = minB;
	_ticksMax_B = maxB;

}

/** *** METHODS *** */

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
 * @param servo uint8_t SB_SERVO_A or SB_SERVO_B
 * @param angle uint8_t 0-180
 */
void SB_Servo_t1604::setAngle(uint8_t servo, uint8_t angle) {
	switch (servo) {
		case SB_SERVO_A:
			if (angle < _angleMin_A) angle = _angleMin_A;
			if (angle > _angleMax_A) angle = _angleMax_A;
			TCA0.SINGLE.CMP0 = _ticksMin_A +
				((uint32_t)(_ticksMax_A - _ticksMin_A) * angle) / 180;
			break;
		case SB_SERVO_B:
			if (angle < _angleMin_B) angle = _angleMin_B;
			if (angle > _angleMax_B) angle = _angleMax_B;
			TCA0.SINGLE.CMP1 = _ticksMin_B +
				((uint32_t)(_ticksMax_B - _ticksMin_B) * angle) / 180;
			break;
	}
}

/**
 * @brief Set the minimum acceptable servo angle (0-180)
 * @param servo uint8_t SB_SERVO_A or SB_SERVO_B
 * @param angle uint8_t 0-180
 */
void SB_Servo_t1604::setMinAngle(uint8_t servo, uint8_t angle) {
	switch (servo) {
		case SB_SERVO_A:
			_angleMin_A = angle;
			break;
		case SB_SERVO_B:
			_angleMin_B = angle;
			break;
	}
}

/**
 * @brief Set the maximum acceptable servo angle (0-180)
 * @param servo uint8_t SB_SERVO_A or SB_SERVO_B
 * @param angle uint8_t 0-180
 */
void SB_Servo_t1604::setMaxAngle(uint8_t servo, uint8_t angle) {
	switch (servo) {
		case SB_SERVO_A:
			_angleMax_A = angle;
			break;
		case SB_SERVO_B:
			_angleMax_B = angle;
			break;
	}
}
