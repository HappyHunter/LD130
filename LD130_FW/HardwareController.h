////////////////////////////////////////////////////////////////////
//
// 	Alexandr Filenkov 2008
//
////////////////////////////////////////////////////////////////////

#ifndef HardwareController_110118
#define HardwareController_110118

#include "../Common/LD130Common.h"

typedef struct tag_TBankInfo
{
	TBankHeadData	m_headData[2];
}TBankInfo;

void initTrigger1(void);
void initTrigger2(void);

void setActiveBank(unsigned short activeBank);

void programBank(const TBankInfo* pBankInfo);

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
