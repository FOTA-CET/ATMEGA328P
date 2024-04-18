#include "usci.h"
#include "stdio.h"
#include <stdint.h>
#include <avr/delay.h>

#include "main.h"
#include "can.h"
#include "uart.h"


void CAN_send(uint32_t id, uint8_t size, uint8_t *data);
int8_t CAN_receive(uint8_t buffer[]);

int main () 
{
	uint8_t buffer[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};

	// Init UART DEBUG
	// stdout = &mystdout; 	// setup our stdio stream
	// USART_Init(MYUBRR);
	// uint8_t boolian = YesNoQ( "Set Prime mode","Receive","Transmit");


	// Init CAN 
	spi_masterInit();
	can_init(CAN_BITRATE_1000); // 500KBPS

	uint8_t boolian = 1; // Only transmit data
	if(boolian)
	{
		while(1)
		{
			if (CAN_receive(buffer) == 1)
			{
				// printf("%c ",can.data.data_u8[0]);
				// printf("%c ",can.data.data_u8[1]);
				// printf("%c ",can.data.data_u8[2]);
				// printf("%c ",can.data.data_u8[3]);
				// printf("%c ",can.data.data_u8[4]);
				// printf("%c ",can.data.data_u8[5]);
				// printf("%c ",can.data.data_u8[6]);
				// printf("%c \n ",can.data.data_u8[7]);
				CAN_send(0x123, 8, can.data.data_u8);
			}
		}
	}
	else 
	{
		while(1)
		{
			_delay_ms(1000);
			CAN_send(0x10, 8, buffer);

		}
	}
	return 1;
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
	PORTB |= (1<<PB1);
	can_read( CANINTF, buffer, 1 );
	if(buffer[0]&0x03)
	{
		can_receive();
		return 1;
	}
	else return -1;
}