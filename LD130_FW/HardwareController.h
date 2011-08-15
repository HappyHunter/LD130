////////////////////////////////////////////////////////////////////
//
// 	Alexandr Filenkov 2011
//
////////////////////////////////////////////////////////////////////

#ifndef HardwareController_120718
#define HardwareController_120718

#include "..\Intf\LD130Common.h"


//-----------------------------------------------------------------------------------------
//
//
//
//-----------------------------------------------------------------------------------------

/**
 *
 * There are several options for triggering.
 *
 * First the simplest and commonly used when the delay+width is
 * smaller that max timer count of 65535 (with 40Mhz it is 1.638
 * msec).In this case we will use timer 2,3 with the instruction
 * counter as a clock source.
 *
 * Second option is when delay+width is greater than 65535 AND
 * width is smaller than 65535. In this case we will use timer 1
 * resolution of 100x clock to start the PWM pulse
 *
 * Third option is when delay+width is greater than 65535 AND
 * delay is smaller than 65535. In this mode we will use Timer
 * 2,3 to count the delay and use Timer 1 to count the duration
 *
 * Fourth option when delay is less than 65535 and width is less than 65535
 * but together they are larger than 65535. In this case we use T2/T3
 * for delay and then use PWM for width
 *
 * Last option is when both delay and width is greater than
 * 65535. In this case we use Timer 1 as an event
 */
typedef enum tag_TTimerFlags
{
	tfPWMOnly			= 0x01,	// simpliest just use PWM
	tfDelayAsT1			= 0x02,	// delay use T1
	tfWidthAsT1			= 0x04,	// width use T1, delay will use either T1 or T2/T3
	tfDelayAsT2T3		= 0x08,	// delay use T2/T3
	tfWidthAsPWM		= 0x10,	// width use PWM module, delay will use either T1 or T2/T3
	tfDelayAsT1ON		= 0x20,	// delay use T1 and is active
	tfWidthAsT1ON		= 0x40,	// width use T1 and is active
	tfTriggerON			= 0x80,	// flag is set when trigger is detected and we still did not finish the delay and width
} TTimerFlags;

/**
* The bank can have the individual settings for each output head. There are 2 output Heads
*
* We can have up to 4 banks of settings, and user can select which bank to use at given moment
*/
typedef struct tag_TBankInfo
{
	unsigned short			m_id;			// the id of the bank
	TBankHeadData 			m_output[MAX_NUM_OF_HEADS];
//	unsigned char			m_flags[MAX_NUM_OF_HEADS];		// flags from TTimerFlags
	unsigned char			m_reserved[16];	// reserved for future use
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
volatile THeadStatus* getHeadStatus(unsigned char anOutputId);



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
 * Returns the counter for the missing trigger. Every time we
 * trigger the controller during the period while it is still doing
 * the previous trigger we will increment the counter. This is
 * just for displaying and statistics purpose
 *
 * @return unsigned long
 */
unsigned long getMissingTriggerCounter1();
unsigned long getMissingTriggerCounter2();


/**
 * Returns the counter for the interrupt trigger. Every time
 * trigger interrupt arrives this counter is incremented. This
 * is just for displaying and statistics purpose
 *
 * @return unsigned long
 */
unsigned long getInterruptTriggerCounter();

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


/**
* Starts the high priority task to advance to the next bank
*
* We implement this as a task not as an interrup since it will
* take some time to reprogram the hardware
*/
void Task_NextBank (void);

/**
 * The function is called for every tick of Timer 1
 *
 * This will be used to trigger the light on long duration
 * pulses
*/
//void processTimer1(void);

//void programBank(TBankInfo* pBankInfo);

void initTrigger1Loopback(void);


void processNextSequence(void);


//short setCurrentDACValue(unsigned char aHead, unsigned char aChanel, unsigned long aValue, unsigned long anAmplifierValue);
//short setVoltageDACValue(unsigned char aHead, unsigned long aValue);
//
//unsigned char headToCurrentChipSelect(unsigned char aHead);
//unsigned char headToVoltageChipSelect(unsigned char aHead);

#endif
