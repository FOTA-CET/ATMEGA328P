#include "usci.h"
#include "stdio.h"
#include <stdint.h>

#include "main.h"
#include "can.h"
// #include "uart.h"
#include "flash.h"
#include <string.h>
#include <avr/boot.h>

void CAN_send(uint32_t id, uint8_t size, uint8_t *data);
int8_t CAN_receive(uint8_t buffer[]);

uint8_t count = 0;

uint16_t addr_data_offset = 0;
uint16_t size_count_can = 1;
uint16_t program_size = 0;
uint8_t flag = 0;

uint8_t flash_page_buff[128] = {};

uint8_t DLC[1] = {};
uint8_t percentBuf[1] = {};
uint16_t counter = 0;
uint8_t currentPercent = -1;

uint8_t timeoutFlag = 0;

#define PERCENT_ADDR 0x100
#define OTA_ADDRESS (uint32_t)0x3800
#define CURRENT_ADDRESS (uint32_t)0x0000
#define NUM_PAGES_TO_COPY 112
#define FLASH_PAGE_SIZE 128 

int8_t processPercentCAN(uint16_t firmwareSize, uint16_t count);
int8_t CAN_receive(uint8_t buffer[]);
void CAN_send(uint32_t id, uint8_t size, uint8_t data[]);
void updateCurrent(uint32_t src_addr, uint32_t dest_addr);
void timerInit(uint16_t timeout_ms);
void gotoApp(void);

ISR(TIMER1_COMPA_vect)
{
	if (!timeoutFlag)
	{
		gotoApp();
	}
	else
	{
		timeoutFlag = 0;
	}
}
int main(int argc, char **argv)
{
	/* setup our stdio stream */
	// stdout = &mystdout;
	// USART_Init(MYUBRR);

	/* Buffer store CAN receive data */
	uint8_t buffer[8] = {};

	/* Init CAN */
	spi_masterInit();
	can_init(CAN_BITRATE_1000); // 500KBPS

	timerInit(5000);
	sei();

	while (1)
	{
		if (CAN_receive(buffer) == 1)
		{
			uint32_t ADDRESS = ((uint32_t)OTA_ADDRESS + (addr_data_offset * 128));

			if (can.address == 0x03)
			{
				timeoutFlag = 1;
				uint32_t ts = 1;
				can_read(RXB0DLC, DLC, 1); // Get data length of CAN MESSAGE
				for (int8_t i = DLC[0] - 1; i >= 0; i--)
				{
					program_size += (uint32_t)(can.data.data_u8[i] * ts);
					ts *= 10;
				}
				flag = 1;
			}

			if (can.address == 0x04 && flag == 1)
			{
				timeoutFlag = 1;
				if (count == 128)
				{
					write_program_pages(ADDRESS, flash_page_buff, sizeof(flash_page_buff));
					memset(flash_page_buff, 0xFF, 128);
					addr_data_offset++;
					count = 0;
				}

				if ((uint32_t)size_count_can * 8 >= program_size)
				{
					can_read(RXB0DLC, DLC, 1); // Get data length of CAN MESSAGE
					for (uint8_t i = 0; i < DLC[0]; i++)
					{
						flash_page_buff[count + i] = can.data.data_u8[i];
					}
					write_program_pages(ADDRESS, flash_page_buff, sizeof(flash_page_buff));

					percentBuf[0] = 100;
					CAN_send(PERCENT_ADDR, 1, percentBuf);

					updateCurrent(OTA_ADDRESS, CURRENT_ADDRESS);
					gotoApp();
				}
				else
				{

					for (uint8_t i = 0; i < 8; i++)
					{
						int8_t percent = processPercentCAN(program_size, counter++);
						if (percent != -1 && percent != currentPercent)
						{
							currentPercent = percent;
							percentBuf[0] = percent;
							CAN_send(PERCENT_ADDR, 1, percentBuf);
						}
					}

					for (uint8_t i = 0; i < 8; i++)
					{
						flash_page_buff[count + i] = can.data.data_u8[i];
					}
					count += 8;
					size_count_can++;
				}
			}
		}
	}
	return 0;
}

void timerInit(uint16_t timeout_ms)
{
	TCCR1B |= (1 << WGM12) | (1 << CS12) | (1 << CS10);
	OCR1A = (F_CPU / 1000 / 1024) * timeout_ms - 1;
	TIMSK1 |= (1 << OCIE1A);
}

void gotoApp()
{
	_SFR_IO8(0x2C) = 0;
	_SFR_IO8(0x2D) = 0;
	DDRB = 0;

	asm volatile("jmp 0");
}

void updateCurrent(uint32_t src_addr, uint32_t dest_addr)
{
	uint32_t address;
	uint32_t address_2;
	for (int i = 0; i < NUM_PAGES_TO_COPY; i++)
	{
		address = ((uint32_t)dest_addr + (i * 128));
		address_2 = ((uint32_t)src_addr + (i * 128));
		for (int j = 0; j < FLASH_PAGE_SIZE; j++)
		{
			flash_page_buff[j] = pgm_read_byte(address_2 + j);
		}
		write_program_pages(address, flash_page_buff, sizeof(flash_page_buff));
	}
}

void CAN_send(uint32_t id, uint8_t size, uint8_t data[])
{
	can_push_ptr->address = id;
	can_push_ptr->status = size;

	can_push_ptr->data.data_u8[0] = data[0];
	can_push_ptr->data.data_u8[1] = data[1];
	can_push_ptr->data.data_u8[2] = data[2];
	can_push_ptr->data.data_u8[3] = data[3];
	can_push_ptr->data.data_u8[4] = data[4];
	can_push_ptr->data.data_u8[5] = data[5];
	can_push_ptr->data.data_u8[6] = data[6];
	can_push_ptr->data.data_u8[7] = data[7];

	can_push();
	can_transmit();
}

int8_t CAN_receive(uint8_t buffer[])
{
	PORTB |= (1 << PB1);
	can_read(CANINTF, buffer, 1);
	if (buffer[0] & 0x03)
	{
		can_receive();
		return 1;
	}
	else
		return -1;
}

int8_t processPercentCAN(uint16_t firmwareSize, uint16_t count)
{
	uint8_t percent = (uint8_t)(((uint32_t)count * 100 + firmwareSize / 2) / firmwareSize);
	if(percent < 100) return percent;
	return -1;
}