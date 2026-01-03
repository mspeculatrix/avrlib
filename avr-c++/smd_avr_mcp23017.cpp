#include "smd_avr_mcp23017.h"

SMD_MCP23017::SMD_MCP23017(uint8_t address) : SMD_I2C_Device(address) {
	// _init(address, I2C_BUS_SPEED_STD, 0);
}

void SMD_MCP23017::begin() {
	writeRegister(MCP23017_IOCON, MCP23017_IOCON_CFG);
	_delay_us(MCP23017_RESPONSE_DELAY);
}

uint8_t SMD_MCP23017::readInput(uint8_t port) {
	return readRegister(MCP23017_GPIO + port);
}

void SMD_MCP23017::setIODIR(uint8_t port, uint8_t config) {
	writeRegister(MCP23017_IODIR + port, config);
	_delay_us(MCP23017_RESPONSE_DELAY);
}

void SMD_MCP23017::setOutput(uint8_t port, uint8_t value) {
	writeRegister(MCP23017_GPIO + port, value);
	_delay_us(MCP23017_RESPONSE_DELAY);
}
