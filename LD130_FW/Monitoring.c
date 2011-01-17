////////////////////////////////////////////////////////////////////
//
// 	Alexandr Filenkov 2010
//
////////////////////////////////////////////////////////////////////

#include <p30fxxxx.h>
#include "Monitoring.h"
#include "osa.h"
#include "Common.h"
#include "lcd.h"
#include "MCP23S17.h"
#include "SPI.h"
#include "LCDMan.h"


static float theTemperatureH1 = 0;
static float theTemperatureH2 = 0;
static float theTemperatureAmb = 0;

static unsigned short theGPIOHead1 = 0;
static unsigned short theGPIOHead2 = 0;

/**
 * SPI_Chip_Select_1 - GPIO 1
 * SPI_Chip_Select_2 - GPIO 2
 * SPI_Chip_Select_3 - Head 1 temp sensor
 * SPI_Chip_Select_4 - Head 2 temp sensor
 * SPI_Chip_Select_5 - Ambiant temperature sensor
 *
 */



/**
 * GPIO 1
 *
 * GPA4 - Fuse H1C4
 * GPA5 - Fuse H1C3
 * GPA6 - Fuse H1C3
 * GPA7 - Fuse H1C1
 *
 * GPB0 - AC-DC Head 1
 * GPB1 - UV_H1_C1
 * GPB2 - UV_H1_C2
 * GPB3 - UV_H1_C4
 * GPB4 - UV_H1_C4
 *
 */

/**
 * GPIO 2
 *
 * GPA4 - Fuse H2C4
 * GPA5 - Fuse H2C3
 * GPA6 - Fuse H2C3
 * GPA7 - Fuse H2C1
 *
 * GPB0 - AC-DC Head 2
 * GPB1 - UV_H2_C1
 * GPB2 - UV_H2_C2
 * GPB3 - UV_H2_C4
 * GPB4 - UV_H2_C4
 *
 */

void Task_Monitoring()
{

	static short tmp = 0;
	_LATA9 = 0;
	delay_us(100); // reset for 10*10K cycles
	_LATA9 = 1;

	MCP23S17Init1();
	MCP23S17Init2();

	theTemperatureH1 = 0;
	theTemperatureH2 = 0;
	theTemperatureAmb = 0;

	for (;;) {
		ClrWdt();
		OS_Yield(); // Unconditional context switching


		/**
		 * Read temperature Head 1
		 */
		OS_Bsem_Wait(SPI_2_Busy_Sema);
		initSPI2(1/*16bit*/, 0, 0);
		SPI2_ChipSelect_Single(SPI_Chip_Select_3, 0); //temp sensor H1
		tmp = SPI2_WriteRead(0x0000); // just read
		tmp = tmp >> 3;  // sign dependand shifting
		theTemperatureH1 = tmp * 0.0625f;
		OS_Bsem_Set(SPI_2_Busy_Sema);

		ClrWdt();
		OS_Yield(); // Unconditional context switching

		/**
		 * Read temperature Head 2
		 */
		OS_Bsem_Wait(SPI_2_Busy_Sema);
		initSPI2(1/*16bit*/, 0, 0);
		SPI2_ChipSelect_Single(SPI_Chip_Select_4, 0); //temp sensor H2
		tmp = SPI2_WriteRead(0x0000); // just read
		tmp = tmp >> 3;  // sign dependand shifting
		theTemperatureH2 = tmp * 0.0625f;
		OS_Bsem_Set(SPI_2_Busy_Sema);

		ClrWdt();
		OS_Yield(); // Unconditional context switching

		/**
		 * Read temperature Ambiant
		 */
		OS_Bsem_Wait(SPI_2_Busy_Sema);
		initSPI2(1/*16bit*/, 0, 0);
		SPI2_ChipSelect_Single(SPI_Chip_Select_5, 0); //temp sensor Ambiant
		tmp = SPI2_WriteRead(0x0000); // just read
		tmp = tmp >> 3;  // sign dependand shifting
		theTemperatureAmb = tmp * 0.0625f;
		OS_Bsem_Set(SPI_2_Busy_Sema);


		ClrWdt();
		OS_Yield(); // Unconditional context switching

		/**
		 * Read GPIO for head 1
		 */
		theGPIOHead1 = MCP23S17ReadHead1(); // just read

		ClrWdt();
		OS_Yield(); // Unconditional context switching


		/**
		 * Read GPIO for head 2
		 */
		theGPIOHead2 = MCP23S17ReadHead2(); // just read


	}
}


//-----------------------------------------------------------------------------------------
float getTemperatureH1()
{
	return theTemperatureH1;
}

//-----------------------------------------------------------------------------------------
float getTemperatureH2()
{
	return theTemperatureH2;
}


//-----------------------------------------------------------------------------------------
float getTemperatureAmb()
{
	return theTemperatureAmb;
}

