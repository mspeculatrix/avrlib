/*

NB: The MCP23017 has a bug that means A7 and B7 can't be used as inputs,
only outputs.

Typical use:

#include "lib/smd_avr_mcp23017.h"
using namespace smd_avr_i2c;

In globals section:
SMD_MCP23017 addrport = SMD_MCP23017(0x21 << 1);

In setup:
I2C_enable_pullups(&I2C_PORT, I2C_SDA_GPIO, I2C_SCL_GPIO);
addrport.begin();

addrport.setIODIR(MCP23017_PORTA, OUTPUT);
addrport.setIODIR(MCP23017_PORTB, OUTPUT);

In main loop:
addrport.setOutput(MCP23017_PORTA, 0xAA);

*/


#ifndef __SMD_AVR_MCP23017_H__
#define __SMD_AVR_MCP23017_H__

#include "smd_avr_i2clib.h"

#define MCP23017_PORTA 0
#define MCP23017_PORTB 1

// Following is default location of IOCON register after power-on reset.
// Need to write to this to set the bank bit.
#define MCP23017_IOCON_CFG 0b00100000

#define MCP23017_IODIR   0x00
#define MCP23017_IPOL    0x02
#define MCP23017_GPINTEN 0x04
#define MCP23017_DEFVAL  0x06
#define MCP23017_INTCON  0x08
#define MCP23017_IOCON   0x0A
#define MCP23017_GPPU    0x0C
#define MCP23017_INTF    0x0E
#define MCP23017_INTCAP  0x10
#define MCP23017_GPIO    0x12
#define MCP23017_OLAT    0x14

// The following delay turns out to be IMPORTANT. The datasheet recommends
// a minimum of 5µs.
#define MCP23017_RESPONSE_DELAY  10 // µs delay for commands to take effect

class SMD_MCP23017 : public SMD_I2C_Device
{
public:
	SMD_MCP23017(uint8_t address);
	void begin();
	uint8_t readInput(uint8_t port);
	void setIODIR(uint8_t port, uint8_t config);
	void setOutput(uint8_t port, uint8_t value);
};


#endif