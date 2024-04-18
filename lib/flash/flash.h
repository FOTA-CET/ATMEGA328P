#ifndef __FLASH_H
#define __FLASH_H
#include <stdint.h>
#include <avr/io.h>
#include <avr/boot.h>
#include <avr/interrupt.h>

void write_program_pages(uint32_t first_page_address, uint8_t *program_buffer, uint16_t size);

#endif