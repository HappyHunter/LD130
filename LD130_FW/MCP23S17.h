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
 * GPA0
 * GPA1
 * GPA2
 * GPA3
 * GPA4 Fuse H1C4 - Input
 * GPA5 - Fuse H1C3 - Input
 * GPA6 - Fuse H1C3 - Input
 * GPA7 - Fuse H1C1 - Input
 *
 * GPB0 - AC-DC Head 1 - Output
 * GPB1 - UV_H1_C1 - Input
 * GPB2 - UV_H1_C2 - Input
 * GPB3 - UV_H1_C4 - Input
 * GPB4 - UV_H1_C4 - Input
 *
 */

/**
 * GPIO 2
 *
 * GPA0
 * GPA1
 * GPA2
 * GPA3
 * GPA4 - Fuse H2C4 - Input
 * GPA5 - Fuse H2C3 - Input
 * GPA6 - Fuse H2C3 - Input
 * GPA7 - Fuse H2C1 - Input
 *
 * GPB0 - AC-DC Head 2 - Output
 * GPB1 - UV_H2_C1 - Input
 * GPB2 - UV_H2_C2 - Input
 * GPB3 - UV_H2_C4 - Input
 * GPB4 - UV_H2_C4 - Input
 *
 */

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

#endif
