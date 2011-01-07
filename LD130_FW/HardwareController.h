////////////////////////////////////////////////////////////////////
//
// 	Alexandr Filenkov 2008
//
////////////////////////////////////////////////////////////////////

#ifndef HardwareController_110118
#define HardwareController_110118

#include "../Common/LD130Common.h"


typedef struct tag_TBankHeadStatus
{
	unsigned short	m_chanelStatusFlags[4];	// the bit mask status flags for each chanel
											// we will calculate the prescaler based on the strobe delay/width specified
} TBankHeadStatus;

//-----------------------------------------------------------------------------------------
// the bank can have the individual settings for each output head
//
// We can have up to 4 banks of settings, and user can select which bank to use at given moment
//-----------------------------------------------------------------------------------------
typedef struct tag_TBankInfo
{
	unsigned short			m_id;			// the id of the bank
	TBankHeadData 			m_output[2];
	volatile unsigned short	m_strobeTimerPrescaler[2];	// prescaler 1:1 1:8 1:64 1:256

	unsigned char	m_reserved[16];	// reserved for future use
} TBankInfo;


/**
 * Initializes all the bank information structures with the
 * default values of 0
*/
void InitAllBankInfo();


/**
 * Returns the currently selected bank ID (1,2,3,4)
 */
unsigned short getActiveBank();

/**
 * Selects the active bank and re-programs the hadrware
 * if the bank ID has changed
 *
 * @param if bForceProgramming flag is non zero then it will
 * re-program the hardware even if the bank ID is the same
 *
 * @return unsigned char if 0 then no actual re-programming was
 *  	   done, 1 if the hardware was re-programmed
*/
unsigned char setActiveBank(unsigned short anActiveBank, unsigned char bForcePrograming);


/**
 * Returns the flags description from TConfigDataFlags that
 * control initialization of the controller
*/
unsigned long getConfigFlags();


/**
 * Perfroms software trigger event
*/
void fireSoftTrigger(unsigned char aTriggerId);


/**
 * Sets the sequence bank ID (1,2,3,4) at the specific index
*/
void setBankSequenceAt(unsigned short aBankId, unsigned short anIndex);


void initTrigger1(void);
void initTrigger2(void);


void programBank(TBankInfo* pBankInfo);

void delay_us(unsigned long aTimeInMicrosec);

void resetAllDACs(void);

void initTrigger1Loopback(void);


void processNextSequence(void);

short setCurrentDACValue(unsigned char aHead, unsigned char aChanel, unsigned long aValue, unsigned long anAmplifierValue);
short setVoltageDACValue(unsigned char aHead, unsigned long aValue);

unsigned char headToCurrentChipSelect(unsigned char aHead);
unsigned char headToVoltageChipSelect(unsigned char aHead);

#endif
