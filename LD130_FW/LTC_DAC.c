////////////////////////////////////////////////////////////////////
//
// 	Alexandr Filenkov 2010
//
////////////////////////////////////////////////////////////////////

#include <p30fxxxx.h>
#include "LTC_DAC.h"
#include "osa.h"
#include "Common.h"
#include "SPI.h"


#define VOUT_ONA _LATB11
#define VOUT_ONB _LATB12


//#define LOG_SETTING_VALUES

/**
 * This function calculates the DAC current value to be
 * programmed to LTC1660 DAC
 *
 * The DAC can be programmed from 0 - 1023
 *
 * Inputs:
 * unsigned long aPower 			- from 0% to 100% with 2
 * fixed decimal digits, so the value of 100% is represented as
 * 10000
 *
 *
 * unsigned long anAmplifierValue	- from 1 - 5
*/
//-----------------------------------------------------------------------------------------
unsigned short makeCurrentDACValue_LTC1660(unsigned long aPower, unsigned long anAmplifierValue)
{
	unsigned long retValue = 1023UL;

	if (aPower >= 10000)
		aPower = 10000;

	if (anAmplifierValue > 5)
		anAmplifierValue = 5;

	if (anAmplifierValue == 0)
		anAmplifierValue = 1;

	retValue *= aPower;

	retValue = (retValue / 5) * anAmplifierValue;

	// first we divide by 100, to remove the fixed decimal point
	// and then we divide by 100% to get the actal value
	retValue /= 10000UL;

	return (retValue);
}

/**
 *
 * This function calculates the DAC voltage value to be
 * programmed to LTC1329 DAC
 *
 * The DAC can be programmed from 0 (100% voltage) - 255 (0% voltage)
 *
 * unsigned long aPower 			- from 0% to 100% in integer steps
 *
*/
//-----------------------------------------------------------------------------------------
unsigned short makeVoltageDACValue_LTC1329(unsigned long aPower)
{
	unsigned long retValue = 255UL;

	if (aPower >= 100)
		aPower = 100;

	aPower = 100 - aPower;

	retValue *= aPower;

	// We divide by 100% to get the actual value
	retValue /= 100UL;

	return (retValue);
}


/**
 * This function will actually program the DAC itself with the
 * values specified
 *
 * The DAC has 8 chanels and they are 10 bit
 *
 * Inputs:
 *
 * unsigned char aChip - is enum from TSpiChanels which defines
 * which DAC we need to program
 *
 * unsigned char aChanel - specifies the chanel to be programmed
 *
 * unsigned short aValue - is the actual 10 bit value to be
 * programmed
*/
//-----------------------------------------------------------------------------------------
short setCurrentDACValue_LTC1660(unsigned char aChip, unsigned char aChanel, unsigned short aValue)
{
	unsigned short workingChanelBin = 0x1000;
	#ifdef LOG_SETTING_VALUES
	unsigned short originalValue = aValue;
	#endif

	switch (aChanel) {
		case 1:
			workingChanelBin = 0x1000;
			break;

		case 2:
			workingChanelBin = 0x2000;
			break;

		case 3:
			workingChanelBin = 0x3000;
			break;

		case 4:
			workingChanelBin = 0x4000;
			break;

		case 5:
			workingChanelBin = 0x5000;
			break;

		case 6:
			workingChanelBin = 0x6000;
			break;

		case 7:
			workingChanelBin = 0x7000;
			break;

		case 8:
			workingChanelBin = 0x8000;
			break;
	}

	if (aValue > 1023)
		aValue = 1023;

	// shift it to the left by 2 bits, since the last 2 bits are
	// not used in this DAC
	aValue <<= 2;

	aValue = workingChanelBin | aValue;


	#ifdef LOG_SETTING_VALUES
	DbgOut("setCurrentDACValue_LTC1660(");
	DbgOutInt(aChip);
	DbgOut(",");
	DbgOutInt(aChanel);
	DbgOut(",");
	DbgOutInt(aValue);
	DbgOut(" {");
	DbgOutInt(originalValue);
	DbgOut("})\r\n");
	#endif

	// select slave device
	initSPI1(1/*1 = 16 bit ON*/, 1 /*CKE*/, 0/*CKP*/);
	// select slave device
	SPI1_NegSelectWriteRead(aChip, aValue);

	return (1);
}

/**
 * This function will actually program the DAC itself with the
 * values specified
 *
 * The DAC has 1 chanel and it is 8 bit DAC
 *
 * Inputs:
 *
 * unsigned char aChip - is enum from TSpiChanels which defines
 * which DAC we need to program
 *
 * unsigned char aChanel - specifies the chanel to be programmed
 *
 * unsigned short aValue - is the actual 8 bit value to be
 * programmed
*/
//-----------------------------------------------------------------------------------------
short setVoltageDACValue_LTC1329(unsigned char aChip, unsigned short aValue)
{
	#ifdef LOG_SETTING_VALUES
	DbgOut("setVoltageDACValue_LTC1329(");
	DbgOutInt(aChip);
	DbgOut(",");
	DbgOutInt(aValue);
	DbgOut(")\r\n");
	#endif

	initSPI1(1/*1 = 16 bit ON*/, 1 /*CKE*/, 0/*CKP*/);
	// select slave device
	SPI1_NegSelectWriteRead(aChip, aValue);

	return (1);
}

/**
 * Converts the Head ID to the chip select for programming the
 * Current value
*/
//-----------------------------------------------------------------------------------------
unsigned char headToCurrentChipSelect(unsigned char aHead)
{
	switch (aHead) {
		case 1:	return SPI_Chip_Select_1; // Chip 1
		case 2:	return SPI_Chip_Select_2; // Chip 2
	}
	return SPI_Chip_Select_None; //error
}

/**
 * Converts the Head ID to the chip select for programming the
 * Voltage value
*/
//-----------------------------------------------------------------------------------------
unsigned char headToVoltageChipSelect(unsigned char aHead)
{
	switch (aHead) {
		case 1:	return SPI_Chip_Select_3; // Chip 3
		case 2:	return SPI_Chip_Select_4; // Chip 4
	}
	return SPI_Chip_Select_None; //error
}

/**
 * Programs the Current into the DAC's chanel with the value
 * specified
 *
 *
 * Inputs:
 *
 *  unsigned char aHead - the Head ID 1 or 2
 *
 *  unsigned char aChanel - the chanel ID: 1,2,3,4
 *
 *  unsigned long aValue - the current value in percents 0 - 100
 *  00% with fixed decimal point at 2 digits
*/
//-----------------------------------------------------------------------------------------
short setCurrentDACValue(unsigned char aHead, unsigned char aChanel, unsigned long aValue, unsigned long anAmplifierValue)
{
	return setCurrentDACValue_LTC1660(headToCurrentChipSelect(aHead), aChanel, makeCurrentDACValue_LTC1660(aValue, anAmplifierValue));
}

/**
 * Programs the Voltage into the DAC with the value specified
 *
 *
 * Inputs:
 *
 *  unsigned char aHead - the Head ID 1 or 2
 *
 *  unsigned long aValue - the current value in percents 0 -
 *  100%
*/
//-----------------------------------------------------------------------------------------
short setVoltageDACValue(unsigned char aHead, unsigned long aValue)
{
	switch (aHead) {
		case 1:
			VOUT_ONA = (aValue > 0); //turn ON voltage for nonzero value
			break;
		case 2:
			VOUT_ONB = (aValue > 0); //turn ON voltage for nonzero value
			break;
	}

	return setVoltageDACValue_LTC1329(headToVoltageChipSelect(aHead), makeVoltageDACValue_LTC1329(aValue));
}

