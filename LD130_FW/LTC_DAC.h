////////////////////////////////////////////////////////////////////
//
// 	Alexandr Filenkov 2011
//
////////////////////////////////////////////////////////////////////

#ifndef LTCDAC_201101231328
#define LTCDAC_201101231328


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
short setCurrentDACValue(unsigned char aHead, unsigned char aChanel, unsigned long aValue, unsigned long anAmplifierValue);


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
short setVoltageDACValue(unsigned char aHead, unsigned long aValue);


#endif
