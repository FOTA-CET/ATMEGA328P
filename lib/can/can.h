/*
 * Tritium MCP2515 CAN interface header
 * Copyright (c) 2015, Tritium Pty Ltd.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *  - Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 *	- Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer 
 *	  in the documentation and/or other materials provided with the distribution.
 *	- Neither the name of Tritium Pty Ltd nor the names of its contributors may be used to endorse or promote products 
 *	  derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
 * IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE. 
 *
 */

#ifndef CAN_H
#define CAN_H

#include "main.h"
#include <avr/io.h>


// Public function prototypes
void can_init( unsigned int bitrate_index );
char can_transmit( void );
void can_receive( void );
void can_push( void );
// void can_abort_transmit( void );
// void can_sleep( void );
// void can_wake( void );
void can_read( unsigned char address, unsigned char *ptr, unsigned char bytes );

// Public variables
typedef struct _can_variables {
	unsigned int		status;
	unsigned int 		address;
	group_64			data;
} can_variables;

#define CAN_BUF_LEN		32
extern can_variables	can;
extern can_variables	canq[CAN_BUF_LEN];
extern can_variables	*can_push_ptr;

// Receive filters and masks
// Receive buffer 0, can choose two different receive blocks, with a single mask
#define RX_MASK_0		0x07E0			// Only care about upper 6 bits of 11-bit address
#define RX_ID_0A		0x0888		// Receive driver controls (ourself) packets, for RTR and bootloader trigger
#define RX_ID_0B		0x0000			// Unused
// Receive buffer 1, can choose four different receive blocks, with a single mask
#define RX_MASK_1		0x07E0			// Only care about upper 6 bits of 11-bit address
#define RX_ID_1A		MC_CAN_BASE		// Receive packets from motor controller
#define RX_ID_1B		EG_CAN_BASE		// Receive packets from eGear (series/parallel switch) controller
#define RX_ID_1C		0x0000			// Unused
#define RX_ID_1D		0x0000			// Unused

// SPI port interface macros

#define can_select    (PORTB &= ~(1<<PORTB2)) // GUY
#define can_deselect  (PORTB |= (1<<PORTB2)) //


// CAN Bitrates
#define CAN_BITRATE_50			0
#define CAN_BITRATE_100			1
#define CAN_BITRATE_125			2
#define CAN_BITRATE_250			3
#define CAN_BITRATE_500			4
#define CAN_BITRATE_1000		5
#define CAN_BITRATE_10			6

// Motor controller CAN base address and packet offsets
#define	MC_CAN_BASE		0x420		// High = Serial Number             Low = "TRIa" string
#define MC_LIMITS		0x01		// High = Active Motor/CAN counts   Low = Error & Limit flags
#define	MC_BUS			0x02		// High = Bus Current               Low = Bus Voltage
#define MC_VELOCITY		0x03		// High = Velocity (m/s)            Low = Velocity (rpm)
#define MC_PHASE		0x04		// High = Phase A Current           Low = Phase B Current
#define MC_V_VECTOR		0x05		// High = Vd vector                 Low = Vq vector
#define MC_I_VECTOR		0x06		// High = Id vector                 Low = Iq vector
#define MC_BEMF_VECTOR	0x07		// High = BEMFd vector              Low = BEMFq vector
#define MC_RAIL1		0x08		// High = 15V                       Low = Unused
#define MC_RAIL2		0x09		// High = 3.3V                      Low = 1.9V
#define MC_FAN			0x0A		// High = Reserved                  Low = Reserved
#define MC_TEMP1		0x0B		// High = Heatsink Phase C Temp     Low = Motor Temp
#define MC_TEMP2		0x0C		// High = Heatsink Phase B Temp     Low = CPU Temp
#define MC_TEMP3		0x0D		// High = Heatsink Phase A Temp     Low = Unused
#define MC_CUMULATIVE	0x0E		// High = DC Bus AmpHours           Low = Odometer

// Driver controls CAN base address and packet offsets
#define DC_CAN_BASE		0x500
#define DC_DRIVE		0x001
#define DC_POWER		0x002
#define DC_RESET		0x003
#define DC_SWITCH		0x005
#define DC_INFO			0x006		//LEGACY SUPPORT ONLY
#define DC_BOOTLOAD		0x016
#define DC_SETUP		0x01F

// Low / high egear switch CAN base address and packet offsets
#define EG_CAN_BASE		0x580
#define EG_COMMAND		0x01
#define EG_STATUS		0x02

#define EG_CMD_NEUTRAL	0x00
#define EG_CMD_LOW		0x01
#define EG_CMD_HIGH		0x02

#define EG_STATE_NEUTRAL	1
#define EG_STATE_SWITCHING	2
#define EG_STATE_LOW		3
#define EG_STATE_HIGH		4

// Status values (for message reception)
#define CAN_ERROR		0xFFFF
#define CAN_MERROR		0xFFFE
#define CAN_WAKE		0xFFFD
#define CAN_RTR			0xFFFC
#define CAN_OK			0x0001

// MCP2515 command bytes
#define MCP_RESET		0xC0
#define MCP_READ		0x03
#define MCP_READ_RX		0x90		// When used, needs to have RX_BUFFER address inserted into lower bits
#define MCP_WRITE		0x02
#define MCP_WRITE_TX	0x40		// When used, needs to have TX_BUFFER address inserted into lower bits
#define MCP_RTS			0x80		// When used, needs to have buffer to transmit inserted into lower bits
#define MCP_STATUS		0xA0
#define MCP_FILTER		0xB0
#define MCP_MODIFY		0x05

// MCP2515 register names
#define RXF0SIDH		0x00
#define RXF0SIDL		0x01
#define RXF0EID8		0x02
#define RXF0EID0		0x03
#define RXF1SIDH		0x04
#define RXF1SIDL		0x05
#define RXF1EID8		0x06
#define RXF1EID0		0x07
#define RXF2SIDH		0x08
#define RXF2SIDL		0x09
#define RXF2EID8		0x0A
#define RXF2EID0		0x0B
#define BFPCTRL			0x0C
#define TXRTSCTRL		0x0D
#define CANSTAT			0x0E
#define CANCTRL			0x0F

#define RXF3SIDH		0x10
#define RXF3SIDL		0x11
#define RXF3EID8		0x12
#define RXF3EID0		0x13
#define RXF4SIDH		0x14
#define RXF4SIDL		0x15
#define RXF4EID8		0x16
#define RXF4EID0		0x17
#define RXF5SIDH		0x18
#define RXF5SIDL		0x19
#define RXF5EID8		0x1A
#define RXF5EID0		0x1B
#define TEC				0x1C
#define REC				0x1D

#define RXM0SIDH		0x20
#define RXM0SIDL		0x21
#define RXM0EID8		0x22
#define RXM0EID0		0x23
#define RXM1SIDH		0x24
#define RXM1SIDL		0x25
#define RXM1EID8		0x26
#define RXM1EID0		0x27
#define CNF3			0x28
#define CNF2			0x29
#define CNF1			0x2A
#define CANINTE			0x2B
#define CANINTF			0x2C
#define EFLAG			0x2D

#define TXB0CTRL		0x30
#define TXB0SIDH		0x31
#define TXB0SIDL		0x32
#define TXB0EID8		0x33
#define TXB0EID0		0x34
#define TXB0DLC			0x35
#define TXB0D0			0x36
#define TXB0D1			0x37
#define TXB0D2			0x38
#define TXB0D3			0x39
#define TXB0D4			0x3A
#define TXB0D5			0x3B
#define TXB0D6			0x3C
#define TXB0D7			0x3D

#define TXB1CTRL		0x40
#define TXB1SIDH		0x41
#define TXB1SIDL		0x42
#define TXB1EID8		0x43
#define TXB1EID0		0x44
#define TXB1DLC			0x45
#define TXB1D0			0x46
#define TXB1D1			0x47
#define TXB1D2			0x48
#define TXB1D3			0x49
#define TXB1D4			0x4A
#define TXB1D5			0x4B
#define TXB1D6			0x4C
#define TXB1D7			0x4D

#define TXB2CTRL		0x50
#define TXB2SIDH		0x51
#define TXB2SIDL		0x52
#define TXB2EID8		0x53
#define TXB2EID0		0x54
#define TXB2DLC			0x55
#define TXB2D0			0x56
#define TXB2D1			0x57
#define TXB2D2			0x58
#define TXB2D3			0x59
#define TXB2D4			0x5A
#define TXB2D5			0x5B
#define TXB2D6			0x5C
#define TXB2D7			0x5D

#define RXB0CTRL		0x60
#define RXB0SIDH		0x61
#define RXB0SIDL		0x62
#define RXB0EID8		0x63
#define RXB0EID0		0x64
#define RXB0DLC			0x65
#define RXB0D0			0x66
#define RXB0D1			0x67
#define RXB0D2			0x68
#define RXB0D3			0x69
#define RXB0D4			0x6A
#define RXB0D5			0x6B
#define RXB0D6			0x6C
#define RXB0D7			0x6D

#define RXB1CTRL		0x70
#define RXB1SIDH		0x71
#define RXB1SIDL		0x72
#define RXB1EID8		0x73
#define RXB1EID0		0x74
#define RXB1DLC			0x75
#define RXB1D0			0x76
#define RXB1D1			0x77
#define RXB1D2			0x78
#define RXB1D3			0x79
#define RXB1D4			0x7A
#define RXB1D5			0x7B
#define RXB1D6			0x7C
#define RXB1D7			0x7D

// MCP2515 RX ctrl bit definitions
#define MCP_RXB0_RTR	0x08
#define MCP_RXB1_RTR	0x08

// MCP2515 Interrupt flag register bit definitions
#define MCP_IRQ_MERR	0x80
#define MCP_IRQ_WAKE	0x40
#define MCP_IRQ_ERR		0x20
#define MCP_IRQ_TXB2	0x10
#define MCP_IRQ_TXB1	0x08
#define MCP_IRQ_TXB0	0x04
#define MCP_IRQ_RXB1	0x02
#define MCP_IRQ_RXB0	0x01

//Guy adding
#define RXM 0x05

#endif	// CAN_H
