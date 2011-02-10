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
#include "HardwareController.h"

#define FILTER_SIZE 5
static float theTemperatureH1[FILTER_SIZE];
static float theTemperatureH2[FILTER_SIZE];
static float theTemperatureAmb[FILTER_SIZE];

static unsigned short theGPIOHead1 = 0;
static unsigned short theGPIOHead2 = 0;

static unsigned short theUART1Disabled = 0;
static unsigned short theUART2Disabled = 0;

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

		// in case if UART1 got disabled for long time
		if (U1MODEbits.UARTEN)
			theUART1Disabled = 0;
		else if (++theUART1Disabled > 10)
			ReStart_UART1();

		// in case if UART1 got disabled for long time
		if (U2MODEbits.UARTEN)
			theUART2Disabled = 0;
		else if (++theUART2Disabled > 10)
			ReStart_UART2();

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


/**
 * Returns the status string for any errors happening
*/
//-----------------------------------------------------------------------------------------
const char* getErrorStatus()
{
	static char buf[11];
	buf[sizeof(buf)-1] = 0;

	// UART 1 status error codes
	buf[9] = '0';
	if (U1STAbits.FERR)			buf[9] += 1;
	if (U1STAbits.PERR) 		buf[9] += 2;
	if (U1STAbits.OERR)			buf[9] += 4;
	if (U1MODEbits.UARTEN==0)	buf[9] += 8;


	// UART 2 status error codes
	buf[8] = '0';
	if (U2STAbits.FERR)			buf[8] += 1;
	if (U2STAbits.PERR) 		buf[8] += 2;
	if (U2STAbits.OERR)			buf[8] += 4;
	if (U1MODEbits.UARTEN==0)	buf[8] += 8;

	// indication that there is a missing trigger 1
	buf[7] = '0' + getMissingTriggerCounter1() % 10;

	// indication that there is a missing trigger 2
	buf[6] = '0' + getMissingTriggerCounter2() %10;

	buf[5] = '0';
	buf[4] = '0';
	buf[3] = '0';
	buf[2] = '0';
	buf[1] = '0';
	buf[0] = '0';

	return buf;
}

