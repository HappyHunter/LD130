////////////////////////////////////////////////////////////////////
//
// 	Alexandr Filenkov 2011
//
////////////////////////////////////////////////////////////////////

#ifndef HardwareController_120718
#define HardwareController_120718

#include "../Common/LD130Common.h"


//-----------------------------------------------------------------------------------------
//
//
//
//-----------------------------------------------------------------------------------------

/**
* The bank can have the individual settings for each output head. There are 2 output Heads
*
* We can have up to 4 banks of settings, and user can select which bank to use at given moment
*/
typedef struct tag_TBankInfo
{
	unsigned short			m_id;			// the id of the bank
	TBankHeadData 			m_output[MAX_NUM_OF_HEADS];
	volatile unsigned short	m_strobeTimerPrescaler[MAX_NUM_OF_HEADS];	// prescaler 1:1 1:8 1:64 1:256

	unsigned char	m_reserved[16];	// reserved for future use
} TBankInfo;


#if 0
//-----------------------------------------------------------------------------------------
// the flash info structure. This structure is stored in the flash memory
//-----------------------------------------------------------------------------------------
typedef struct tag_TFlashInfo
{
	unsigned short m_signature1;		// the begin signature == 0x2512
	unsigned short m_structVersion;		// the version of the flash data structure

	char m_serialNumber[36];			// the controller serial number in form of GUID

	unsigned long m_flags;				// various flags that control how we can use data from flash

	unsigned short m_activeBank;		// currently selected bank

	unsigned long m_uart1_baud_rate;	// stored baud rate for UART 1
	unsigned long m_uart2_baud_rate;	// stored baud rate for UART 2

	TBankInfo m_bankInfo[MAX_NUM_OF_BANKS];	// the data for each output bank

	unsigned char  m_bankSequence[512];	// the trigger sequence that we will use
	unsigned short m_bankSequenceEnd;	// the position AFTER the last valid element

	unsigned short m_signature2;		// the end signature == 0x2011
} TFlashInfo;

#endif

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

/**
 * retruns the sequence bank ID (1,2,3,4) at the specific index
*/
unsigned short getBankSequenceAt(unsigned short anIndex);

/**
* Sets the max sequence length
*/
void setBankSequenceLength(unsigned short aLength);


/**
* Returns the max sequence length
*/
unsigned short getBankSequenceLength();

/**
* returns the current pointer in the bank sequence
*/
unsigned short getCurrentBankSequencePosition();


/**
* getBankInfo() returns pointer to a bank info by the bank ID (1,2,3,4)
*/
TBankInfo* getBankInfo(unsigned short aBankId);

/**
* returns the status of the head after programming
*/
THeadStatus* getHeadStatus(unsigned char anOutputId);



/**
 * Returns the counter for the trigger. Every time we trigger
 * the controller we will increment the counter. This is just
 * for displaying and statistics purpose
 *
 * @return unsigned long
 */
unsigned long getTriggerCounter1();
unsigned long getTriggerCounter2();


/**
 * Reprograms all the DAC settings to 0
*/
void resetAllDACs(void);


/**
 * Initializes all the input capture module properties to enable
 * the interrupts on trigger change
*/
void initTrigger1(void);
void initTrigger2(void);


void programBank(TBankInfo* pBankInfo);


void initTrigger1Loopback(void);


void processNextSequence(void);

//short setCurrentDACValue(unsigned char aHead, unsigned char aChanel, unsigned long aValue, unsigned long anAmplifierValue);
//short setVoltageDACValue(unsigned char aHead, unsigned long aValue);
//
//unsigned char headToCurrentChipSelect(unsigned char aHead);
//unsigned char headToVoltageChipSelect(unsigned char aHead);

#endif
