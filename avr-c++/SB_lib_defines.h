/*
 * Some essential defines for SensorBus stuff. This is in a separate file
 * because I expect the list of items to grow and because the SensorBus
 * libraries are going to be specific to certain microcontrollers (or families)
 * whereas this is meant to be universal.
*/

#ifndef __SB_LIB_DEFINES_H__
#define __SB_LIB_DEFINES_H__


// MESSAGE TYPES

#define SBMSG_USONIC_DATA_US 10 	// Ultrasonic sensor data
#define SBMSG_SET_PARAM 20	// msg from node to module, set a parameter

#endif
