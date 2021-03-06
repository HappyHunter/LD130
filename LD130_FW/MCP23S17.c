////////////////////////////////////////////////////////////////////
//
// 	Alexandr Filenkov 2010
//
////////////////////////////////////////////////////////////////////


#include <p30fxxxx.h>
#include "osa.h"
#include "MCP23S17.h"
#include "SPI.h"
#include "common.h"

extern void delay_us(unsigned long aTimeInMicrosec);

// addresses of ports on the chip
//
// When a bit is set, the corresponding pin becomes an
// input. When a bit is clear, the corresponding pin
// becomes an output
#define IODIRA 0x00
#define IODIRB 0x01

#define IOPOLA 0x02
#define IOPOLB 0x03

#define IOCON 0x0A

#define GPIOA 0x12
#define GPIOB 0x13

#define OLATA 0x14
#define OLATB 0x15


//-----------------------------------------------------------------------------------------
void MCP23S17Init1()
{
//	OS_Bsem_Wait(SPI_2_Busy_Sema);
	initSPI2(0/*8 bit mode*/, 1/*CKE*/, 0/*CKP*/);


	SPI2_ChipSelect_Single(SPI_Chip_Select_1, 1);
	delay_us(10); //






	SPI2_ChipSelect_Single(SPI_Chip_Select_1, 0); // enable I/O expander
	delay_us(10); //
	SPI2_WriteRead(0x40); // Opcode write mode, address 0x00
	SPI2_WriteRead(IOCON);

//	bit 7 BANK: Controls how the registers are addressed
//				1 = The registers associated with each port are separated into different banks
//				0 = The registers are in the same bank (addresses are sequential)
//	bit 6 MIRROR: INT Pins Mirror bit
//				1 = The INT pins are internally connected
//				0 = The INT pins are not connected. INTA is associated with PortA and INTB is associated with PortB
//	bit 5 SEQOP: Sequential Operation mode bit.
//				1 = Sequential operation disabled, address pointer does not increment.
//				0 = Sequential operation enabled, address pointer increments.
//	bit 4 DISSLW: Slew Rate control bit for SDA output.
//				1 = Slew rate disabled.
//				0 = Slew rate enabled.
//	bit 3 HAEN: Hardware Address Enable bit (MCP23S17 only).
//				Address pins are always enabled on MCP23017.
//				1 = Enables the MCP23S17 address pins.
//				0 = Disables the MCP23S17 address pins.
//	bit 2 ODR: This bit configures the INT pin as an open-drain output.
//			1 = Open-drain output (overrides the INTPOL bit).
//			0 = Active driver output (INTPOL bit sets the polarity).
//	bit 1 INTPOL: This bit sets the polarity of the INT output pin.
//		1 = Active-high.
//		0 = Active-low.
//	bit 0 Unimplemented: Read as �0�.
	SPI2_WriteRead(0x20); // same bank,SEQOP disabled
	SPI2_ChipSelect_Single(SPI_Chip_Select_1, 1); // disable I/O
	delay_us(10); //








	SPI2_ChipSelect_Single(SPI_Chip_Select_1, 0); // enable I/O expander
	delay_us(10); //
	SPI2_WriteRead(0x40); // Opcode write mode, address 0x00
//	Controls the direction of the data I/O.
//	When a bit is set, the corresponding pin becomes an
//	input. When a bit is clear, the corresponding pin
//	becomes an output.
	SPI2_WriteRead(IODIRA); // configure the IO ports direction
	SPI2_WriteRead(0xF0);	// GPA0-GPA3 outputs, GPA4-GPA7 Inputs
	SPI2_WriteRead(0xFE);	// Port B GPB0 is output
	SPI2_ChipSelect_Single(SPI_Chip_Select_1, 1); // disable I/O
	delay_us(10); //








	SPI2_ChipSelect_Single(SPI_Chip_Select_1, 0); // enable I/O expander
	delay_us(10); //
	SPI2_WriteRead(0x40); 	// Opcode write mode, address 0x00
	SPI2_WriteRead(IOPOLA); // set the  polarity

	//	This register allows the user to configure the polarity on
//	the corresponding GPIO port bits.
//	If a bit is set, the corresponding GPIO register bit will
//	reflect the inverted value on the pin.
	SPI2_WriteRead(0x00); // write to port
	SPI2_WriteRead(0x00); // write to port
	SPI2_ChipSelect_Single(SPI_Chip_Select_1, 1); // disable I/O
//	OS_Bsem_Set(SPI_2_Busy_Sema);
}

// Function : MCP23S17Init()
// Notes: Setup CS and RESET pins
// Reset the MCP23S17
// Setup SPI module on PIC
//-----------------------------------------------------------------------------------------
void MCP23S17Init2()
{
//	OS_Bsem_Wait(SPI_2_Busy_Sema);
	initSPI2(0/*8 bit mode*/, 1/*CKE*/, 0/*CKP*/);

	SPI2_ChipSelect_Single(SPI_Chip_Select_2, 1);
	delay_us(10); //






	SPI2_ChipSelect_Single(SPI_Chip_Select_2, 0); // enable I/O expander
	delay_us(10); //

	SPI2_WriteRead(0x40); // Opcode write mode, address 0x00
	SPI2_WriteRead(IOCON);

//	bit 7 BANK: Controls how the registers are addressed
//				1 = The registers associated with each port are separated into different banks
//				0 = The registers are in the same bank (addresses are sequential)
//	bit 6 MIRROR: INT Pins Mirror bit
//				1 = The INT pins are internally connected
//				0 = The INT pins are not connected. INTA is associated with PortA and INTB is associated with PortB
//	bit 5 SEQOP: Sequential Operation mode bit.
//				1 = Sequential operation disabled, address pointer does not increment.
//				0 = Sequential operation enabled, address pointer increments.
//	bit 4 DISSLW: Slew Rate control bit for SDA output.
//				1 = Slew rate disabled.
//				0 = Slew rate enabled.
//	bit 3 HAEN: Hardware Address Enable bit (MCP23S17 only).
//				Address pins are always enabled on MCP23017.
//				1 = Enables the MCP23S17 address pins.
//				0 = Disables the MCP23S17 address pins.
//	bit 2 ODR: This bit configures the INT pin as an open-drain output.
//			1 = Open-drain output (overrides the INTPOL bit).
//			0 = Active driver output (INTPOL bit sets the polarity).
//	bit 1 INTPOL: This bit sets the polarity of the INT output pin.
//		1 = Active-high.
//		0 = Active-low.
//	bit 0 Unimplemented: Read as �0�.
	SPI2_WriteRead(0x20); // same bank,SEQOP disabled
	SPI2_ChipSelect_Single(SPI_Chip_Select_2, 1); // disable I/O
	delay_us(10); //






	SPI2_ChipSelect_Single(SPI_Chip_Select_2, 0); // enable I/O expander
	delay_us(10); //
	SPI2_WriteRead(0x40); // Opcode write mode, address 0x00
//	Controls the direction of the data I/O.
//	When a bit is set, the corresponding pin becomes an
//	input. When a bit is clear, the corresponding pin
//	becomes an output.
	SPI2_WriteRead(IODIRA); // configure the IO ports direction
	SPI2_WriteRead(0xF0);	// GPA0-GPA3 outputs, GPA4-GPA7 Inputs
	SPI2_WriteRead(0xFE);	// Port B GPB0 is output
	SPI2_ChipSelect_Single(SPI_Chip_Select_2, 1); // disable I/O
	delay_us(10); //







	SPI2_ChipSelect_Single(SPI_Chip_Select_2, 0); // enable I/O expander
	delay_us(10); //
	SPI2_WriteRead(0x40); // Opcode write mode, address 0x00
	SPI2_WriteRead(IOPOLA); // set port A as outputs
//	This register allows the user to configure the polarity on
//	the corresponding GPIO port bits.
//	If a bit is set, the corresponding GPIO register bit will
//	reflect the inverted value on the pin.
	SPI2_WriteRead(0x00); // write to port
	SPI2_WriteRead(0x00); // write to port
	SPI2_ChipSelect_Single(SPI_Chip_Select_2, 1); // disable I/O
//	OS_Bsem_Set(SPI_2_Busy_Sema);
}



//-----------------------------------------------------------------------------------------
unsigned short MCP23S17ReadHead1()
{
	unsigned short retValA = 0;
	unsigned short retValB = 0;

//	OS_Bsem_Wait(SPI_2_Busy_Sema);
	initSPI2(0, 0, 0);

	SPI2_ChipSelect_Single(SPI_Chip_Select_1, 0); // enable I/O expander
	delay_us(5);

	SPI2_WriteRead(0x41); // Opcode read mode, address 0x00
	SPI2_WriteRead(GPIOA); // set port A
	retValA = SPI2_WriteRead(0x00); // read port A
	retValB = SPI2_WriteRead(0x00); // read port B

	SPI2_ChipSelect_Single(SPI_Chip_Select_1, 1); // disable I/O
//	OS_Bsem_Set(SPI_2_Busy_Sema);
	return ((retValB << 8) | retValA);
}


//-----------------------------------------------------------------------------------------
void MCP23S17WriteHead1(unsigned short data)
{
	static unsigned short theData = 0;

	theData = data;	// save the local state, since OS_Bsem_Wait can switch the context

//	OS_Bsem_Wait(SPI_2_Busy_Sema);
	initSPI2(0, 0, 0);
	SPI2_ChipSelect_Single(SPI_Chip_Select_1, 0); // enable I/O expander
	delay_us(5);

	SPI2_WriteRead(0x40); // Opcode write mode, address 0x00
	SPI2_WriteRead(OLATA); // set port write A
	SPI2_WriteRead(data & 0x00FF); // write port A
	SPI2_WriteRead((data>>8) && 0x00FF); // write port B

	SPI2_ChipSelect_Single(SPI_Chip_Select_1, 1); // disable I/O
//	OS_Bsem_Set(SPI_2_Busy_Sema);
}


//-----------------------------------------------------------------------------------------
unsigned short MCP23S17ReadHead2()
{
	unsigned short retValA = 0;
	unsigned short retValB = 0;

//	OS_Bsem_Wait(SPI_2_Busy_Sema);
	initSPI2(0, 0, 0);

	SPI2_ChipSelect_Single(SPI_Chip_Select_2, 0); // enable I/O expander
	delay_us(5);

	SPI2_WriteRead(0x41); // Opcode read mode, address 0x00
	SPI2_WriteRead(GPIOA); // set port A
	retValA = SPI2_WriteRead(0x00); // read port A
	retValB = SPI2_WriteRead(0x00); // read port B

	SPI2_ChipSelect_Single(SPI_Chip_Select_2, 1); // disable I/O

//	OS_Bsem_Set(SPI_2_Busy_Sema);
	return ((retValB << 8) | retValA);
}

//-----------------------------------------------------------------------------------------
void MCP23S17WriteHead2(unsigned short data)
{
	static unsigned short theData = 0;

	theData = data;	// save the local state, since OS_Bsem_Wait can switch the context

//	OS_Bsem_Wait(SPI_2_Busy_Sema);
	initSPI2(0, 0, 0);
	SPI2_ChipSelect_Single(SPI_Chip_Select_2, 0); // enable I/O expander
	delay_us(5);

	SPI2_WriteRead(0x40); // Opcode write mode, address 0x00
	SPI2_WriteRead(OLATA); // set port write A
	SPI2_WriteRead(data & 0x00FF); // write port A
	SPI2_WriteRead((data>>8) && 0x00FF); // write port B

	SPI2_ChipSelect_Single(SPI_Chip_Select_2, 1); // disable I/O
//	OS_Bsem_Set(SPI_2_Busy_Sema);
}

/**
 * Enable or disable the VOUT for each chanel separately. We
 * need to do that to prevent small current leakage even if we
 * set the current to zero (0) .
 *
 * Inputs:
 *
 *  unsigned char aHead - the Head ID 1 or 2
 *
 *  unsigned long aChanel1,aChanel2,aChanel3,aChanel4 - the
 *  current value in percents 0 - 100 00% with fixed decimal
 *  point at 2 digits
*/
//-----------------------------------------------------------------------------------------
void MCP23S17EnableVout(unsigned char aHead, unsigned long aChanel1, unsigned long aChanel2, unsigned long aChanel3, unsigned long aChanel4)
{
	unsigned short theValue = 0;
	if (aHead == 1) {
		if (aChanel1 > 0) theValue |= ioH1C1VOutEnabled;
		if (aChanel2 > 0) theValue |= ioH1C2VOutEnabled;
		if (aChanel3 > 0) theValue |= ioH1C3VOutEnabled;
		if (aChanel4 > 0) theValue |= ioH1C4VOutEnabled;
		MCP23S17WriteHead1(theValue);
	} else {
		if (aChanel1 > 0) theValue |= ioH2C1VOutEnabled;
		if (aChanel2 > 0) theValue |= ioH2C2VOutEnabled;
		if (aChanel3 > 0) theValue |= ioH2C3VOutEnabled;
		if (aChanel4 > 0) theValue |= ioH2C4VOutEnabled;
		MCP23S17WriteHead2(theValue);
	}
}

