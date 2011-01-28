
////////////////////////////////////////////////////////////////////
//
// 	Alexandr Filenkov 2010
//
////////////////////////////////////////////////////////////////////


#include <p30fxxxx.h>
#include "osa.h"
#include "SPI.h"
//#include "Uart.h"

extern void delay_us(unsigned long aTimeInMicrosec);


static unsigned char SPI1_ChipSelectInitialized = 0;
static unsigned char SPI2_ChipSelectInitialized = 0;
//-----------------------------------------------------------------------------------------
// Initalizes SPI1, no slave select, no interrupts, 8 bit mode
//-----------------------------------------------------------------------------------------
void initSPI1(unsigned char b16Bit, unsigned char CKE, unsigned char CKP)
{
	SPI1STATbits.SPIEN = 0;	// disable SPI
	_TRISF7 = 1;		// SDI1/RF7 is input
	_TRISF6 = 0;		// SCK1/RF6 is clock output
	_TRISF8 = 0;		// SDO1/RF8	is output

	SPI1CONbits.MODE16 = b16Bit;// enable/disable 16bit mode

	SPI1STATbits.SPIROV = 0;	// clear the overflow buffer flag

	SPI1STATbits.SPITBF = 0;	// clear the flag SPI transmit buffer full status bit (SPITBF=0 transmit started, SPIxTBF empty,
					// SPITBF=1 transmit not yet started, SPIxTBF is full)

	SPI1STATbits.SPIRBF = 0;	// clear the flag SPI receive buffer full status bit (SPIRBF=0 receive is not complete
					// SPIxRXB is empty, SPIRBF=1 receive complete SPIxRXB is full)


	SPI1CONbits.MSTEN = 1;	// Master mode enable bit (MSTEN=0 slave mode, MSTEN=1 master mode)

	SPI1CONbits.FRMEN = 0;	// disable Framed SPI support bit

	SPI1CONbits.DISSDO = 0;	// enable output functionality


	SPI1CONbits.SMP = 0;	// SPI data input sample phase bit (Master mode: SMP=0 input data sampled at middle of data output time,
							// SMP=1 input data sampled at end of data output time; Slave mode: SMP must be cleared)

	SPI1CONbits.CKE = CKE;	// SPI clock edge select bit (CKE=0 serial output data changes on transition from IDLE clock state to
							// active clock state, CKE=1 serial output data changes on transition from active clock state to IDLE clock state)

	SPI1CONbits.SSEN = 0;	// Slave select enable bit (SSEN=0 SS1 pin not used by module, SSEN=1 SS1 pin used for slave mode)

	SPI1CONbits.CKP = CKP;	// Clock polarity select bit (CKP=0 IDLE state for clock is a low level, active state is a high level,
							// CKP=1 IDLE state for clock is a high level, active state is a low level)

	SPI1CONbits.PPRE = 0x01;	// Primary prescale (master mode) bits 64:1
								// 00 - 0 – primary prescale 64:1
								// 01 - 1	primary prescale 16:1
								// 10 - 2 – primary prescale 4:1
								// 11 - 3– primary prescale 1:1


	SPI1CONbits.SPRE = 0x07;// Secondary prescale (master mode) bits 5:1
							// 000 – secondary prescale 8:1
							// 001 – secondary prescale 7:1
							// ...
							// 110 – secondary prescale 2:1
							// 111 – secondary prescale 1:1


	_SPI1IE = 0;		// disable SPI interrupt
	_SPI1IF = 0;		// clear SPI interrupt flag

	SPI1STATbits.SPIEN = 1;	// enable SPI module

	if (!SPI1_ChipSelectInitialized) {

		SPI1_ChipSelectInitialized = 1;

		_LATB7  = 1;	// Chip deselect - Chip 1
		_LATB8  = 1;	// Chip deselect - Chip 2
		_LATB9  = 1;	// Chip deselect - Chip 3
		_LATB10 = 1;	// Chip deselect - Chip 4

		_TRISB7  = 0;	// Chip select - Chip 1
		_TRISB8  = 0;	// Chip select - Chip 2
		_TRISB9  = 0;	// Chip select - Chip 3
		_TRISB10 = 0;	// Chip select - Chip 4

		_LATB7  = 1;	// Chip deselect - Chip 1
		_LATB8  = 1;	// Chip deselect - Chip 2
		_LATB9  = 1;	// Chip deselect - Chip 3
		_LATB10 = 1;	// Chip deselect - Chip 4

	}
}

//-----------------------------------------------------------------------------------------
// Writes data to SPI and then waits until the receiving is completed
//-----------------------------------------------------------------------------------------
void SPI1_Set16BitMode(unsigned char is16BitMode)
{
	// wait until the transmission is done
	while (SPI1STATbits.SPITBF) {
		ClrWdt();
	}
	if ((SPI1CONbits.MODE16 && is16BitMode) || (!SPI1CONbits.MODE16 && !is16BitMode)) {
		return; //mode unchanged
	}

	SPI1STATbits.SPIEN = 0;	// disable SPI
	SPI1CONbits.MODE16 = (is16BitMode != 0); // enable/disable 16bit mode
	SPI1STATbits.SPIEN = 1;	// enable SPI module
}
//-----------------------------------------------------------------------------------------
// Do Chip Select and then write data to SPI and then waits until the receiving is completed
//-----------------------------------------------------------------------------------------
unsigned short SPI1_SelectWriteRead(unsigned char aChip, unsigned short aData)
{
	// select the chip
	SPI1_ChipSelect_Single(aChip, 1);

	// wait some delay
	delay_us(5);

	unsigned short retVal = SPI1_WriteRead(aData);
	delay_us(5);

	// deselect the chip
	SPI1_ChipSelect_Single(aChip, 0);

	return retVal;
}

//-----------------------------------------------------------------------------------------
// Do Chip Select and then write data to SPI and then waits until the receiving is completed
//-----------------------------------------------------------------------------------------
unsigned short SPI1_NegSelectWriteRead(unsigned char aChip, unsigned short aData)
{
	// select the chip
	SPI1_ChipSelect_Single(aChip, 0);

	// wait some delay
	delay_us(5);

	unsigned short retVal = SPI1_WriteRead(aData);

	// wait some delay
	delay_us(5);

	// deselect the chip
	SPI1_ChipSelect_Single(aChip, 1);

	return retVal;
}

//-----------------------------------------------------------------------------------------
// Writes data to SPI and then waits until the receiving is completed
//-----------------------------------------------------------------------------------------
unsigned short SPI1_WriteRead(unsigned short aData)
{
	// wait until the transmission is done
	while (SPI1STATbits.SPITBF) {
		ClrWdt();
	}

	if (SPI1CONbits.MODE16)          /* word write */
		SPI1BUF = aData;
	else
		SPI1BUF = aData & 0xff;   /*  byte write  */

	while (!SPI1STATbits.SPIRBF) {
		ClrWdt();
	}
	SPI1STATbits.SPITBF = 0;	// clear the flag SPI transmit buffer full status bit (SPITBF=0 transmit started, SPIxTBF empty,
	SPI1STATbits.SPIRBF = 0;	// clear the flag SPI receive buffer full status bit (SPIRBF=0 receive is not complete

	SPI1STATbits.SPIROV = 0;

	// read the value
	if (SPI1CONbits.MODE16)
		return (SPI1BUF);           /* return word read */

	return (SPI1BUF & 0xff);    /* return byte read */
}


//-----------------------------------------------------------------------------------------
// Selects chip for SPI communication based on bit mask provided
//-----------------------------------------------------------------------------------------
void SPI1_ChipSelect(unsigned char aChip)
{
	_LATB7  = (aChip & SPI_Chip_Select_1) != 0 ? 0 : 1;	// Chip select - Chip 1
	_LATB8  = (aChip & SPI_Chip_Select_2) != 0 ? 0 : 1;	// Chip select - Chip 2
	_LATB9  = (aChip & SPI_Chip_Select_3) != 0 ? 0 : 1;	// Chip select - Chip 3
	_LATB10 = (aChip & SPI_Chip_Select_4) != 0 ? 0 : 1;	// Chip select - Chip 4
}

//-----------------------------------------------------------------------------------------
// Selects/deselects one chip for SPI communication
//-----------------------------------------------------------------------------------------
void SPI1_ChipSelect_Single(unsigned char aChip, unsigned char bSelected)
{
	switch (aChip) {
		case SPI_Chip_Select_1: _LATB7  = bSelected ; break; // Chip select - Chip 1
		case SPI_Chip_Select_2: _LATB8  = bSelected ; break; // Chip select - Chip 2
		case SPI_Chip_Select_3: _LATB9  = bSelected ; break; // Chip select - Chip 3
		case SPI_Chip_Select_4: _LATB10  = bSelected; break; // Chip select - Chip 4
	}
}



//-----------------------------------------------------------------------------------------
// Initalizes SPI2
//-----------------------------------------------------------------------------------------
void initSPI2(unsigned char b16Bit, unsigned char CKE, unsigned char CKP)
{

	SPI2STATbits.SPIEN = 0;	// disable SPI
	_TRISG7 = 1;			// SDI2/RG7 is input RX
	_TRISG6 = 0;			// SCK2/RG6 is clock output
	_TRISG8 = 0;			// SDO2/RG8	is output TX


	SPI2STATbits.SPIROV = 0;	// clear the overflow buffer flag

	SPI2STATbits.SPITBF = 0;	// clear the flag SPI transmit buffer full status bit (SPITBF=0 transmit started, SPIxTBF empty,
								// SPITBF=1 transmit not yet started, SPIxTBF is full)

	SPI2STATbits.SPIRBF = 0;	// clear the flag SPI receive buffer full status bit (SPIRBF=0 receive is not complete
								// SPIxRXB is empty, SPIRBF=1 receive complete SPIxRXB is full)


	SPI2CONbits.MSTEN = 1;	// Master mode enable bit (MSTEN=0 slave mode, MSTEN=1 master mode)

	SPI2CONbits.FRMEN = 0;	// disable Framed SPI support bit

	SPI2CONbits.DISSDO = 0;// enable output functionality

	SPI2CONbits.MODE16 = b16Bit;// enable/disable 16bit mode

	SPI2CONbits.SMP = 0;	// SPI data input sample phase bit (Master mode: SMP=0 input data sampled at middle of data output time,
							// SMP=1 input data sampled at end of data output time; Slave mode: SMP must be cleared)

	SPI2CONbits.CKE = CKE;	// SPI clock edge select bit (CKE=0 serial output data changes on transition from IDLE clock state to
							// active clock state, CKE=1 serial output data changes on transition from active clock state to IDLE clock state)

	SPI2CONbits.SSEN = 0;	// Slave select enable bit (SSEN=0 SS2 pin not used by module, SSEN=1 SS2 pin used for slave mode)

	SPI2CONbits.CKP = CKP;	// Clock polarity select bit (CKP=0 IDLE state for clock is a low level, active state is a high level,
							// CKP=1 IDLE state for clock is a high level, active state is a low level)

	SPI2CONbits.PPRE = 0x01;	// Primary prescale (master mode) bits 64:1
								// 00 - 0 – primary prescale 64:1
								// 01 - 1	primary prescale 16:1
								// 10 - 2 – primary prescale 4:1
								// 11 - 3 – primary prescale 1:1


	SPI2CONbits.SPRE = 0x07;	// Secondary prescale (master mode) bits 5:1
								// 000 – secondary prescale 8:1
								// 001 – secondary prescale 7:1
								// ...
								// 110 – secondary prescale 2:1
								// 111 – secondary prescale 1:1


	_SPI2IE = 0;		// disable SPI interrupt
	_SPI2IF = 0;		// clear SPI interrupt flag

	SPI2STATbits.SPIEN = 1;	// enable SPI module

	if (!SPI2_ChipSelectInitialized) {
		SPI2_ChipSelectInitialized = 1;

		_LATE8  = 1;	// Chip select - Chip 1
		_LATE9  = 1;	// Chip select - Chip 2
		_LATG9  = 1;	// Chip select - Chip 3
		_LATC3  = 1;	// Chip select - Chip 4
		_LATC1  = 1;	// Chip select - Chip 5

		_TRISE8  = 0;	// Chip select - Chip 1
		_TRISE9  = 0;	// Chip select - Chip 2
		_TRISG9  = 0;	// Chip select - Chip 3
		_TRISC3  = 0;	// Chip select - Chip 4
		_TRISC1  = 0;	// Chip select - Chip 5

		_LATE8  = 1;	// Chip select - Chip 1
		_LATE9  = 1;	// Chip select - Chip 2
		_LATG9  = 1;	// Chip select - Chip 3
		_LATC3  = 1;	// Chip select - Chip 4
		_LATC1  = 1;	// Chip select - Chip 5
	}
}

//-----------------------------------------------------------------------------------------
// Do Chip Select and then write data to SPI and then waits until the receiving is completed
//-----------------------------------------------------------------------------------------
unsigned short SPI2_SelectWriteRead(unsigned char aChip, unsigned short aData)
{
	// select the chip
	SPI2_ChipSelect_Single(aChip, 1);

	// wait some delay
	delay_us(5);

	unsigned short retVal = SPI2_WriteRead(aData);
	delay_us(5);

	// deselect the chip
	SPI2_ChipSelect_Single(aChip, 0);

	return retVal;
}

//-----------------------------------------------------------------------------------------
// Do Chip Select and then write data to SPI and then waits until the receiving is completed
//-----------------------------------------------------------------------------------------
unsigned short SPI2_NegSelectWriteRead(unsigned char aChip, unsigned short aData)
{
	// select the chip
	SPI2_ChipSelect_Single(aChip, 0);

	// wait some delay
	delay_us(5);

	unsigned short retVal = SPI2_WriteRead(aData);
	delay_us(5);

	// deselect the chip
	SPI2_ChipSelect_Single(aChip, 1);

	return retVal;
}


//-----------------------------------------------------------------------------------------
// Writes data to SPI and then waits until the receiving is completed
//-----------------------------------------------------------------------------------------
unsigned short SPI2_WriteRead(unsigned short aData)
{
	// wait until the transmission is done
	while (SPI2STATbits.SPITBF) {
		ClrWdt();
	}

	if (SPI2CONbits.MODE16)          /* word write */
		SPI2BUF = aData;
	else
		SPI2BUF = aData & 0xff;   /*  byte write  */

	while (!SPI2STATbits.SPIRBF) {
		ClrWdt();
	}
	SPI2STATbits.SPITBF = 0;	// clear the flag SPI transmit buffer full status bit (SPITBF=0 transmit started, SPIxTBF empty,
	SPI2STATbits.SPIRBF = 0;	// clear the flag SPI receive buffer full status bit (SPIRBF=0 receive is not complete

	SPI2STATbits.SPIROV = 0;

	// return the value
	if (SPI2CONbits.MODE16)
		return (SPI2BUF);           /* return word read */

	return (SPI2BUF & 0xff);    /* return byte read */
}


//-----------------------------------------------------------------------------------------
// Selects chip for SPI communication based on bit mask provided
//-----------------------------------------------------------------------------------------
void SPI2_ChipSelect(unsigned char aChip)
{

	_LATE8  = (aChip & SPI_Chip_Select_1) != 0 ? 1 : 0;	// Chip select - Chip 1
	_LATE9  = (aChip & SPI_Chip_Select_2) != 0 ? 1 : 0;	// Chip select - Chip 2
	_LATG9  = (aChip & SPI_Chip_Select_3) != 0 ? 1 : 0;	// Chip select - Chip 3
	_LATC3  = (aChip & SPI_Chip_Select_4) != 0 ? 1 : 0;	// Chip select - Chip 4
	_LATC1  = (aChip & SPI_Chip_Select_5) != 0 ? 1 : 0;	// Chip select - Chip 5
}

//-----------------------------------------------------------------------------------------
// Selects/deselects one chip for SPI communication
//-----------------------------------------------------------------------------------------
void SPI2_ChipSelect_Single(unsigned char aChip, unsigned char bSelected)
{
	switch (aChip) {
		case SPI_Chip_Select_1: _LATE8  = bSelected; break; // Chip select - Chip 1
		case SPI_Chip_Select_2: _LATE9  = bSelected; break; // Chip select - Chip 2
		case SPI_Chip_Select_3: _LATG9  = bSelected; break; // Chip select - Chip 3
		case SPI_Chip_Select_4: _LATC3  = bSelected; break; // Chip select - Chip 4
		case SPI_Chip_Select_5: _LATC1  = bSelected; break; // Chip select - Chip 5
	}
}

