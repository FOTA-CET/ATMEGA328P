SRC_DIR = src
BUILD_DIR = build


# The CC variable typically represents the C compiler that will be used during the
# compilation process. By assigning avr-gcc to CC, it indicates that the avr-gcc
# compiler is being used for compiling the C source files in the project.

CC = avr-gcc

# Compiler Options:

# -Wall:
# This flag enables a set of warning messages during compilation. It helps to
# catch potential issues or suspicious code constructs that might lead to bugs
# or undefined behavior.

# -Os:
# This flag optimizes the generated code for size. It instructs the compiler to
# perform various optimizations that reduce the code size, although it may have a
# slight impact on execution speed. This is often desirable for resource-constrained
# microcontrollers where minimizing code size is important.

# -mmcu=atmega328p:
# This flag specifies the target microcontroller for which the code is being compiled.
# In this case, it is set to the Atmega328P microcontroller. The compiler uses this
# information to set the appropriate processor-specific options and generate code
# tailored for the specified microcontroller.

# -std=gnu99:
# This flag sets the C language standard to C99. It instructs the compiler to use the C99
# standard for language features and syntax. The C99 standard introduced several new
# features and improvements over previous versions of the C language.

CFLAGS = -Wall -Os -mmcu=atmega328p -std=gnu99

LDFLAGS = -Wl,-section-start=.text=0x7C00

# Define MK_DIR based on the operating system or environment in which the Makefile is being executed
ifdef SystemRoot
	MK_DIR = mkdir
else
	ifeq ($(shell uname), Linux)
		MK_DIR = mkdir -pv
	endif

	ifeq ($(shell uname | cut -d _ -f 1), CYGWIN)
		MK_DIR = mkdir -pv
	endif

	ifeq ($(shell uname | cut -d _ -f 1), MINGW32)
		MK_DIR = mkdir -pv
	endif

	ifeq ($(shell uname | cut -d _ -f 1), MINGW64)
		MK_DIR = mkdir -pv
	endif
endif

all: hex_and_bin

# create the build directory
$(BUILD_DIR):
	$(MK_DIR) $(BUILD_DIR)

# create the object file for main.c
main.o: $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/main.o -c $(SRC_DIR)/main.c

# link and build the program in ELF format for Atmega328P
program.elf: $(BUILD_DIR) main.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(BUILD_DIR)/program.elf $(BUILD_DIR)/main.o

# convert ELF file to HEX and BIN files to be programmed on Atmega328P
hex_and_bin: $(BUILD_DIR) program.elf
	avr-objcopy -j .text -j .data -O ihex $(BUILD_DIR)/program.elf $(BUILD_DIR)/program.hex
	avr-objcopy -j .text -j .data -O binary $(BUILD_DIR)/program.elf $(BUILD_DIR)/program.bin
	avr-size --format=avr --mcu=atmega328p $(BUILD_DIR)/program.elf

# generate the assembler file of the program (this is only for studying assembly instructions)
assemble: $(BUILD_DIR) program.elf
	avr-objdump -d -S $(BUILD_DIR)/program.elf > $(BUILD_DIR)/assembled.s

# disassemble the hex file, which can be either generated by the compiler or dumped from the MCU,
# and generate the assembly file of the program (this is only for studying assembly instructions)
disassemble: $(BUILD_DIR) hex_and_bin
	avr-objcopy -I ihex -O elf32-avr $(BUILD_DIR)/program.hex $(BUILD_DIR)/disassembled.elf
	avr-objdump -D -m avr $(BUILD_DIR)/disassembled.elf > $(BUILD_DIR)/disassembled.s

# upload the built program (hex file) to Atmega328P
upload: hex_and_bin
	avrdude -c ft232h -p m328p -U flash:w:$(BUILD_DIR)/program.hex
# avrdude:
# The command-line tool used for programming AVR microcontrollers.

# -c ft232h:
# Specifies the programmer type. In this case, it indicates that an FT232H board
# is being used as the programmer. The specific programmer type may vary depending
# on the hardware setup.

# -p m328p:
# Specifies the target AVR microcontroller model. Here, it's set to m328p, which corresponds
# to the ATmega328P microcontroller.

# -U flash:w:$(BUILD_DIR)/program.hex:
# Specifies the action to be performed by avrdude. In this case, it is set to
# write the contents of the program.hex file to the flash memory (flash:w:) of the microcontroller.
# The program.hex file is located in the build directory.

# Remove build directory with all built files
clean:
	rm -rf $(BUILD_DIR) 
