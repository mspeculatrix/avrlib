#ifndef __SB_NODELIB_NG__
#define __SB_NODELIB_NG__

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include <smd_std_macros.h>
#include <SB_lib_defines.h>

#include <SB_devicelib_ng.h>

using namespace SensorBus;

class SB_Node : public SB_Device {

public:
	SB_Node(volatile PORT_t* port, uint8_t clkPin_pm,
		uint8_t actPin_pm, PORT_t* dat_port);

protected:

	void _setDefaults(void);	// overloads parent method
};

#endif