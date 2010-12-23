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


void InitAllBankInfo();





void initTrigger1(void);
void initTrigger2(void);

void setActiveBank(unsigned short activeBank);

void programBank(TBankInfo* pBankInfo);

void delay_us(unsigned long aTimeInMicrosec);

void resetAllDACs(void);

void fireSoftTrigger(unsigned char aTriggerId);
void initTrigger1Loopback(void);


void processNextSequence(void);

short setCurrentDACValue(unsigned char aHead, unsigned char aChanel, unsigned long aValue, unsigned long anAmplifierValue);
short setVoltageDACValue(unsigned char aHead, unsigned long aValue);

unsigned char headToCurrentChipSelect(unsigned char aHead);
unsigned char headToVoltageChipSelect(unsigned char aHead);

#endif
