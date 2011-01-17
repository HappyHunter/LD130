////////////////////////////////////////////////////////////////////
//
// 	Alexandr Filenkov 2010
//
////////////////////////////////////////////////////////////////////


#include <p30fxxxx.h>
#include "osa.h"
#include "LCDMan.h"
#include "LCD.h"
#include <string.h>
#include "HardwareController.h"
#include "Monitoring.h"
#include "Common.h"

#define NUM_OF_SCREENS 3

static char TheScreens[NUM_OF_SCREENS][LCD_MAXROWS][LCD_MAXCOLS];
//static char TheScreensChanged[NUM_OF_SCREENS];
/*
*    01234567890123456789
*
*    1A:100.00%1B:100.00%
*    1C:100.00%1D:100.00%
*    2A:100.00%2B:100.00%
*    2C:100.00%2D:100.00%
*
*    D:999999999us.B1.RTI
*    W:999999999us.999999
*    D:999999999us.999999
*    D:999999999us.......
*
*    1:P100.00.O:1 B1.RTI
*    2:P100.00.D999999999
*    3:P100.00.W999999999
*    4:P100.00.C999999999
*
*    1:P100.00.O:2 B1.RTI
*    2:P100.00.D999999999
*    3:P100.00.W999999999
*    4:P100.00.C999999999
*
*    Boreal Inc..........
*    t1:55.5..t2:55.5....
*    ERR:OK..............
*    B:4.NB:4...Ver:1.2.3
*/

void updateScreen1();
void updateScreen2();
void updateScreen3();


//-----------------------------------------------------------------------------------------
void Task_LCDMan (void)
{
	static unsigned short theCounter = 0;
	static unsigned char  i = 0;
	static unsigned char  j = 0;
	static unsigned char  activeScreen = 0;
	static unsigned char  theScreenToUpdate = 0;

	memset(TheScreens, ' ', sizeof(TheScreens));
//	memset(TheScreensChanged, 0, sizeof(TheScreensChanged));

	// wait some time before initializing the LCD to stabilize the power
	while (++theCounter < 30000) {
		ClrWdt();
		OS_Yield(); // Unconditional context switching
	}

	setLcdText(0, 1, 0, "Boreal Inc.");
	setLcdText(0, 4, 9, "Boreal Inc.");
	setLcdText(1, 1, 0, "Boreal Inc.");
	setLcdText(1, 4, 9, "Boreal Inc.");
	setLcdText(2, 1, 0, "Boreal Inc.");
	setLcdText(2, 4, 9, "Boreal Inc.");

	// initialize the LCD controller
	lcd_init();

	for (theCounter = 0; ;++theCounter) {
		ClrWdt();
		OS_Yield(); // Unconditional context switching

		// update the active LCD screen
		for (i = 0; i < LCD_MAXROWS; ++i) {
			lcd_gotoxy (i+1, 1);				// Position cursor
			for (j = 0; j < LCD_MAXCOLS; ++j) {
				lcd_putc (TheScreens[activeScreen][i][j]);	//	Show and bump

				// this is really low priority task, so try to yield as much as possible
				ClrWdt();
				OS_Yield(); // Unconditional context switching
			}
		}

		// switch the active screen
		if (++theCounter > 10000) {
			theCounter = 0;
			if (++activeScreen >= NUM_OF_SCREENS)
				activeScreen = 0;

		}


		switch (++theScreenToUpdate % NUM_OF_SCREENS) {
		case 0:
			updateScreen1();
			break;

		case 1:
			updateScreen2();
			break;

		case 2:
			updateScreen3();
			break;
		}
	}
}

//-----------------------------------------------------------------------------------------
void setLcdText(unsigned char aScreenId, unsigned char aLine, unsigned char aCol, const char* pText)
{
	unsigned char iCol;

	// validate the params
	if (aScreenId >= NUM_OF_SCREENS) aScreenId = NUM_OF_SCREENS-1;
	if (aLine >= LCD_MAXROWS) aLine = LCD_MAXROWS-1;
	if (aCol >= LCD_MAXCOLS) aCol = LCD_MAXCOLS-1;

	// clear the line
//	memset(TheScreens[aScreenId][aLine], ' ', LCD_MAXCOLS);

	// now set the line
	for (iCol = aCol; iCol < LCD_MAXCOLS; ++iCol) {
		if (!pText || *pText == 0)
			break;

		TheScreens[aScreenId][aLine][iCol] = *pText;
		++pText;
	}

}

//-----------------------------------------------------------------------------------------
void setLcdChar(unsigned char aScreenId, unsigned char aLine, unsigned char aCol, char aChar)
{
	// validate the params
	if (aScreenId >= NUM_OF_SCREENS) aScreenId = NUM_OF_SCREENS-1;
	if (aLine >= LCD_MAXROWS) aLine = LCD_MAXROWS-1;
	if (aCol >= LCD_MAXCOLS) aCol = LCD_MAXCOLS-1;

	// clear the line
//	memset(TheScreens[aScreenId][aLine], ' ', LCD_MAXCOLS);

	// now set the line
	TheScreens[aScreenId][aLine][aCol] = aChar;

}

//-----------------------------------------------------------------------------------------
const char* getIntAsString(char* buf, unsigned long aValue)
{
	const char* pCh = 0;
	buf[11] = 0;

	buf[10] = (aValue % 10) + '0';
	aValue = aValue / 10;

	buf[9] = (aValue % 10) + '0';
	aValue = aValue / 10;

	buf[8] = (aValue % 10) + '0';
	aValue = aValue / 10;

	buf[7] = (aValue % 10) + '0';
	aValue = aValue / 10;

	buf[6] = (aValue % 10) + '0';
	aValue = aValue / 10;

	buf[5] = (aValue % 10) + '0';
	aValue = aValue / 10;

	buf[4] = (aValue % 10) + '0';
	aValue = aValue / 10;

	buf[3] = (aValue % 10) + '0';
	aValue = aValue / 10;

	buf[2] = (aValue % 10) + '0';
	aValue = aValue / 10;

	buf[1] = (aValue % 10) + '0';
	aValue = aValue / 10;

	buf[0] = (aValue % 10) + '0';


	// strip the leading zeros
	for (pCh = buf; pCh && *pCh == '0'; ++pCh)
		;

	// in case if all are zeros, then still output 0
	if (*pCh == 0){
//		buf[0] = '0';	assume that there is '0' already there
		buf[1] = 0;
		pCh = buf;
	}
	return pCh;
}


//-----------------------------------------------------------------------------------------
void updateScreen1()
{
	unsigned short theActiveBank;
	char buf[12];

	// set the LCD buffer with proper information
	theActiveBank = getActiveBank();

	// set line 1
	setLcdText(0, 0, 0, "1:P");
	getIntAsString(buf, getBankInfo(theActiveBank)->m_output[0].m_powerChanel1);
	setLcdChar(0, 0, 8, buf[10]);
	setLcdChar(0, 0, 7, buf[9]);
	setLcdChar(0, 0, 6, '.');
	setLcdChar(0, 0, 5, buf[8]);
	setLcdChar(0, 0, 4, buf[7]);
	setLcdChar(0, 0, 3, buf[6]);
	setLcdText(0, 0, 9, " O:1 B");
	getIntAsString(buf, theActiveBank);
	setLcdChar(0, 0, 15, buf[10]);
	setLcdText(0, 0, 16, " RTI");

	// set line 2
	setLcdText(0, 1, 0, "2:P");
	getIntAsString(buf, getBankInfo(theActiveBank)->m_output[0].m_powerChanel2);
	setLcdChar(0, 1, 8, buf[10]);
	setLcdChar(0, 1, 7, buf[9]);
	setLcdChar(0, 1, 6, '.');
	setLcdChar(0, 1, 5, buf[8]);
	setLcdChar(0, 1, 4, buf[7]);
	setLcdChar(0, 1, 3, buf[6]);
	setLcdText(0, 1, 9, " D");
	getIntAsString(buf, getBankInfo(theActiveBank)->m_output[0].m_strobeDelay);
	setLcdText(0, 0, 11, &buf[8]);

	// set line 3
	setLcdText(0, 2, 0, "3:P");
	getIntAsString(buf, getBankInfo(theActiveBank)->m_output[0].m_powerChanel3);
	setLcdChar(0, 2, 8, buf[10]);
	setLcdChar(0, 2, 7, buf[9]);
	setLcdChar(0, 2, 6, '.');
	setLcdChar(0, 2, 5, buf[8]);
	setLcdChar(0, 2, 4, buf[7]);
	setLcdChar(0, 2, 3, buf[6]);
	setLcdText(0, 2, 9, " W");
	getIntAsString(buf, getBankInfo(theActiveBank)->m_output[0].m_strobeWidth);
	setLcdText(0, 2, 11, &buf[8]);


	// set line 4
	setLcdText(0, 3, 0, "4:P");
	getIntAsString(buf, getBankInfo(theActiveBank)->m_output[0].m_powerChanel4);
	setLcdChar(0, 3, 8, buf[10]);
	setLcdChar(0, 3, 7, buf[9]);
	setLcdChar(0, 3, 6, '.');
	setLcdChar(0, 3, 5, buf[8]);
	setLcdChar(0, 3, 4, buf[7]);
	setLcdChar(0, 3, 3, buf[6]);
	setLcdText(0, 3, 9, " C");
	getIntAsString(buf, getTriggerCounter1());
	setLcdText(0, 3, 11, &buf[8]);
}


//-----------------------------------------------------------------------------------------
void updateScreen2()
{
	unsigned short theActiveBank;
	char buf[12];

	// set the LCD buffer with proper information
	theActiveBank = getActiveBank();

	// set line 1
	setLcdText(1, 0, 0, "1:P");
	getIntAsString(buf, getBankInfo(theActiveBank)->m_output[1].m_powerChanel1);
	setLcdChar(1, 0, 8, buf[10]);
	setLcdChar(1, 0, 7, buf[9]);
	setLcdChar(1, 0, 6, '.');
	setLcdChar(1, 0, 5, buf[8]);
	setLcdChar(1, 0, 4, buf[7]);
	setLcdChar(1, 0, 3, buf[6]);
	setLcdText(1, 0, 9, " O:2 B");
	getIntAsString(buf, theActiveBank);
	setLcdChar(1, 0, 15, buf[10]);
	setLcdText(1, 0, 16, " RTI");

	// set line 2
	setLcdText(1, 1, 0, "2:P");
	getIntAsString(buf, getBankInfo(theActiveBank)->m_output[1].m_powerChanel2);
	setLcdChar(1, 1, 8, buf[10]);
	setLcdChar(1, 1, 7, buf[9]);
	setLcdChar(1, 1, 6, '.');
	setLcdChar(1, 1, 5, buf[8]);
	setLcdChar(1, 1, 4, buf[7]);
	setLcdChar(1, 1, 3, buf[6]);
	setLcdText(1, 1, 9, " D");
	getIntAsString(buf, getBankInfo(theActiveBank)->m_output[1].m_strobeDelay);
	setLcdText(1, 1, 11, &buf[8]);

	// set line 3
	setLcdText(1, 2, 0, "3:P");
	getIntAsString(buf, getBankInfo(theActiveBank)->m_output[1].m_powerChanel3);
	setLcdChar(1, 2, 8, buf[10]);
	setLcdChar(1, 2, 7, buf[9]);
	setLcdChar(1, 2, 6, '.');
	setLcdChar(1, 2, 5, buf[8]);
	setLcdChar(1, 2, 4, buf[7]);
	setLcdChar(1, 2, 3, buf[6]);
	setLcdText(1, 2, 9, " W");
	getIntAsString(buf, getBankInfo(theActiveBank)->m_output[1].m_strobeWidth);
	setLcdText(1, 2, 11, &buf[8]);


	// set line 4
	setLcdText(1, 3, 0, "4:P");
	getIntAsString(buf, getBankInfo(theActiveBank)->m_output[1].m_powerChanel4);
	setLcdChar(1, 3, 8, buf[10]);
	setLcdChar(1, 3, 7, buf[9]);
	setLcdChar(1, 3, 6, '.');
	setLcdChar(1, 3, 5, buf[8]);
	setLcdChar(1, 3, 4, buf[7]);
	setLcdChar(1, 3, 3, buf[6]);
	setLcdText(1, 3, 9, " C");
	getIntAsString(buf, getTriggerCounter2());
	setLcdText(1, 3, 11, &buf[8]);
}

//-----------------------------------------------------------------------------------------
void updateScreen3()
{
	char buf[12];
	setLcdText(2, 0, 0, "Boreal Inc.");

	setLcdText(2, 1, 0, "t1:");
	getIntAsString(buf, getTemperatureH1());
	setLcdChar(2, 2, 3, buf[10]);
	setLcdChar(2, 2, 4, '.');
	setLcdChar(2, 2, 5, buf[9]);
	setLcdChar(2, 2, 6, buf[8]);
	setLcdChar(2, 2, 7, buf[7]);
	setLcdText(2, 1, 11, "t2:");
	getIntAsString(buf, getTemperatureH2());
	setLcdChar(2, 2, 14, buf[10]);
	setLcdChar(2, 2, 15, '.');
	setLcdChar(2, 2, 16, buf[9]);
	setLcdChar(2, 2, 17, buf[8]);
	setLcdChar(2, 2, 18, buf[7]);

	setLcdText(2, 2, 0, "NO ERROR");

	setLcdText(2, 3, 0, "Ver:");
	setLcdChar(2, 3, 4, VERSION_MAJOR+'0');
	setLcdChar(2, 3, 5, '.');
	setLcdChar(2, 3, 6, VERSION_MINOR+'0');
	setLcdChar(2, 3, 7, '.');
	setLcdChar(2, 3, 8, VERSION_BUILD+'0');
}
