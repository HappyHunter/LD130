////////////////////////////////////////////////////////////////////
//
// 	Alexandr Filenkov 2011
//
////////////////////////////////////////////////////////////////////

#ifndef MCP23S17_201101071319
#define MCP23S17_201101071319



/**
 * GPIO 1
 *
 * GPA0 - H1C1 - enable VOUT - Output
 * GPA1 - H1C2 - enable VOUT - Output
 * GPA2 - H1C3 - enable VOUT - Output
 * GPA3 - H1C4 - enable VOUT - Output
 *
 * GPA4 - Fuse H1C4 - Input
 * GPA5 - Fuse H1C3 - Input
 * GPA6 - Fuse H1C3 - Input
 * GPA7 - Fuse H1C1 - Input
 *
 * GPB0 - Unused
 * GPB1 - UV_H1_C1 - Input
 * GPB2 - UV_H1_C2 - Input
 * GPB3 - UV_H1_C4 - Input
 * GPB4 - UV_H1_C4 - Input
 *
 */

typedef enum tag_TGPIOH1
{
	ioH1C1VOutEnabled 	=		0x0001,
	ioH1C2VOutEnabled 	=		0x0002,
	ioH1C3VOutEnabled 	=		0x0004,
	ioH1C4VOutEnabled 	=		0x0008,

	ioH1C1FuseOK 		=		0x0010,
	ioH1C2FuseOK 		=		0x0020,
	ioH1C3FuseOK 		=		0x0040,
	ioH1C4FuseOK 		=		0x0080,

	ioH1Unused1			=		0x0100,
	ioH1C1UnderVoltage 	=		0x0200,
	ioH1C2UnderVoltage 	=		0x0400,
	ioH1C3UnderVoltage 	=		0x0800,

	ioH1C4UnderVoltage 	=		0x1000,
	ioH1Unused2			=		0x2000,
	ioH1Unused3			=		0x4000,
	ioH1Unused4			=		0x8000,
} TGPIOH1;

/**
 * GPIO 2
 *
 * GPA0 - H2C1 - enable VOUT - Output
 * GPA1 - H2C2 - enable VOUT - Output
 * GPA2 - H2C3 - enable VOUT - Output
 * GPA3 - H2C4 - enable VOUT - Output
 *
 * GPA4 - Fuse H2C4 - Input
 * GPA5 - Fuse H2C3 - Input
 * GPA6 - Fuse H2C3 - Input
 * GPA7 - Fuse H2C1 - Input
 *
 * GPB0 - Unused
 * GPB1 - UV_H2_C1 - Input
 * GPB2 - UV_H2_C2 - Input
 * GPB3 - UV_H2_C4 - Input
 * GPB4 - UV_H2_C4 - Input
 *
 */

typedef enum tag_TGPIOH2
{
	ioH2C1VOutEnabled 	=		0x0001,
	ioH2C2VOutEnabled 	=		0x0002,
	ioH2C3VOutEnabled 	=		0x0004,
	ioH2C4VOutEnabled 	=		0x0008,

	ioH2C1FuseOK 		=		0x0010,
	ioH2C2FuseOK 		=		0x0020,
	ioH2C3FuseOK 		=		0x0040,
	ioH2C4FuseOK 		=		0x0080,

	ioH2Unused1 		=		0x0100,
	ioH2C1UnderVoltage 	=		0x0200,
	ioH2C2UnderVoltage 	=		0x0400,
	ioH2C3UnderVoltage 	=		0x0800,

	ioH2C4UnderVoltage 	=		0x1000,
	ioH2Unused2 		=		0x2000,
	ioH2Unused3 		=		0x4000,
	ioH2Unused4 		=		0x8000,
} TGPIOH2;

/**
 * Initializes the General purpose IO
 *
 * During initialization the chip is reset and the IOs are
 * programmes as inputs/outputs according to the comments above
*/
void MCP23S17Init1();
void MCP23S17Init2();

/**
* Read all 16 bits from the IO expander
* Low 8 bits are port A
* High 8 bits are port B
*/
unsigned short MCP23S17ReadHead1();
unsigned short MCP23S17ReadHead2();

/**
* Write all 16 bits to the IO expander
* Low 8 bits are port A
* High 8 bits are port B
*/
void MCP23S17WriteHead1(unsigned short data);
void MCP23S17WriteHead2(unsigned short data);


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
void MCP23S17EnableVout(unsigned char aHead, unsigned long aChanel1, unsigned long aChanel2, unsigned long aChanel3, unsigned long aChanel4);

#endif
