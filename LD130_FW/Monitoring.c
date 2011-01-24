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




void Task_Monitoring()
{
	static short tmp = 0;
//	static unsigned long tmp1 = 0;


	theTemperatureH1 = 0;
	theTemperatureH2 = 0;
	theTemperatureAmb = 0;
	for (;;) {
		ClrWdt();
		OS_Yield(); // Unconditional context switching
		OS_Timer();

		/**
		 * Read temperature Head 1
		 */
		initSPI2(1/*16bit*/, 0, 1);
		tmp = SPI2_NegSelectWriteRead(SPI_Chip_Select_3, 0x0000); // just read
		tmp = tmp >> 3;  // sign dependand shifting
		theTemperatureH1 = tmp * 0.0625f;

		#if 1
		DbgOut("H1={");
//		DbgOutInt(tmp<<3);
//		DbgOut("} {");
//		DbgOutInt(tmp);
//		DbgOut("} {");
		DbgOutFloat(theTemperatureH1);
		DbgOut("}\r\n");
		OS_Delay(64000);
		#endif

		ClrWdt();
		OS_Yield(); // Unconditional context switching

		/**
		 * Read temperature Head 2
		 */
		initSPI2(1/*16bit*/, 0, 1);
		tmp = SPI2_NegSelectWriteRead(SPI_Chip_Select_4, 0x0000); // just read
		// in case if sensor is not connected then the return is FFFF
		if (tmp == 0xFFFF) tmp = 0;
		tmp = tmp >> 3;  // sign dependand shifting
		theTemperatureH2 = tmp * 0.0625f;

		#if 0
		DbgOut("H2={");
		DbgOutInt(tmp<<3);
		DbgOut("} {");
		DbgOutInt(tmp);
		DbgOut("} {");
		DbgOutFloat(theTemperatureH2);
		DbgOut("}\r\n");
		OS_Delay(64000);
		#endif

		ClrWdt();
		OS_Yield(); // Unconditional context switching

		/**
		 * Read temperature Ambiant
		 */
		initSPI2(1/*16bit*/, 0, 1);
		tmp = SPI2_NegSelectWriteRead(SPI_Chip_Select_5, 0x0000); // just read
		tmp = tmp >> 3;  // sign dependand shifting
		theTemperatureAmb = tmp * 0.0625f;

		#if 0
		DbgOut("AM={");
		DbgOutInt(tmp<<3);
		DbgOut("} {");
		DbgOutInt(tmp);
		DbgOut("} {");
		DbgOutFloat(theTemperatureAmb);
		DbgOut("}\r\n");
		OS_Delay(64000);
		#endif


		ClrWdt();
		OS_Yield(); // Unconditional context switching

		/**
		 * Read GPIO for head 1
		 */
		theGPIOHead1 = MCP23S17ReadHead1(); // just read
		#if 1
		DbgOut("theGPIOHead1={");
		DbgOutInt(theGPIOHead1);
		DbgOut("}\r\n");
		OS_Delay(64000);
		#endif

		ClrWdt();
		OS_Yield(); // Unconditional context switching


		/**
		 * Read GPIO for head 2
		 */
		theGPIOHead2 = MCP23S17ReadHead2(); // just read
		#if 1
		DbgOut("theGPIOHead2={");
		DbgOutInt(theGPIOHead2);
		DbgOut("}\r\n");
		#endif

		OS_Delay(950000UL);
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

