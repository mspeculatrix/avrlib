# Base Makefile for ATtiny & ATmega projects

# Edit the following for the project & microcontroller
MCU   = attiny1604			# Microcontroller - choose one
MCU   = atmega4809
MCU   = atmega328p

F_CPU = 20000000UL			# Clock speed

TARGET = SB_Mod_SR04_t1604	# Project/executable name

# ----- PROJECT DEPENDENCIES ---------------------------------------------------

# Edit these lists to include required library files from ~/avrlib/avr-common
# and ~/avrlib/avr-c++

# HEADER-ONLY LIBRARIES - list all header-only dependencies
AVRLIB_HDRS = SB_lib_defines.h \
	smd_std_macros.h

# CUSTOM LIBRARY IMPLEMENTATIONS - list all custom libraries (.cpp files)
AVRLIB_SRCS = SB_modulelib_ng.cpp \
	SB_sr04lib_ng.cpp \
	smd_ng_serial.cpp

# ----- Programmer Settings - select one set -----------------------------------

# For 'classic' AVRs using Atmel-ICE
PROGRAMMER_TYPE = atmelice_isp
PROGRAMMER_ARGS = -B 125kHz

# For 'modern' AVRs using Atmel-ICE
PROGRAMMER_TYPE = atmelice_updi
PROGRAMMER_ARGS = -P usb -B 125kHz

# For 'modern' AVRs using serial UPDI
ROGRAMMER_TYPE = serialupdi
PROGRAMMER_ARGS = -P /dev/ttyUSB0

# ------------------------------------------------------------------------------
# Everything below can be left as-is
# ------------------------------------------------------------------------------

# Library Paths
LIBCOMMON = $(HOME)/avrlib/avr-common
LIBCPP    = $(HOME)/avrlib/avr-c++

# Local project files
LOCAL_SRCS = $(wildcard *.cpp) $(wildcard lib/*.cpp)

# Combine with custom library dependencies
CPPSOURCES = $(LOCAL_SRCS) $(AVRLIB_SRCS)
OBJECTS    = $(addsuffix .o, $(basename $(notdir $(CPPSOURCES))))

# Tell Make where to look for source files
VPATH = $(LIBCOMMON):$(LIBCPP):lib

# Tools
CXX      = avr-g++
OBJCOPY  = avr-objcopy
OBJDUMP  = avr-objdump
AVRSIZE  = avr-size
AVRDUDE  = avrdude

# Compiler flags
CPPFLAGS = -Os -DF_CPU=$(F_CPU) -I. -Ilib -I$(LIBCOMMON) -I$(LIBCPP)
CPPFLAGS += -Wall -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CPPFLAGS += -ffunction-sections -fdata-sections -MMD -MP

# Linker flags
LDFLAGS = -DF_CPU=$(F_CPU) -lm -Os -Wl,-Map,$(TARGET).map -Wl,--gc-sections

# Other flags
TARGET_ARCH = -mmcu=$(MCU)

# ----- Rules ------------------------------------------------------------------

.PHONY: all clean flash size disassemble debug info

all: $(TARGET).hex size

# Compile .cpp to .o
%.o: %.cpp $(AVRLIB_HDRS) Makefile
	$(CXX) $(CPPFLAGS) $(TARGET_ARCH) -c -o $@ $<

$(TARGET).elf: $(OBJECTS)
	$(CXX) $(LDFLAGS) $(TARGET_ARCH) $^ -o $@

%.hex: %.elf
	$(OBJCOPY) -j .text -j .data -j .rodata -O ihex $< $@

%.eeprom: %.elf
	$(OBJCOPY) -j .eeprom --change-section-lma .eeprom=0 -O ihex $< $@

%.lst: %.elf
	$(OBJDUMP) -S $< > $@

# Include dependency files (.d)
-include $(OBJECTS:.o=.d)

# ----- Maintenance ------------------------------------------------------------

size:
	$(AVRSIZE) $(TARGET).elf

clean:
	rm -f *.elf *.hex *.o *.d *.map *.lst lib/*.o

# ----- Actions ----------------------------------------------------------------

flash: $(TARGET).hex
	$(AVRDUDE) -c $(PROGRAMMER_TYPE) -p $(MCU) $(PROGRAMMER_ARGS) -U flash:w:$<

disassemble: $(TARGET).lst

debug: clean
debug: CPPFLAGS += -g -DDEBUG
debug: $(TARGET).hex

info:
	$(AVRDUDE) -c $(PROGRAMMER_TYPE) -p $(MCU) $(PROGRAMMER_ARGS) -v

# ----- Fuse settings ----------------------------------------------------------

# Mega 48, 88, 168, 328 with 16MHz external clock, no div, preserve EEPROM
m328P_extclk_nodiv:
		$(AVRDUDE) -c $(PROGRAMMER_TYPE) -p $(MCU) $(PROGRAMMER_ARGS) \
				-U lfuse:w:0xFF:m -U hfuse:w:0xD6:m -U efuse:w:0xFD:m

# Mega 48, 88, 168, 328 using internal clock, divided by 8
m328P_intclk_nodiv:
		$(AVRDUDE) -c $(PROGRAMMER_TYPE) -p $(MCU) $(PROGRAMMER_ARGS) \
				-U lfuse:w:0x62:m -U hfuse:w:0xDF:m -U efuse:w:0x00:m

# Mega 48, 88, 168, 328 using internal clock, no clock div
m328P_intclk_nodiv:
		$(AVRDUDE) -c $(PROGRAMMER_TYPE) -p $(MCU) $(PROGRAMMER_ARGS) \
				-U lfuse:w:0xE2:m -U hfuse:w:0xDF:m -U efuse:w:0x00:m


m4809_enable_reset:
	$(AVRDUDE) -c $(PROGRAMMER_TYPE) -p $(MCU) \
	           $(PROGRAMMER_ARGS) -U fuse5:w:0xc8:m

m4809_fast_clock:
	$(AVRDUDE) -c $(PROGRAMMER_TYPE) -p $(MCU) \
	           $(PROGRAMMER_ARGS) -U fuse2:w:0x02:m


t1604_fast_clock:
	$(AVRDUDE) -c $(PROGRAMMER_TYPE) -p $(MCU) \
				$(PROGRAMMER_ARGS) -U fuse5:w:0xc4:m
