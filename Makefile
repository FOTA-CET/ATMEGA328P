BUILD_DIR = build

DEVICE     = atmega328p
CLOCK      = 16000000
OBJECTS    = src/main.o src/can.o src/usci.o src/uart.o

AVRDUDE = avrdude -p$(DEVICE)
LDFLAGS = -Wl,-section-start=.text=0000
COMPILE = avr-gcc $(LDFLAGS) -Wall -Os -DF_CPU=$(CLOCK) -mmcu=$(DEVICE) -std=gnu99

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

all:	main.hex avr-size

# create the build directory
$(BUILD_DIR):
	$(MK_DIR) $(BUILD_DIR)

%.o:	%.c
	$(COMPILE) -c $< -o $@

clean:
	rm -f $(BUILD_DIR)/main.hex $(BUILD_DIR)/main.elf $(OBJECTS)

main.elf: $(PLATFORM_OBJECTS) $(OBJECTS)
	$(COMPILE) -o $(BUILD_DIR)/main.elf $(OBJECTS)

main.hex: $(BUILD_DIR) main.elf
	rm -f $(BUILD_DIR)/main.hex
	avr-objcopy -j .text -j .data -O ihex $(BUILD_DIR)/main.elf $(BUILD_DIR)/main.hex

avr-size: $(BUILD_DIR)/main.elf
	avr-size $(BUILD_DIR)/main.elf --mcu=$(DEVICE) --format=avr
