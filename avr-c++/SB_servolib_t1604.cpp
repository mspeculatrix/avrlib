/* SB_servolib_ng.cpp */

#include "SB_servolib_t1604.h"

using namespace SB_Servo;

/**
 * @brief CONSTRUCTORS
 */

 // Version using all default values
SB_Servo_t1604::SB_Servo_t1604(void)
	: SB_Module(_SB_PORT, _SB_CLK, _SB_ACT, _SB_DAT, _SB_DATPORT, _SB_DATCTRL) {
	SB_Servo_t1604(_TICKS_MIN_DFL, _TICKS_MAX_DFL,
		_TICKS_MIN_DFL, _TICKS_MAX_DFL);
}

// Version using passed values for Servo A, defaults for Servo B
SB_Servo_t1604::SB_Servo_t1604(uint16_t minA, uint16_t maxA)
	: SB_Module(_SB_PORT, _SB_CLK, _SB_ACT, _SB_DAT, _SB_DATPORT, _SB_DATCTRL) {
	SB_Servo_t1604(minA, maxA, _TICKS_MIN_DFL, _TICKS_MAX_DFL);
}

// Version using passed values for both Servo A and Servo B
SB_Servo_t1604::SB_Servo_t1604(uint16_t minA, uint16_t maxA,
	uint16_t minB, uint16_t maxB)
	: SB_Module(_SB_PORT, _SB_CLK, _SB_ACT, _SB_DAT, _SB_DATPORT, _SB_DATCTRL) {
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

	PORTA.DIRSET = _LED_A_PIN | _LED_B_PIN;		// Set LED pins as outputs
	PORTB.DIRSET = _SERVO_A_PIN | _SERVO_B_PIN;	// Set servo pins as outputs

	// Set TCA0 to Normal (single slope) mode and enable WO0 and W01
	// Not using WO2 as this clashes with the USART
	TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_SINGLESLOPE_gc
		| TCA_SINGLE_CMP0EN_bm		// enable WO0 on PB0
		| TCA_SINGLE_CMP1EN_bm;		// enable WO1 on PB1

	// Set the period (16-bit register)
	TCA0.SINGLE.PER = _PERIOD_20MS; 	// 50Hz fequency

	// Set the clock prescaler to 64 and enable the timer
	TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV64_gc | TCA_SINGLE_ENABLE_bm;

	serial.begin();
	serial.writeln("Servo module initialised.");
}

/**
 * @brief Print an incoming SB message via serial
 * @param buf pointer to the buffer
 */
void SB_Servo_t1604::printMsg(uint8_t* buf) {
	for (uint8_t i = 0; i < buf[0]; i++) {
		serial.write(buf[i]);
		serial.write(" ");
	}
	serial.writeln(" ");
}

/**
 * @brief Get SensorBus message.
 * Overwrites parent method.
 */
SensorBus::err_code SB_Servo_t1604::recvMessage(void) {
	return SB_Module::recvMessage(_SB_DAT);
}

/**
 * @brief Take appropriate actions according to message received.
 */
void SB_Servo_t1604::respondToMessage(void) {
	switch (recvMsgBuf[1]) {
		case SBMSG_SET_PARAM:
			if (recvMsgBuf[2] < 255) {
				setAngle(SERVO_A, recvMsgBuf[2]);
			}
			if (recvMsgBuf[3] < 255) {
				setAngle(SERVO_B, recvMsgBuf[3]);
			}
			break;
	}
}

/**
 * @brief Set the servo angle (0-180)
 * @param servo uint8_t SERVO_A or SERVO_B
 * @param angle uint8_t 0-180
 */
void SB_Servo_t1604::setAngle(const uint8_t servo, uint8_t angle) {
	switch (servo) {
		case SERVO_A:
			if (angle < _angleMin_A) angle = _angleMin_A;
			if (angle > _angleMax_A) angle = _angleMax_A;
			TCA0.SINGLE.CMP0 = _ticksMin_A +
				((uint32_t)(_ticksMax_A - _ticksMin_A) * angle) / 180;
			break;
		case SERVO_B:
			if (angle < _angleMin_B) angle = _angleMin_B;
			if (angle > _angleMax_B) angle = _angleMax_B;
			TCA0.SINGLE.CMP1 = _ticksMin_B +
				((uint32_t)(_ticksMax_B - _ticksMin_B) * angle) / 180;
			break;
	}
}

/**
 * @brief Set an LED state
 * @param led LED_A or LED_B
 * @param state ON or OFF
 */
void SB_Servo_t1604::setLED(uint8_t led, uint8_t state) {
	if (state == ON) {
		PORTA.OUTSET = _LED_PINS[led];
	} else {
		PORTA.OUTCLR = _LED_PINS[led];
	}
}

/**
 * @brief Set the minimum acceptable servo angle (0-180)
 * @param servo uint8_t SERVO_A or SERVO_B
 * @param angle uint8_t 0-180
 */
void SB_Servo_t1604::setMinAngle(const uint8_t servo, uint8_t angle) {
	switch (servo) {
		case SERVO_A:
			_angleMin_A = angle;
			break;
		case SERVO_B:
			_angleMin_B = angle;
			break;
	}
}

/**
 * @brief Set the maximum acceptable servo angle (0-180)
 * @param servo uint8_t SERVO_A or SERVO_B
 * @param angle uint8_t 0-180
 */
void SB_Servo_t1604::setMaxAngle(const uint8_t servo, uint8_t angle) {
	switch (servo) {
		case SERVO_A:
			_angleMax_A = angle;
			break;
		case SERVO_B:
			_angleMax_B = angle;
			break;
	}
}

/**
 * @brief Toggle an LED state
 * @param led LED_A or LED_B
 */
void SB_Servo_t1604::toggleLED(uint8_t led) {
	PORTA.OUTTGL = _LED_PINS[led];
}
