/*
* main.c
*
* Created : 06-04-2024
* Author  : Thanh Tung
*/

#define F_CPU 16000000UL // Defining the CPU Frequency

#include <avr/io.h>
#include <util/delay.h>
#include <avr/boot.h>
#include <avr/interrupt.h>

#define USART_BAUDRATE 9600 // Desired Baud Rate
#define BAUD_PRESCALER (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)

#define ASYNCHRONOUS (0<<UMSEL00) // USART Mode Selection

// Define parity configs
#define DISABLED    (0<<UPM00)
#define EVEN_PARITY (2<<UPM00)
#define ODD_PARITY  (3<<UPM00)

// Select DISABLE PARITY
#define PARITY_MODE  DISABLED

// Define stop bit configs
#define ONE_BIT (0<<USBS0)
#define TWO_BIT (1<<USBS0)

// Select one bit stop
#define STOP_BIT ONE_BIT

// Define data length
#define FIVE_BIT  (0<<UCSZ00)
#define SIX_BIT   (1<<UCSZ00)
#define SEVEN_BIT (2<<UCSZ00)
#define EIGHT_BIT (3<<UCSZ00)

// Select 8 bit length
#define DATA_BIT   EIGHT_BIT

uint8_t count = 0;

uint8_t data_offset = 0;
uint16_t offset = 0;
uint16_t program_size = 0;
uint8_t flag = 0;

uint8_t data_hex_buf[1024]={};
uint8_t program_size_buf[32]={};
uint8_t OK_MEN[] = "OK MEN !\r\n";
uint8_t READ_OK[] = "Read OK !\r\n";

// Chua dung
// uint8_t WRITE_FAIL[]= "Write Fall !\r\n";

void USART_Init()
{
	// Set Baud Rate
	UBRR0H = BAUD_PRESCALER >> 8;
	UBRR0L = BAUD_PRESCALER;
	
	// Set Frame Format
	UCSR0C = ASYNCHRONOUS | PARITY_MODE | STOP_BIT | DATA_BIT;
	
	// Enable Receiver and Transmitter
	UCSR0B = (1<<RXEN0) | (1<<TXEN0);
}

uint8_t USART_ReceivePolling()
{
	uint8_t DataByte;
	while (( UCSR0A & (1<<RXC0)) == 0) {}; // Do nothing until data have been received
	DataByte = UDR0 ;
	return DataByte;
}

void USART_TransmitPolling(uint8_t DataByte)
{
	while (( UCSR0A & (1<<UDRE0)) == 0) {}; // Do nothing until UDR is ready
	UDR0 = DataByte;
}

void USART_TransmitString(uint8_t *data)
{
  while(*data != 0)
	{
		USART_TransmitPolling(*data);
		data++;
	}
}
// Minimum 2 bytes data flash and only work with 0x7C00 config
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

int main()
{
	USART_Init();
	while (1)
	{
    if(!flag)
    {
      program_size_buf[offset] = USART_ReceivePolling();
      if (program_size_buf[offset] == '\n')
      {
        uint32_t ts = 1;
        for(uint8_t i = offset - 1; i >= 0; i--)
        {
          if(program_size_buf[i] == '=') break;
          program_size += (uint32_t)(program_size_buf[i] - 48)*ts;
          ts*=10;
        }
        USART_TransmitString(OK_MEN);
        flag = 1;
        offset = 0;
        continue;
      }
      offset++;
    }
    else if(flag)
    {
      if(offset == 1024)
      {
        USART_TransmitString(READ_OK);
        uint32_t ADDRESS_PAGE = ((uint32_t)0x00000 + (data_offset*1024)); 
        write_program_pages(ADDRESS_PAGE,data_hex_buf,sizeof(data_hex_buf));
        for(uint16_t i = 0; i < 1024;i++)
        {
          data_hex_buf[i] = 0xFF;
        }
        data_offset++;
        offset = 0;
      }
      else
      {
        data_hex_buf[offset] = USART_ReceivePolling();
        offset++;
      }

      if(offset + data_offset*1024 == program_size)
      {
        USART_TransmitString(READ_OK);
        uint32_t ADDRESS_PAGE = ((uint32_t)0x00000 + (data_offset*1024)); 
        write_program_pages(ADDRESS_PAGE,data_hex_buf,offset);
        asm("jmp 0");
      }
    }
	}
	return 0;
}