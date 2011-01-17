////////////////////////////////////////////////////////////////////
//
// 	Alexandr Filenkov 2011
//
////////////////////////////////////////////////////////////////////

#ifndef MCP23S17_201101071319
#define MCP23S17_201101071319

// Function : MCP23S17Init()
// Notes: Setup CS and RESET pins
// Reset the MCP23S17
void MCP23S17Init1();
void MCP23S17Init2();

/**
* Read all 16 bits from the IO expander
*/
unsigned short MCP23S17ReadHead1();
unsigned short MCP23S17ReadHead2();

/**
* Write all 16 bits to the IO expander
*/
void MCP23S17WriteHead1(unsigned short data);
void MCP23S17WriteHead2(unsigned short data);

#endif
