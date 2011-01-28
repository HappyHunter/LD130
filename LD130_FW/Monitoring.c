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

#define FILTER_SIZE 5
static float theTemperatureH1[FILTER_SIZE];
static float theTemperatureH2[FILTER_SIZE];
static float theTemperatureAmb[FILTER_SIZE];

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
	static short iReading = 0;


	for (iReading = 0; iReading < sizeof(theTemperatureH1)/sizeof(theTemperatureH1[0]); ++iReading) {
		theTemperatureH1[iReading] = 0;
		theTemperatureH2[iReading] = 0;
		theTemperatureAmb[iReading] = 0;
	}

	for (iReading = 0;;++iReading) {
		// overflow protection
		if (iReading >= sizeof(theTemperatureH1)/sizeof(theTemperatureH1[0])) {
			iReading = 0;
		}

		ClrWdt();
		OS_Yield(); // Unconditional context switching

		/**
		 * Read temperature Head 1
		 */
		initSPI2(1/*16bit*/, 0, 1);
		tmp = SPI2_NegSelectWriteRead(SPI_Chip_Select_3, 0x0000); // just read
		tmp = tmp >> 3;  // sign dependand shifting
		theTemperatureH1[iReading] = tmp * 0.0625f;

		#if 0
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
		theTemperatureH2[iReading] = tmp * 0.0625f;

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
		theTemperatureAmb[iReading] = tmp * 0.0625f;

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
		#if 0
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
		#if 0
		DbgOut("theGPIOHead2={");
		DbgOutInt(theGPIOHead2);
		DbgOut("}\r\n");
		OS_Delay(950000UL);
		#endif

		OS_Delay(50);
	}
}


//-----------------------------------------------------------------------------------------
float getTemperatureH1()
{
	float fVal = 0;
	short iReading = 0;
	for (iReading = 0; iReading < sizeof(theTemperatureH1)/sizeof(theTemperatureH1[0]); ++iReading) {
		fVal += theTemperatureH1[iReading];
	}
	return fVal/(sizeof(theTemperatureH1)/sizeof(theTemperatureH1[0]));
}

//-----------------------------------------------------------------------------------------
float getTemperatureH2()
{
	float fVal = 0;
	short iReading = 0;
	for (iReading = 0; iReading < sizeof(theTemperatureH2)/sizeof(theTemperatureH2[0]); ++iReading) {
		fVal += theTemperatureH2[iReading];
	}
	return fVal/(sizeof(theTemperatureH2)/sizeof(theTemperatureH2[0]));
}


//-----------------------------------------------------------------------------------------
float getTemperatureAmb()
{
	float fVal = 0;
	short iReading = 0;
	for (iReading = 0; iReading < sizeof(theTemperatureAmb)/sizeof(theTemperatureAmb[0]); ++iReading) {
		fVal += theTemperatureAmb[iReading];
	}
	return fVal/(sizeof(theTemperatureAmb)/sizeof(theTemperatureAmb[0]));
}

