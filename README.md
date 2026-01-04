# AVRLIB

C, C++ and assembly library files for development on AVR microcontrollers.

These are my personal libraries and are nearly all in a constant state of refurbishment. Nothing is guaranteed to be working or even complete. Use at your own risk.

## AVR-ASM

Useful stuff for working with AVR assembly on 'classic' 8-bit microcontrollers. I've only toyed with assembly out of curiosity, so don't expect anything clever.

## AVR-C

As you'd expect, libraries for C code. I don't use plain C very much, so these are quite old. Might come in handy, though.

## AVR-C++

The naming convention for these libraries is:

- **smd_avr_\*** - Libraries for 'classic' 8-bit AVR microcontrollers.
- **smd_ng_\*** - Libraries for 'new generation' or 'modern` 8-bit AVR microcontrollers, such as the ATtiny1604 or ATmega4809.
- **SB_\*** - Libraries for the SensorBus protocol for my sensor and robotics projects. These have a `_ng` suffix when they are for new generation chips (which is likely to be the standard).

## AVR-COMMON

These are header-only files, mostly defines/macros. They are intended to work on any AVR chip.
