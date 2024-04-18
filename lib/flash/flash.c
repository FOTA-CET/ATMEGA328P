#include "flash.h"
void write_program_pages(uint32_t first_page_address, uint8_t *program_buffer, uint16_t size)
{
  uint16_t b;
  uint16_t p;
  uint16_t current_page_size;
  uint32_t current_page_address;
  uint8_t sreg_last_state;

  uint16_t SIZE_IN_BYTES = size;
  uint8_t PROGRAM_NUMBER_OF_PAGES =  (SIZE_IN_BYTES / SPM_PAGESIZE) + ((SIZE_IN_BYTES % SPM_PAGESIZE == 0) ? 0 : 1);

  // Disable interrupts.
  sreg_last_state = SREG;
  cli();

  eeprom_busy_wait();

  for (p = 0; p < PROGRAM_NUMBER_OF_PAGES; p++)
  {
    // Calculate current page size in bytes
    if (p == PROGRAM_NUMBER_OF_PAGES - 1)
    {
      // Last page size
      current_page_size = SIZE_IN_BYTES - SPM_PAGESIZE * (PROGRAM_NUMBER_OF_PAGES - 1);
    }
    else
    {
      // Other page sizes
      current_page_size = SPM_PAGESIZE;
    }

    current_page_address = first_page_address + p * SPM_PAGESIZE;

    boot_page_erase(current_page_address);
    boot_spm_busy_wait(); // Wait until the memory is erased.

    for (b = 0; b < current_page_size; b += 2)
    {
      // Set up little-endian word
      uint16_t w = *program_buffer++;
      w += (*program_buffer++) << 8;

      boot_page_fill(current_page_address + b, w);
    }

    boot_page_write(current_page_address); // Store buffer in flash page.
    boot_spm_busy_wait();                  // Wait until the memory is written.
  }

  // Re-enable RWW-section again. We need this if we want to jump back
  // to the application after bootloading.
  boot_rww_enable();

  // Re-enable interrupts (if they were ever enabled).
  SREG = sreg_last_state;
}