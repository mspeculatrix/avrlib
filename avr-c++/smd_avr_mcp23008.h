/*

Typical use:

#include "lib/smd_avr_mcp23008.h"
using namespace smd_avr_i2c;

In globals section:
SMD_MCP23008 dataport = SMD_MCP23008(0x21 << 1);

In setup:
I2C_enable_pullups(&I2C_PORT, I2C_SDA_GPIO, I2C_SCL_GPIO);

dataport.begin();

dataport.setIODIR(OUTPUT);

In main loop:
dataport.setOutput(0xAA);

*/
#ifndef __SMD_AVR_MCP23008_H__
#define __SMD_AVR_MCP23008_H__

#include "smd_avr_i2clib.h"

// Register addresses
#define MCP23008_IODIR   0x00
#define MCP23008_IPOL    0x01
#define MCP23008_GPINTEN 0x02
#define MCP23008_DEFVAL  0x03
#define MCP23008_INTCON  0x04
#define MCP23008_IOCON   0x05
#define MCP23008_GPPU    0x06
#define MCP23008_INTF    0x07
#define MCP23008_INTCAP  0x08
#define MCP23008_GPIO    0x09
#define MCP23008_OLAT    0x0A

#define MCP23008_IOCON_CFG 0b00100000
// The following delay turns out to be IMPORTANT. The datasheet recommends
// a minimum of 5µs.
#define MCP23008_RESPONSE_DELAY 10 // µs delay for commands to take effect

class SMD_MCP23008 : public SMD_I2C_Device
{
public:
	SMD_MCP23008(uint8_t address);
	void begin();
	uint8_t readInput();
	void setIODIR(uint8_t config);
	void setOutput(uint8_t value);
};

#endif
