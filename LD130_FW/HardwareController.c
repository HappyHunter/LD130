////////////////////////////////////////////////////////////////////
//
// 	Alexandr Filenkov 2010
//
////////////////////////////////////////////////////////////////////


#include <p30fxxxx.h>
#include "osa.h"
#include "common.h"
#include "HardwareController.h"
#include "SPI.h"
#include "Uart.h"
#include "LTC_DAC.h"
#include "MCP23S17.h"



// devine this one to print into COM2 the debug info about the parameters for
// the triggrt timing
//#define TRIG_TIMING_DEBUG_OUT
//#define TRIG_TIMING_DEBUG_OUT_T1
//#define TRIG_TIMING_DEBUG_OUT_IN

//-----------------------------------------------------------------------------------------
// Currently we can have up to 4 banks of parameters.
//
// Each bank has all the settings for both output heads 1 and 2.
//
// We can choose which bank is active by sending a select bank message or
// by selecting the bank sequence.
//
// If we choose the bank sequence then the hardware wil be reprogrammed after
// each trigger
//-----------------------------------------------------------------------------------------
static TBankInfo 		theBankInfo[MAX_NUM_OF_BANKS];

/*
* The status of the output heads after programming
*/
static volatile THeadStatus theHeadStatus[MAX_NUM_OF_HEADS];

//-----------------------------------------------------------------------------------------
// The bank sequence is a circular buffer which we follow when we trigger.
// Once the sequence is set we will program the hardware with the parameters for the bank
// in index 0. Then after the trigger we will increment the position and reprogram the hardware
// from the bank referred by index 1, etc.
//
// After we reach the end of the sequence, we will roll over and start from the begining
//
// The sequence can be reset by a message or by an IO *****TBD*****
//-----------------------------------------------------------------------------------------
volatile static unsigned char 	BankSequence[512];	// the sequence of IDs of the banks
volatile static unsigned short 	BankSequencePos = 0;	// the current position in the sequence
volatile static unsigned short 	BankSequenceEnd = 0;	// the position AFTER the last valid element, can be used as a sequence length
volatile static unsigned short 	BankLastUsedId  = 1;	// the last used bank ID, we will check if ID is the same no need to reprogram the hardware


// The flags from the flash that control some hardware stuff
static unsigned long TheFlashFlags;


/**
 * The trigger counter, just for displaying purpose
 */
static volatile unsigned long theTriggerCounter1 = 0;
static volatile unsigned long theTriggerCounter2 = 0;

/**
 * The missing trigger counter, just for displaying purpose
 */
static volatile unsigned long theTriggerMissedCounter1 = 0;
static volatile unsigned long theTriggerMissedCounter2 = 0;

static volatile unsigned long theInterruptCounter = 0;

/**
 * The reprogramming trigger counter, just for displaying
 * purpose
 */
static volatile unsigned long theTriggerMissedWhileReprogrammingCounter1 = 0;
static volatile unsigned long theTriggerMissedWhileReprogrammingCounter2 = 0;


/**
 * In case as described by TTimerFlags we do not use PWM module
 * these are the timer counters that we must respect for
 * triggering
 *
 * For every timer tick we will decrement the counters and when
 * they rich 0 we will do the light triggering
 */
typedef struct tag_TDelayInfo
{
	volatile unsigned long	m_delayTimer;	// in case if we are not using PWM module
	volatile unsigned long	m_widthTimer;	// these 2 fileds will have delay time in Timer1 ticks
	volatile unsigned char	m_flags;		// one of the flags from TTimerFlags
} TDelayInfo;

// the initial calculated delay info
// when we program the current scanner head we
//will put the calculated values here
static TDelayInfo	TheDelayInfo[MAX_NUM_OF_HEADS];

// the working copy of delay info
// this info modified in interrupts
// and at the end of the cycle it is copied from TheDelayInfo
static volatile TDelayInfo	TheDelayInfoWorking[MAX_NUM_OF_HEADS];


/**
 * Trigger pins:
 * 	Trigger 1:	RD5/67/CN14 Input  _LATD3/OC4/63 - output
 * 	Trigger 2:	RD6/68/CN15	Input  _LATD2/OC3/62 - output
 */


void setPulseInfoOutput1(TBankInfo* pInfo);
void setPulseInfoOutput2(TBankInfo* pInfo);

//extern unsigned long TheFlashFlags;


//-----------------------------------------------------------------------------------------
// section that describes the trigger 1 behaviour

static volatile unsigned char Trig1_IN_Enabled;		// The flag that enables trig IN signal
													// it will be set to disabled during any re-programming operations

static volatile unsigned char Trig1_IN_Last;		// the initail state of trig1

static volatile unsigned char Trig1_Timer2_Enabled; // if we need to start timer 2 when we receive trigger 1
//static volatile unsigned char Trig1_Timer2_Edge;	// on which edge to trigger the timer

static volatile unsigned char Trig1_Timer3_Enabled;	// if we need to start timer 3 when we receive trigger 1
//static volatile unsigned char Trig1_Timer3_Edge;	// on which edge to start the timer


//-----------------------------------------------------------------------------------------
// section that describes the trigger 2 behaviour

static volatile unsigned char Trig2_IN_Enabled;		// The flag that enables trig IN signal
									// it will be set to disabled during any re-programming operations

static volatile unsigned char Trig2_IN_Last;

static volatile unsigned char Trig2_Timer2_Enabled; // if we need to start timer 2 when we receive trigger 2
//static volatile unsigned char Trig2_Timer2_Edge;	// on which edge to trigger the timer

static volatile unsigned char Trig2_Timer3_Enabled;	// if we need to start timer 3 when we receive trigger 2
//static volatile unsigned char Trig2_Timer3_Edge;	// on which edge to start the timer


static volatile unsigned char AdvanceToTheNextBank = 1;	// flag that is set when we nned to advance to the next bank



/**
 * getTimeInTicks() converts microcesonds to the system timer
 * tick counts
 *
 * Note that the function itself takes between 1060 to 1076
 * ticks to do the conversion
 *
 * and it takes 34 cycles to do the lookup
 */
//-----------------------------------------------------------------------------------------
// this table is for CPU running at 14MHz
static const unsigned short theDelayLookupTable[80] __attribute__ ((space(auto_psv))) =
{
	0,	  14,  29,  44,  58,  73,  88, 103, 117, 132,
	147, 162, 176, 191, 206, 221, 235, 250, 265, 280,
	294, 309, 324, 339, 353, 368, 383, 398, 412, 427,
	442, 456, 471, 486, 501, 515, 530, 545, 560, 574,
	589, 604, 619, 633, 648, 663, 678, 692, 707, 722,
	737, 751, 766, 781, 796, 810, 825, 840, 854, 869,
	884, 899, 913, 928, 943, 958, 972, 987,1002,1017,
   1031,1046,1061,1076,1090,1105,1120,1135,1149,1164,
};

static inline unsigned long getTimeInTicks(unsigned long aTimeInMicrosec)
{

	if (aTimeInMicrosec < (sizeof(theDelayLookupTable)/ sizeof(theDelayLookupTable[0]))) {

		// we need to multiply the return value by 2 in case if we use 30MHz
		#ifdef USE_30_MHZ
		return (theDelayLookupTable[aTimeInMicrosec]<<1);
		#else
		return theDelayLookupTable[aTimeInMicrosec];
		#endif
	}

	return ((unsigned long)((long double)(((long double)(A_FOSC_/1000000.0)) * ((long double)aTimeInMicrosec)))) ;
	//	return ((unsigned long)((long double)((long double)(A_FOSC_)/((long double)1000000.0)) * (aTimeInMicrosec))) ;
}

static inline unsigned long getTimeInTicksPre(long double aTimeInMicrosec, long double timerPreScaler)
{
	return ((unsigned long)((long double)(((long double)(A_FOSC_/1000000.0)) * (aTimeInMicrosec)) / timerPreScaler)) ;
//	return ((unsigned long)((long double)(((long double)A_FOSC_)/((long double)1000000.0) * (aTimeInMicrosec)) / timerPreScaler)) ;
}


#define TRIG_OUTA_PIC_TRIS	_TRISD3
#define TRIG_OUTA_PIC 		_LATD3

#define TRIG_OUTB_PIC_TRIS	_TRISD2
#define TRIG_OUTB_PIC 		_LATD2


#define TRIG_INA_CPLD_TRIS 	_TRISD5
#define TRIG_INA_CPLD 		_RD5

#define TRIG_INB_CPLD_TRIS 	_TRISD6
#define TRIG_INB_CPLD 		_RD6

#define	OUTA_CONFIG 		OC4CONbits
#define	OUTA_TIME_START 	OC4R
#define	OUTA_TIME_STOP 		OC4RS

#define	OUTB_CONFIG 		OC3CONbits
#define	OUTB_TIME_START 	OC3R
#define	OUTB_TIME_STOP 		OC3RS


/**
 * When 0 then it is AC or trigger mode, when 1 it is DC mode
 */
#define AC_DC_H1			_LATF0
#define AC_DC_H2			_LATF1



/**
 * The trigger edge selection. The signal is sent to CPLD to
 * perform the edge detection.
 *
 * 0 - Raising Edge
 * 1 - Falling Edge
 */
#define TRIGGER1_EDGE_TRIS	_TRISE5
#define TRIGGER1_EDGE		_LATE5

#define TRIGGER2_EDGE_TRIS	_TRISE6
#define TRIGGER2_EDGE		_LATE6

/**
 * stops execution by the delay specified
 * since it involves getTimeInTicks() function we need to
 * differintiate for the speed
 *
 * For the delays less than 3 usec we use hard coded values from
 * getTimeInTicks() function
 *
 * For larger delays we need to subtract the time it takes to
 * call the getTimeInTicks() function
 *
*/
//-----------------------------------------------------------------------------------------
void delay_us(unsigned long aTimeInMicrosec)
{
	static unsigned long i;

	//  case of 0usec delay
	if (aTimeInMicrosec == 0) {
		Nop();
		Nop();
		return;
	}

	//  case of 1usec delay
	if (aTimeInMicrosec == 1) {
		#ifdef USE_30_MHZ
			for (i = 28; i > 0; --i) {
				Nop();
			}
		#else
			for (i = 14; i > 0; --i) {
				Nop();
			}
		#endif
		return;
	}

	//  case of 2usec delay
	if (aTimeInMicrosec == 2) {
		#ifdef USE_30_MHZ
			for (i = 58; i > 0; --i) {
				Nop();
			}
		#else
			for (i = 29; i > 0; --i) {
				Nop();
			}
		#endif
	   	return;
	}

	//  case of 3usec or more delay
	// we need to subtract 33 clock cycles it takes to call the getTimeInTicks()
	// if we use the lookup table
	// otherwise the calculations can take around 1060 clock cycles
	if (aTimeInMicrosec < (sizeof(theDelayLookupTable)/ sizeof(theDelayLookupTable[0]))) {
		for (i = (getTimeInTicks(aTimeInMicrosec) - 34); i > 0; --i) {
			Nop();
		}
	} else{
		for (i = (getTimeInTicks(aTimeInMicrosec) - 1060); i > 0; --i) {
			Nop();
		}
	}
}


// prototype function
void programOutputHead(TBankInfo* pInfo, unsigned char anOutput);


//-----------------------------------------------------------------------------------------
// Initializes the Output Head data (constructor)
//
// Sets all members to 0, and sets the head ID to the id provided
//-----------------------------------------------------------------------------------------
void initOutputHeadData(TBankHeadData* pData, unsigned char anId, unsigned char aBankId)
{
	// store the ID
	pData->m_outputId = anId;   	// 1 - head 1, 2 - Head 2
    pData->m_bankId = aBankId;		// 1, 2, 3, 4

    pData->m_voltage = 0;			// 0 - 100 Volts
    pData->m_powerChanel1 = 0; 		// 0 - 100 00% with fixed decimal point at 2 digits
    pData->m_powerChanel2 = 0; 		// for example the power of 35.23% will be sent as 3523
    pData->m_powerChanel3 = 0; 		// the power of 99.00% will be sent as 9900
    pData->m_powerChanel4 = 0; 		// the power of 100.00% will be sent as 10000
    pData->m_strobeDelay = 10;		// the delay of outcoming light strobe in microseconds
    pData->m_strobeWidth = 10;		// the duration of outcoming light strobe in microseconds
    pData->m_triggerEdge = 0;		// the edge of incoming trigger to start counting, 0 - raising, 1 - falling, 2 - DC mode
    pData->m_triggerId = (anId == 1? TriggerID1 : TriggerID2); // the ID of the trigger this output head will trigger on. 1 - Trigger 1, 2 - Trigger 2, 3 - Trigger 1 or 2
    pData->m_chanelAmplifier = 1;	 // the amplification value 1-5
}


//-----------------------------------------------------------------------------------------
// Initializes all 4 bank info structures.
//-----------------------------------------------------------------------------------------
void InitAllBankInfo()
{
	unsigned short i,j;
	for (i = 1; i <= MAX_NUM_OF_BANKS; ++i) {
		theBankInfo[i - 1].m_id = i;
		for (j = 1; j <= MAX_NUM_OF_HEADS; ++j) {
			initOutputHeadData(&(theBankInfo[i - 1].m_output[j-1]), j, i);
		}
	}

	for (i = 1; i <= MAX_NUM_OF_HEADS; ++i) {
		theHeadStatus[i-1].m_statusChanel1 = 0;
		theHeadStatus[i-1].m_statusChanel2 = 0;
		theHeadStatus[i-1].m_statusChanel3 = 0;
		theHeadStatus[i-1].m_statusChanel4 = 0;

		theHeadStatus[i-1].m_trigCountChanel1.m_low = theHeadStatus[i-1].m_trigCountChanel1.m_hi = 0;	// life time trigger count
		theHeadStatus[i-1].m_trigCountChanel2.m_low = theHeadStatus[i-1].m_trigCountChanel2.m_hi = 0;	// life time trigger count
		theHeadStatus[i-1].m_trigCountChanel3.m_low = theHeadStatus[i-1].m_trigCountChanel3.m_hi = 0;	// life time trigger count
		theHeadStatus[i-1].m_trigCountChanel4.m_low = theHeadStatus[i-1].m_trigCountChanel4.m_hi = 0;	// life time trigger count

		theHeadStatus[i-1].m_onTimeChanel1.m_low = theHeadStatus[i-1].m_onTimeChanel1.m_hi = 0;	// ON time for the light
		theHeadStatus[i-1].m_onTimeChanel2.m_low = theHeadStatus[i-1].m_onTimeChanel2.m_hi = 0;	// ON time for the light
		theHeadStatus[i-1].m_onTimeChanel3.m_low = theHeadStatus[i-1].m_onTimeChanel3.m_hi = 0;	// ON time for the light
		theHeadStatus[i-1].m_onTimeChanel4.m_low = theHeadStatus[i-1].m_onTimeChanel4.m_hi = 0;	// ON time for the light

		TheDelayInfo[i-1].m_delayTimer  = 0;
		TheDelayInfo[i-1].m_widthTimer  = 0;
		TheDelayInfo[i-1].m_flags	  	= 0;

		TheDelayInfoWorking[i-1].m_delayTimer  = 0;
		TheDelayInfoWorking[i-1].m_widthTimer  = 0;
		TheDelayInfoWorking[i-1].m_flags	   = 0;
	}
}

//-----------------------------------------------------------------------------------------
// initalizes the PIN for trigger 1
// It uses CN14 input located at RD5 input (TRIG_INA_CPLD)
// It also enables the interrupt on input change
//-----------------------------------------------------------------------------------------
void initTrigger1()
{
	_CNIE = 0;				// global disable interrupts on pin change

	TRIG_INA_CPLD_TRIS = 1;	// enable RD5 as input TRIG_INA_CPLD is shared with CN14
	_CNIF  = 0;				// clear CN notification flag
	_CN14IE = 1;			// enable interrupt on the pin status change
	_CNIF  = 0;				// clear CN notification flag again
	_CN14PUE = 0;			// enable pullup for this pin (optional step only if external pullup is not present)
							//
	IPC3bits.CNIP = 0x01;	// assign the priority level 1.1 is high, 7 is low, 0 disabled
	Trig1_IN_Last = TRIG_INA_CPLD;// read the current status of input
	_CNIE = 1;				// global enable interrupts on pin change

	Trig1_Timer2_Enabled = 0; 	// if we need to start timer 2 when we receive trigger 1
	Trig1_Timer3_Enabled = 0;	// if we need to start timer 3 when we receive trigger 1

	//Setup trigger 1 output (pin: TRIG_OUTA_PIC)
	TRIG_OUTA_PIC_TRIS = 0;	// set the port as output
	TRIG_OUTA_PIC = 0;		// clear latched trigger 1 output data

	T2CONbits.TON = 0;		// disable the timer 2

	//AdvanceToTheNextBank = 0; //TODO check needed?

	TRIGGER1_EDGE = 0;  	// program the trigger edge selection as output
	TRIGGER1_EDGE_TRIS = 0;

	Trig1_IN_Enabled = 1;	// trigger 1 input is now enabled

	theTriggerCounter1 = 0;	// reset the trigger counter
}

//-----------------------------------------------------------------------------------------
// initalizes the PIN for trigger 2
// It uses CN15 input located at RD6 input
// It also enables the interrupt on input change
//-----------------------------------------------------------------------------------------
void initTrigger2()
{
	_CNIE = 0;				// global disable interrupts on pin change
							//
	TRIG_INB_CPLD_TRIS = 1;	// enable RD6 as input TRIG_INB_CPLD_TRIS is shared with CN15
	//--//--//--//
	#if 0
	_CN15IE = 1;			// enable interrupt on the pin status change
	#endif
	_CNIF  = 0;				// clear CN notification flag
	_CN15PUE = 0;			// enable pullup for this pin (optional step only if external pullup is not present)

	IPC3bits.CNIP = 0x01;	// assign the priority level 1.1 is high, 7 is low, 0 disabled
	Trig2_IN_Last = TRIG_INB_CPLD;// read the current status of input
	_CNIE = 1;				// global enable interrupts on pin change

	Trig2_Timer2_Enabled = 0; 	// if we need to start timer 2 when we receive trigger 2
	Trig2_Timer3_Enabled = 0;	// if we need to start timer 3 when we receive trigger 2

	//Setup trigger 2 output (pin: TRIG_OUTB_PIC)
	TRIG_OUTB_PIC_TRIS = 0;	// set the port as output
	TRIG_OUTB_PIC = 0;		// clear latched trigger 2 output data

	T3CONbits.TON = 0;		// disable the timer 3

	TRIGGER2_EDGE = 0;  	// program the trigger edge selection as output
	TRIGGER2_EDGE_TRIS = 0;

	Trig2_IN_Enabled = 1;	// the trigger 2 input is now enabled

	theTriggerCounter2 = 0;	// reset the trigger counter
}


/**
 * Program the hardware with the bank info supplied
*/
//-----------------------------------------------------------------------------------------
void programBank(TBankInfo* aBankInfo)
{
	static unsigned char old_Trig1_IN_Enabled;
	static unsigned char old_Trig2_IN_Enabled;
	static unsigned short period = 0;
	static unsigned char  yield = 0;
	static TBankInfo*     pBankInfo = 0;

	old_Trig1_IN_Enabled = Trig1_IN_Enabled;
	old_Trig2_IN_Enabled = Trig2_IN_Enabled;
	pBankInfo = aBankInfo;

	// wait until the trigger timers OFF
	period = 0;
	yield = 0;
	while (T2CONbits.TON || T3CONbits.TON || TRIG_OUTA_PIC || TRIG_OUTB_PIC || (TheDelayInfoWorking[HEAD1_IDX].m_flags & tfTriggerON) != 0 || (TheDelayInfoWorking[HEAD2_IDX].m_flags & tfTriggerON) != 0) {

		ClrWdt();

		if (++period > 10000) {
			if (T2CONbits.TON)
				DbgOut("T2CONbits.TON=ON\r\n");

			if (T3CONbits.TON)
				DbgOut("T3CONbits.TON=ON\r\n");

			if (TRIG_OUTA_PIC)
				DbgOut("TRIG_OUTA_PIC=ON\r\n");

			if (TRIG_OUTB_PIC)
				DbgOut("TRIG_OUTB_PIC=ON\r\n");

			if ((TheDelayInfoWorking[HEAD1_IDX].m_flags & tfTriggerON) != 0){
				DbgOut("[HEAD1_IDX].m_flags=tfTriggerON\r\n");
			}
			if ((TheDelayInfoWorking[HEAD2_IDX].m_flags & tfTriggerON) != 0){
				DbgOut("[HEAD2_IDX].m_flags=tfTriggerON\r\n");
			}

			period = 0;
		}
	}

	// Disable trigger inputs temporarily, will be restored later
	Trig1_IN_Enabled = 0;
	Trig2_IN_Enabled = 0;


	#if 0
	int iii;
	for (iii = 1; iii < 100; ++iii) {
		int tm1 = TMR1;
		int t = getTimeInTicks(iii);
		int tm2 = TMR1;

		DbgOut("getTimeInTicks tm1=");
		DbgOutInt(tm1);
		DbgOut("  tm2=");
		DbgOutInt(tm2);
		DbgOut("  iii=");
		DbgOutInt(iii);
		DbgOut("  t=");
		DbgOutInt(t);
		DbgOut("  diff=");
		DbgOutInt(tm2-tm1);
		DbgOut("\r\n");
	}
	#endif


	_T2IE = 0;				// disable timer 2 interrupt
	_T3IE = 0;				// disable timer 3 interrupt
	T2CONbits.TON = 0;		// disable timer 2
	T3CONbits.TON = 0;		// disable timer 3
	TMR2 = 0;				// reset the counter for timer 2
	TMR3 = 0;				// reset the counter for timer 3
	OUTA_CONFIG.OCM = 0x0;	// disable pulse generator, OUTA
	OUTB_CONFIG.OCM = 0x0;	// disable pulse generator, OUTB


	DbgOut("Programming bank = ");
	DbgOutInt(pBankInfo->m_id);
	DbgOut("\r\n");

	//remember the bank ID to not reprogram the hardware next time if the bank doesn't change
	BankLastUsedId = pBankInfo->m_id;

	programOutputHead(pBankInfo, 1);

	programOutputHead(pBankInfo, 2);

	T2CONbits.TON = 0;		// disable timer 2
	TMR2=0;					// reset the counter for timer 2
	T3CONbits.TON = 0;		// disable timer 3
	TMR3=0;					// reset the counter for timer 3

	OUTA_CONFIG.OCTSEL = 0;	// use timer 2 as a source for trigger 1 output
	OUTB_CONFIG.OCTSEL = 1;	// use timer 3 as a source for trigger 2 output

	_T2IE = 0;				// disable timer 2 interrupt
	_T3IE = 0;				// disable timer 3 interrupt

	// set the timer 2 period as the maximum of two ending times
	// at the end of the period the timer interrupt will fire
	// in that interrupt we will reenable OC module
//		PR2 = OUTA_TIME_STOP > OUTB_TIME_STOP ? OUTA_TIME_STOP : OUTB_TIME_STOP;
	PR2 = OUTA_TIME_STOP;
	PR3 = OUTB_TIME_STOP;

	// make the timer 2 period little bit longer
	// we will re-program the hardware if required on timer 2 interrupt
	PR2 = PR2 + 2;

	// make the timer 3 period little bit longer
	// we will re-program the hardware if required on timer 3 interrupt
	PR3 = PR3 + 2;

	// output head 1 is always attached to timer 2
	// so check if we need to trigger it on TriggerID1
	Trig1_Timer2_Enabled = (pBankInfo->m_output[HEAD1_IDX].m_triggerId & TriggerID1) && pBankInfo->m_output[HEAD1_IDX].m_strobeWidth != 0;
	TRIGGER1_EDGE    	  = pBankInfo->m_output[HEAD1_IDX].m_triggerEdge == TriggerRaising ? 0 : 1;		// the status of the pin when we should fire the timer

	// output head 2 always attached to timer 3
	// so check if we need to trigger it on TriggerID1
	Trig1_Timer3_Enabled = (pBankInfo->m_output[HEAD2_IDX].m_triggerId & TriggerID1) && pBankInfo->m_output[HEAD2_IDX].m_strobeWidth != 0;
	TRIGGER2_EDGE    = pBankInfo->m_output[HEAD2_IDX].m_triggerEdge == TriggerRaising ? 0 : 1;		// the status of the pin when we should fire the timer


	// output head 1 is always attached to timer 2
	// so check if we need to trigger it on TriggerID2 this time
	Trig2_Timer2_Enabled = (pBankInfo->m_output[HEAD1_IDX].m_triggerId & TriggerID2) && pBankInfo->m_output[HEAD1_IDX].m_strobeWidth != 0;
	TRIGGER1_EDGE    = pBankInfo->m_output[HEAD1_IDX].m_triggerEdge == TriggerRaising ? 0 : 1;		// the status of the pin when we should fire the timer

	// output head 2 is always attached to timer 3
	// so check if we need to trigger it on TriggerID2 this time
	Trig2_Timer3_Enabled = (pBankInfo->m_output[HEAD2_IDX].m_triggerId & TriggerID2) && pBankInfo->m_output[HEAD2_IDX].m_strobeWidth != 0;
	TRIGGER2_EDGE    = pBankInfo->m_output[HEAD2_IDX].m_triggerEdge == TriggerRaising ? 0 : 1;		// the status of the pin when we should fire the timer


	#if 0
	// so if we need to trigger head 1, then enable interrupt
	if (Trig1_Timer2_Enabled || Trig2_Timer2_Enabled) {
		_T2IF = 0;				// reset interrupt flag for timer 2
		_T2IE = 1;				// enable timer 2 interrupt
	}

	// so if we need to trigger head 2, then enable interrupt
	if (Trig1_Timer3_Enabled || Trig2_Timer3_Enabled) {
		_T3IF = 0;				// reset interrupt flag for timer 3
		_T3IE = 1;				// enable timer 3 interrupt
	}

	// Check whether we have to use trigger input 1
	// since both heads use the same trigger IDs we just check head0
	if ((pBankInfo->m_output[HEAD1_IDX].m_triggerId & TriggerID1) != 0) {
		Trig1_Timer2_Edge    = pBankInfo->m_output[HEAD1_IDX].m_triggerEdge == 0 ? 1 : 0;		// the status of the pin when we should fire the timer
		Trig1_Timer2_Enabled = 1;
	}
	else {
		Trig1_Timer2_Enabled = 0;
	}

	// Check whether we have to use trigger input 2
	if ((pBankInfo->m_output[HEAD1_IDX].m_triggerId & TriggerID2) != 0) {
		Trig2_Timer2_Edge    = pBankInfo->m_output[HEAD1_IDX].m_triggerEdge == 0 ? 1 : 0;		// the status of the pin when we should fire the timer
		Trig2_Timer2_Enabled = 1;
	}
	else {
		Trig2_Timer2_Enabled = 0;
	}
	#endif

	Trig1_IN_Enabled = old_Trig1_IN_Enabled;
	Trig2_IN_Enabled = old_Trig2_IN_Enabled;

	DbgOut(" Trig1_IN_Enabled = ");
	DbgOutInt(Trig1_IN_Enabled);
	DbgOut(" Trig2_IN_Enabled = ");
	DbgOutInt(Trig2_IN_Enabled);

	DbgOut(" programBank=done\r\n");
}


/**
 * Programs the output head hardware with the parameters
 * specified
 *
 * Inputs:
 *
 * TBankInfo* pInfo - pointer to the bank info structure as a
 * source
 *
 * unsigned char anOutput - Output ID: 1,2
 *
*/
//-----------------------------------------------------------------------------------------
void programOutputHead(TBankInfo* pInfo, unsigned char anOutputId)
{
	unsigned char noChannelPower;


	if (anOutputId < 1) anOutputId = 1;
	if (anOutputId > 2) anOutputId = 2;

	DbgOut("Programming output head...\r\n");

	DbgOut("Init pulse module\r\n");


	//
	// in case if we are using AC mode
	// then we must switch to AC before we program DACs
	// since the DAC current can be very high for AC mode
	//
	if (pInfo->m_output[anOutputId-1].m_triggerEdge != TriggerDC){
		if (anOutputId == 1)
			AC_DC_H1 = 0; // 1 - DC mode is on , 0 - DCMode is off
		else
			AC_DC_H2 = 0; // 1 - DC mode is on , 0 - DCMode is off
	}


	if (anOutputId == 1){
		setPulseInfoOutput1(pInfo);
	} else{
		setPulseInfoOutput2(pInfo);
	}

	DbgOut("Init head power V{");
	DbgOutInt(pInfo->m_output[anOutputId-1].m_voltage);
	DbgOut("}{");
	DbgOutInt(pInfo->m_output[anOutputId-1].m_powerChanel1);
	DbgOut("}{");
	DbgOutInt(pInfo->m_output[anOutputId-1].m_powerChanel2);
	DbgOut("}{");
	DbgOutInt(pInfo->m_output[anOutputId-1].m_powerChanel3);
	DbgOut("}{");
	DbgOutInt(pInfo->m_output[anOutputId-1].m_powerChanel4);
	DbgOut("}\r\n");


	noChannelPower = 	pInfo->m_output[anOutputId-1].m_powerChanel1 == 0 &&
						pInfo->m_output[anOutputId-1].m_powerChanel2 == 0 &&
						pInfo->m_output[anOutputId-1].m_powerChanel3 == 0 &&
						pInfo->m_output[anOutputId-1].m_powerChanel4 == 0;

	setCurrentDACValue(anOutputId, 1, pInfo->m_output[anOutputId-1].m_powerChanel1, pInfo->m_output[anOutputId-1].m_chanelAmplifier);
	setCurrentDACValue(anOutputId, 2, pInfo->m_output[anOutputId-1].m_powerChanel2, pInfo->m_output[anOutputId-1].m_chanelAmplifier);
	setCurrentDACValue(anOutputId, 3, pInfo->m_output[anOutputId-1].m_powerChanel3, pInfo->m_output[anOutputId-1].m_chanelAmplifier);
	setCurrentDACValue(anOutputId, 4, pInfo->m_output[anOutputId-1].m_powerChanel4, pInfo->m_output[anOutputId-1].m_chanelAmplifier);

	// enable or disable VOUT for each chanel separately
	// in case if the chanel power is set to 0
	MCP23S17EnableVout(anOutputId, pInfo->m_output[anOutputId-1].m_powerChanel1, pInfo->m_output[anOutputId-1].m_powerChanel2, pInfo->m_output[anOutputId-1].m_powerChanel3, pInfo->m_output[anOutputId-1].m_powerChanel4);

	// in case if all the current values are set to 0 set voltage to 0 as well
	setVoltageDACValue(anOutputId, noChannelPower ? 0 : pInfo->m_output[anOutputId-1].m_voltage);

	//
	// in case if we are using DC mode
	// then we must switch to DC after we program DACs
	// since the DAC current must be low for DC mode
	//
	if (pInfo->m_output[anOutputId-1].m_triggerEdge == TriggerDC){
		if (anOutputId == 1)
			AC_DC_H1 = 1; // 1 - DC mode is on , 0 - DCMode is off
		else
			AC_DC_H2 = 1; // 1 - DC mode is on , 0 - DCMode is off
	}

	if (pInfo->m_output[anOutputId-1].m_powerChanel1 > 5000)
		theHeadStatus[anOutputId-1].m_statusChanel1 = 1;
	else
		theHeadStatus[anOutputId-1].m_statusChanel1 = 0;

	if (pInfo->m_output[anOutputId-1].m_powerChanel2 > 5000)
		theHeadStatus[anOutputId-1].m_statusChanel2 = 1;
	else
		theHeadStatus[anOutputId-1].m_statusChanel2 = 0;

	if (pInfo->m_output[anOutputId-1].m_powerChanel3 > 5000)
		theHeadStatus[anOutputId-1].m_statusChanel3 = 1;
	else
		theHeadStatus[anOutputId-1].m_statusChanel3 = 0;

	if (pInfo->m_output[anOutputId-1].m_powerChanel4 > 5000)
		theHeadStatus[anOutputId-1].m_statusChanel4 = 1;
	else
		theHeadStatus[anOutputId-1].m_statusChanel4 = 0;

//	DbgOut("Enable pulse module\r\n");
//	if (anOutputId == 1)
//		OUTA_CONFIG.OCM = 0x4;	// enable single pulse mode
//	else
//		OUTB_CONFIG.OCM = 0x4;	// enable single pulse mode


	DbgOut("Program output head done\r\n");
}


//-----------------------------------------------------------------------------------------
void resetAllDACs()
{
	short i = 0;
	for (i = 1; i <= 8; ++i) {
		setCurrentDACValue(1/*Head ID*/, i/*Chanel ID*/, 0, 0);  //Reset current for head 1 on all channels
		setCurrentDACValue(2/*Head ID*/, i/*Chanel ID*/, 0, 0);  //Reset current for head 2 on all channels
	}

	setVoltageDACValue(1/*Head ID*/, 0); //Reset voltage for head 1
	setVoltageDACValue(2/*Head ID*/, 0); //Reset voltage for head 2
}



//-----------------------------------------------------------------------------------------
void processNextSequence(void)
{
	static unsigned char old_Trig1_IN_Enabled;
	static unsigned char old_Trig2_IN_Enabled;

	//if ((TheFlashFlags & fifUseBanks) == 0) {
	//	AdvanceToTheNextBank = 0;	//multi-banks not enabled
	//	return ;
	//}

	if (AdvanceToTheNextBank == 0)
		return ; 					//has already processed current bank, not ready for the next bank in sequence yet

	AdvanceToTheNextBank = 0;		// reset the flag

	// temporarily disable the input trigger
	old_Trig1_IN_Enabled = Trig1_IN_Enabled;
	old_Trig2_IN_Enabled = Trig2_IN_Enabled;
	Trig1_IN_Enabled = 0;
	Trig2_IN_Enabled = 0;

	DbgOut("processNextSequence");
	DbgOut("\r\n");

	//advance to the next bank in the bank sequence (auto-wrap to the beginning)
	if (++BankSequencePos > BankSequenceEnd)
		BankSequencePos = 0;

	//if the next bank in the sequence is different from the previous, then
	//reprogram the hardware with new bank settings
	if (BankSequence[BankSequencePos] != BankLastUsedId) {
		programBank(&theBankInfo[ BankSequence[BankSequencePos] ]);
	}

	//restore the input trigger settings
	Trig1_IN_Enabled = old_Trig1_IN_Enabled;
	Trig2_IN_Enabled = old_Trig2_IN_Enabled;
	DbgOut(" Trig1_IN_Enabled = ");
	DbgOutInt(Trig1_IN_Enabled);
	DbgOut(" Trig2_IN_Enabled = ");
	DbgOutInt(Trig2_IN_Enabled);
	DbgOut("\r\n");
}

//-----------------------------------------------------------------------------------------
unsigned char setActiveBank(unsigned short anActiveBank, unsigned char bForcePrograming)
{
	if (anActiveBank < 1) anActiveBank = 1;
	if (anActiveBank > MAX_NUM_OF_BANKS) anActiveBank = MAX_NUM_OF_BANKS;

	if (BankLastUsedId != anActiveBank || bForcePrograming) {
		BankLastUsedId = anActiveBank;
		#if 1
		DbgOut("setActiveBank(");
		DbgOutInt(anActiveBank);
		DbgOut(",");
		DbgOutInt(bForcePrograming);
		DbgOut(")\r\n");
		#endif

		programBank(&theBankInfo[anActiveBank-1]);
		return 1;
	}
	return 0;
}

//-----------------------------------------------------------------------------------------
unsigned short getActiveBank()
{
	return BankLastUsedId;
}


//-----------------------------------------------------------------------------------------
unsigned long getConfigFlags()
{
	return TheFlashFlags;
}

//-----------------------------------------------------------------------------------------
void setBankSequenceAt(unsigned short aBankId, unsigned short anIndex)
{
	if (anIndex < sizeof(BankSequence)/sizeof(BankSequence[0])) {
		if (aBankId < 1) aBankId = 1;
		if (aBankId > 4) aBankId = 4;
		BankSequence[anIndex] = aBankId;
	}
}

//-----------------------------------------------------------------------------------------
unsigned short getBankSequenceAt(unsigned short anIndex)
{
	if (anIndex < sizeof(BankSequence)/sizeof(BankSequence[0])) {
		return BankSequence[anIndex];
	}
	return(0);
}

//-----------------------------------------------------------------------------------------
void setBankSequenceLength(unsigned short aLength)
{
	if (aLength > (sizeof(BankSequence) / sizeof(BankSequence[0]))) aLength = (sizeof(BankSequence) / sizeof(BankSequence[0]));
	BankSequenceEnd = aLength;
}

//-----------------------------------------------------------------------------------------
unsigned short getBankSequenceLength()
{
	return BankSequenceEnd;
}


//-----------------------------------------------------------------------------------------
TBankInfo* getBankInfo(unsigned short aBankId)
{
	if (aBankId < 1) aBankId = 1;
	if (aBankId > MAX_NUM_OF_BANKS) aBankId = MAX_NUM_OF_BANKS;
	return &theBankInfo[aBankId-1];
}


//-----------------------------------------------------------------------------------------
volatile THeadStatus* getHeadStatus(unsigned char anOutputId)
{
	if (anOutputId < 1) anOutputId = 1;
	if (anOutputId > 2) anOutputId = 2;

	return &theHeadStatus[anOutputId-1];
}

//-----------------------------------------------------------------------------------------
unsigned short getCurrentBankSequencePosition()
{
	return (BankSequencePos);
}

//-----------------------------------------------------------------------------------------
unsigned long getTriggerCounter1()
{
	return theTriggerCounter1;
}

//-----------------------------------------------------------------------------------------
unsigned long getTriggerCounter2()
{
	return theTriggerCounter2;
}

//-----------------------------------------------------------------------------------------
unsigned long getMissingTriggerCounter1()
{
	return theTriggerMissedCounter1;
}
//-----------------------------------------------------------------------------------------
unsigned long getMissingTriggerCounter2()
{
	return theTriggerMissedCounter2;
}

//-----------------------------------------------------------------------------------------
unsigned long getInterruptTriggerCounter()
{
	return theInterruptCounter;
}




/**
 * setPulseInfoOutput1() programs PWM module to generate 1
 * signle pulse with the delay and the duration specified
 *
 * The PWM module will use Timer 2 as a source
 *
 * Since it is a hardware module it will automaticaly control
 * the output (TRIG_OUTA_PIC) pin level
 *
*/
//-----------------------------------------------------------------------------------------
void setPulseInfoOutput1(TBankInfo* pInfo)
{
	static unsigned long interruptDelay;

	unsigned long theDelayTimeInTicks = 0;
	unsigned long theWidthTimeInTicks = 0;

	if (pInfo->m_output[HEAD1_IDX].m_triggerEdge == TriggerDC)
		return;


	OUTA_CONFIG.OCM = 0;	//disable pulse module initially
	T2CONbits.TON = 0;		//disable the timer

	TRIG_OUTA_PIC = 0; 		//set pulse output to low for output A
	OUTA_CONFIG.OCTSEL = 0;	// use timer 2 as a source

	theDelayTimeInTicks = getTimeInTicks(pInfo->m_output[HEAD1_IDX].m_strobeDelay);
	theWidthTimeInTicks = getTimeInTicks(pInfo->m_output[HEAD1_IDX].m_strobeWidth);

	TheDelayInfo[HEAD1_IDX].m_delayTimer  = 0;
	TheDelayInfo[HEAD1_IDX].m_widthTimer  = 0;
	TheDelayInfo[HEAD1_IDX].m_flags	  	  = 0;

	// we need to be carefull with integer overflow if both numbers are big then adding them will yield wrong results
	if (theDelayTimeInTicks <= 0xFFF0 && theWidthTimeInTicks <= 0xFFF0 && (theDelayTimeInTicks + theWidthTimeInTicks) <= 0xFFF0) {
		// ok in this case everything fits into PWM timer
		TheDelayInfo[HEAD1_IDX].m_flags = tfPWMOnly;
	} else if (theDelayTimeInTicks >= 0xFFF0 && theWidthTimeInTicks <= 0xFFF0) {
		// use T1 as delay and PWM for width only
		TheDelayInfo[HEAD1_IDX].m_flags = tfDelayAsT1 | tfWidthAsPWM;
	} else if (theDelayTimeInTicks <= 0xFFF0 && theWidthTimeInTicks >= 0xFFF0){
		// delay use T2T3 and for width use T1
		TheDelayInfo[HEAD1_IDX].m_flags = tfDelayAsT2T3 | tfWidthAsT1;
	} else if (theDelayTimeInTicks <= 0xFFF0 && theWidthTimeInTicks <= 0xFFF0 && (theDelayTimeInTicks + theWidthTimeInTicks) > 0xFFF0){
		// in this case we use T2/T3 for delay first and then use PWM for width
		TheDelayInfo[HEAD1_IDX].m_flags = tfDelayAsT2T3 | tfWidthAsPWM;
	} else {
		// last resort use T1 for everything
		TheDelayInfo[HEAD1_IDX].m_flags = tfDelayAsT1 | tfWidthAsT1;
	}



	// the fixed delay between we detect the trigger and ISR enables the timer
	// for  7.37Mhz crystal in 16X PLL mode
	// right now for head 1 it is 3us
	interruptDelay = getTimeInTicks(3);

	DbgOut(" setPulseInfoOutput1 width=");
	DbgOutInt(pInfo->m_output[HEAD1_IDX].m_strobeWidth);
	DbgOut(" {");
	DbgOutInt(theWidthTimeInTicks);
	DbgOut("} delay=");
	DbgOutInt(pInfo->m_output[HEAD1_IDX].m_strobeDelay);
	DbgOut(" {");
	DbgOutInt(theDelayTimeInTicks);
	DbgOut("} flags=");
	DbgOutInt(TheDelayInfo[HEAD1_IDX].m_flags);
	DbgOut(" ISR delay=");
	DbgOutInt(interruptDelay);


	T2CONbits.TCKPS=0x0;	// use prescaler *1:1* 1:8 1:64 1:256

	T2CONbits.TCS=0;		// use instruction clock
	TMR2=0;					// reset the counter for timer 2
	_T2IF = 0;				// reset interrupt flag for timer 2
							//

	// do everything in PWM module
	if ((TheDelayInfo[HEAD1_IDX].m_flags & tfPWMOnly) != 0) {

		OUTA_CONFIG.OCM = 0x0;		// disable single pulse mode

		// pulse time START
		OUTA_TIME_START =  theDelayTimeInTicks;

		// check if the delay is more than ISR delay, then adjust it for the ISR delay
		if (OUTA_TIME_START >= interruptDelay)
			OUTA_TIME_START -= interruptDelay;
		else
			OUTA_TIME_START = 1;

		if (OUTA_TIME_START == 0)			// the minimum delay is 1 timer tick
			OUTA_TIME_START = 1;

		DbgOut(" PWM OUTA_TIME_START=");
		DbgOutInt(OUTA_TIME_START);


		// pulse time STOP
		OUTA_TIME_STOP= OUTA_TIME_START + theWidthTimeInTicks;

		if (OUTA_TIME_STOP < 2)			// the minimum width is 1 timer tick after the delay
			OUTA_TIME_STOP = 2;

		DbgOut(" PWM OUTA_TIME_STOP=");
		DbgOutInt(OUTA_TIME_STOP);

		_T2IE = 1;				// enable timer 2 interrupt
		PR2 = OUTA_TIME_STOP+2;	// now program the Timer 2 duration so as soon as we program that the timer 2 is ready
								// we will start the timer in the input interrupt, when we detect TRIG_INA
								//
								// TODO: check! in this interrupt we will reenable this OC module
	}

	if ((TheDelayInfo[HEAD1_IDX].m_flags & tfWidthAsPWM) != 0) {

		// pulse time START
		OUTA_TIME_START =  1;

		// check if the delay is more than ISR delay, then adjust it for the ISR delay
		if (OUTA_TIME_START >= interruptDelay)
			OUTA_TIME_START -= interruptDelay;
		else
			OUTA_TIME_START = 1;

		if (OUTA_TIME_START == 0)			// the minimum delay is 1 timer tick
			OUTA_TIME_START = 1;

		DbgOut(" WIDTH OUTA_TIME_START=");
		DbgOutInt(OUTA_TIME_START);


		// pulse time STOP
		OUTA_TIME_STOP= OUTA_TIME_START + theWidthTimeInTicks;

		if (OUTA_TIME_STOP < 2)			// the minimum width is 1 timer tick after the delay
			OUTA_TIME_STOP = 2;

		DbgOut(" WIDTH OUTA_TIME_STOP=");
		DbgOutInt(OUTA_TIME_STOP);
	}


	if ((TheDelayInfo[HEAD1_IDX].m_flags & tfDelayAsT1) != 0) {
		TheDelayInfo[HEAD1_IDX].m_delayTimer = getTimeInTicksPre(pInfo->m_output[HEAD1_IDX].m_strobeDelay, TIMER1_PERIOD);	// get the number of ticks per timer 1 counter

		if (TheDelayInfo[HEAD1_IDX].m_delayTimer == 0)
			TheDelayInfo[HEAD1_IDX].m_delayTimer = 1;
		DbgOut(" T1 Delay=");
		DbgOutInt(TheDelayInfo[HEAD1_IDX].m_delayTimer);
	}

	if ((TheDelayInfo[HEAD1_IDX].m_flags & tfWidthAsT1) != 0) {
		TheDelayInfo[HEAD1_IDX].m_widthTimer = getTimeInTicksPre(pInfo->m_output[HEAD1_IDX].m_strobeWidth, TIMER1_PERIOD);	// get the number of ticks per timer 1 counter

		if (TheDelayInfo[HEAD1_IDX].m_widthTimer == 0)
			TheDelayInfo[HEAD1_IDX].m_widthTimer = 1;

		DbgOut(" T1 Width=");
		DbgOutInt(TheDelayInfo[HEAD1_IDX].m_widthTimer);
	}

	if ((TheDelayInfo[HEAD1_IDX].m_flags & tfDelayAsT2T3) != 0) {
		_T2IE = 1;						// enable timer 2 interrupt
										//
		PR2 = theDelayTimeInTicks;		// now program the Timer 2 as a delay

		TheDelayInfo[HEAD1_IDX].m_delayTimer = theDelayTimeInTicks;

		DbgOut(" T2/T3 Delay=");
		DbgOutInt(theDelayTimeInTicks);
	}

	// copy the flags from configuration to working
	TheDelayInfoWorking[HEAD1_IDX] = TheDelayInfo[HEAD1_IDX];


	DbgOut("\r\n");
}


/**
 * setPulseInfoOutput2() programs PWM module to generate 1
 * signle pulse with the delay and the duration specified
 *
 * The PWM module will use Timer 2 as a source
 *
 * Since it is a hardware module it will automaticaly control
 * the output (TRIG_OUTA_PIC) pin level
 *
 *
*/
//-----------------------------------------------------------------------------------------
void setPulseInfoOutput2(TBankInfo* pInfo)
{
	static unsigned long interruptDelay;
	unsigned long theDelayTimeInTicks = 0;
	unsigned long theWidthTimeInTicks = 0;

	if (pInfo->m_output[HEAD2_IDX].m_triggerEdge == TriggerDC)
		return;

	OUTB_CONFIG.OCM = 0;	//disable pulse module initially
	T3CONbits.TON = 0;		//disable the timer

	TRIG_OUTB_PIC = 0; 		//set pulse output to low for output B
	OUTB_CONFIG.OCTSEL = 1;	// use timer 3 as a source

	theDelayTimeInTicks = getTimeInTicks(pInfo->m_output[HEAD2_IDX].m_strobeDelay);
	theWidthTimeInTicks = getTimeInTicks(pInfo->m_output[HEAD2_IDX].m_strobeWidth);

	TheDelayInfo[HEAD2_IDX].m_delayTimer  = 0;
	TheDelayInfo[HEAD2_IDX].m_widthTimer  = 0;
	TheDelayInfo[HEAD2_IDX].m_flags	  	  = 0;

	// we need to be carefull with integer overflow if both numbers are big then adding them will yield wrong results
	if (theDelayTimeInTicks <= 0xFFF0 && theWidthTimeInTicks <= 0xFFF0 && (theDelayTimeInTicks + theWidthTimeInTicks) <= 0xFFF0) {
		// ok in this case everything fits into PWM timer
		TheDelayInfo[HEAD2_IDX].m_flags = tfPWMOnly;
	} else if (theDelayTimeInTicks >= 0xFFF0 && theWidthTimeInTicks <= 0xFFF0) {
		// use T1 as delay and PWM for width only
		TheDelayInfo[HEAD2_IDX].m_flags = tfDelayAsT1 | tfWidthAsPWM;
	} else if (theDelayTimeInTicks <= 0xFFF0 && theWidthTimeInTicks >= 0xFFF0){
		// delay use T2T3 and for width use T1
		TheDelayInfo[HEAD2_IDX].m_flags = tfDelayAsT2T3 | tfWidthAsT1;
	} else if (theDelayTimeInTicks <= 0xFFF0 && theWidthTimeInTicks <= 0xFFF0 && (theDelayTimeInTicks + theWidthTimeInTicks) > 0xFFF0){
		// in this case we use T2/T3 for delay first and then use PWM for width
		TheDelayInfo[HEAD2_IDX].m_flags = tfDelayAsT2T3 | tfWidthAsPWM;
	} else {
		// last resort use T1 for everything
		TheDelayInfo[HEAD2_IDX].m_flags = tfDelayAsT1 | tfWidthAsT1;
	}



	// the fixed delay between we detect the trigger and ISR enables the timer
	// for  7.37Mhz crystal in 16X PLL mode
	// right now for head 2 it is 6us
	interruptDelay = getTimeInTicks(6);

	DbgOut(" setPulseInfoOutput2 width=");
	DbgOutInt(pInfo->m_output[HEAD2_IDX].m_strobeWidth);
	DbgOut(" {");
	DbgOutInt(theWidthTimeInTicks);
	DbgOut("} delay=");
	DbgOutInt(pInfo->m_output[HEAD2_IDX].m_strobeDelay);
	DbgOut(" {");
	DbgOutInt(theDelayTimeInTicks);
	DbgOut("} flags=");
	DbgOutInt(TheDelayInfo[HEAD2_IDX].m_flags);
	DbgOut(" ISR delay=");
	DbgOutInt(interruptDelay);


	T3CONbits.TCKPS=0x0;	// use prescaler *1:1* 1:8 1:64 1:256

	T3CONbits.TCS=0;		// use instruction clock
	TMR3=0;					// reset the counter for timer 3
	_T3IF = 0;				// reset interrupt flag for timer 3
							//

	// do everything in PWM module
	if ((TheDelayInfo[HEAD2_IDX].m_flags & tfPWMOnly) != 0) {

		OUTB_CONFIG.OCM = 0x0;		// disable single pulse mode

		// pulse time START
		OUTB_TIME_START =  theDelayTimeInTicks;

		// check if the delay is more than ISR delay, then adjust it for the ISR delay
		if (OUTB_TIME_START >= interruptDelay)
			OUTB_TIME_START -= interruptDelay;
		else
			OUTB_TIME_START = 1;

		if (OUTB_TIME_START == 0)			// the minimum delay is 1 timer tick
			OUTB_TIME_START = 1;

		DbgOut(" PWM OUTB_TIME_START=");
		DbgOutInt(OUTB_TIME_START);


		// pulse time STOP
		OUTB_TIME_STOP= OUTB_TIME_START + theWidthTimeInTicks;

		if (OUTB_TIME_STOP < 2)			// the minimum width is 1 timer tick after the delay
			OUTB_TIME_STOP = 2;

		DbgOut(" PWM OUTB_TIME_STOP=");
		DbgOutInt(OUTB_TIME_STOP);

		_T3IE = 1;				// enable timer 2 interrupt
		PR3 = OUTB_TIME_STOP+2;	// now program the Timer 2 duration so as soon as we program that the timer 2 is ready
								// we will start the timer in the input interrupt, when we detect TRIG_INA
								//
								// TODO: check! in this interrupt we will reenable this OC module
	}

	if ((TheDelayInfo[HEAD2_IDX].m_flags & tfWidthAsPWM) != 0) {

		// pulse time START
		OUTB_TIME_START =  1;

		// check if the delay is more than ISR delay, then adjust it for the ISR delay
		if (OUTB_TIME_START >= interruptDelay)
			OUTB_TIME_START -= interruptDelay;
		else
			OUTB_TIME_START = 1;

		if (OUTB_TIME_START == 0)			// the minimum delay is 1 timer tick
			OUTB_TIME_START = 1;

		DbgOut(" WIDTH OUTB_TIME_START=");
		DbgOutInt(OUTB_TIME_START);


		// pulse time STOP
		OUTB_TIME_STOP= OUTB_TIME_START + theWidthTimeInTicks;

		if (OUTB_TIME_STOP < 2)			// the minimum width is 1 timer tick after the delay
			OUTB_TIME_STOP = 2;

		DbgOut(" WIDTH OUTB_TIME_STOP=");
		DbgOutInt(OUTB_TIME_STOP);
	}


	if ((TheDelayInfo[HEAD2_IDX].m_flags & tfDelayAsT1) != 0) {
		TheDelayInfo[HEAD2_IDX].m_delayTimer = getTimeInTicksPre(pInfo->m_output[HEAD2_IDX].m_strobeDelay, TIMER1_PERIOD);	// get the number of ticks per timer 1 counter

		if (TheDelayInfo[HEAD2_IDX].m_delayTimer == 0)
			TheDelayInfo[HEAD2_IDX].m_delayTimer = 1;
		DbgOut(" T1 Delay=");
		DbgOutInt(TheDelayInfo[HEAD2_IDX].m_delayTimer);
	}

	if ((TheDelayInfo[HEAD2_IDX].m_flags & tfWidthAsT1) != 0) {
		TheDelayInfo[HEAD2_IDX].m_widthTimer = getTimeInTicksPre(pInfo->m_output[HEAD2_IDX].m_strobeWidth, TIMER1_PERIOD);	// get the number of ticks per timer 1 counter

		if (TheDelayInfo[HEAD2_IDX].m_widthTimer == 0)
			TheDelayInfo[HEAD2_IDX].m_widthTimer = 1;

		DbgOut(" T1 Width=");
		DbgOutInt(TheDelayInfo[HEAD2_IDX].m_widthTimer);
	}

	if ((TheDelayInfo[HEAD2_IDX].m_flags & tfDelayAsT2T3) != 0) {
		_T3IE = 1;						// enable timer 3 interrupt
										//
		PR3 = theDelayTimeInTicks;		// now program the Timer 3 as a delay

		TheDelayInfo[HEAD2_IDX].m_delayTimer = theDelayTimeInTicks;

		DbgOut(" T2/T3 Delay=");
		DbgOutInt(theDelayTimeInTicks);
	}

	// copy the flags from configuration to working
	TheDelayInfoWorking[HEAD2_IDX] = TheDelayInfo[HEAD2_IDX];



	DbgOut("\r\n");
}


typedef enum tag_TTrigCounter
{
	tcIncTrig1		= 0x01,	// increment trigger counter 1
	tcIncTrig2	   	= 0x02,	// increment trigger counter 2
	tcIncMissTrig1	= 0x04,	// increment missing trigger counter 1
	tcIncMissTrig2	= 0x08,	// increment missing trigger counter 1
} TTrigCounter;


/**
 * processTriggerIn() processes the trigger input and starts the
 * corresponding timer module
*/
//-----------------------------------------------------------------------------------------
void processTriggerIn(unsigned char aTrigInACpld, unsigned char aTrigInBCpld)
{
	static volatile unsigned char theTrigInACpld;
	static volatile unsigned char theTrigInBCpld;


	static volatile unsigned char bIncTrig;


	theTrigInACpld  = aTrigInACpld;	// Current status of IO
	theTrigInBCpld  = aTrigInBCpld;

	bIncTrig = 0;

	#ifdef TRIG_TIMING_DEBUG_OUT_IN1
		DbgOut("TriggerIn A:");
		DbgOutInt(theTrigInACpld);
		DbgOut(" LA:");
		DbgOutInt(Trig1_IN_Last);
		DbgOut(" PA:");
		DbgOutInt(aTrigInACpld);

		DbgOut(" B:");
		DbgOutInt(theTrigInBCpld);
		DbgOut(" LB:");
		DbgOutInt(Trig2_IN_Last);
		DbgOut(" PB:");
		DbgOutInt(aTrigInBCpld);

		DbgOut("\r\n");
	#endif

	// trig 1 is fired
	if (Trig1_IN_Last != theTrigInACpld && Trig1_IN_Enabled) {

		#ifdef TRIG_TIMING_DEBUG_OUT_IN
			DbgOut("Trig 1, OK::");
		#endif

		// only if the edge is the same and the timer is not running
		if (Trig1_Timer2_Enabled && theTrigInACpld == 1) {

			if ((TheDelayInfoWorking[HEAD1_IDX].m_flags & tfTriggerON) == 0) {
				#ifdef TRIG_TIMING_DEBUG_OUT_IN
					DbgOut(" H1=OK:: ");
				#endif

				// when we get the trigger here we can process only 3 possible conditions
				// first is if we use PWM module for both delay and width
				// second if we use T2/T3 for the delay
				// and third if we use T1 for the delay
				// the width controlling part will be done in interrupts for T1 or T2/T3
				//
				// the order of IF statements is important to minimise the delays

				if ((TheDelayInfo[HEAD1_IDX].m_flags & tfPWMOnly) != 0) {
					OUTA_CONFIG.OCM = 0x4;		// enable single pulse mode
					_T2IE = 1;					// enable timer 2 interrupt
					TMR2 = 0;					// reset the counter for timer 2

					#ifdef TRIG_TIMING_DEBUG_OUT_IN
						DbgOutInt(PR2);
						DbgOut("=PR2: tfPWMOnly:: ");
					#endif

					// the trigger has been processed
					// do not re-program hardware until it is cleared
					// set it before we enable the timer
					TheDelayInfoWorking[HEAD1_IDX].m_flags |= tfTriggerON;

					T2CONbits.TON = 1;		// turn ON timer
				}
				else if ((TheDelayInfoWorking[HEAD1_IDX].m_flags & tfDelayAsT2T3) != 0) {
					OUTA_CONFIG.OCM = 0;		// disable single pulse mode

					_T2IE = 1;					// enable timer 2 interrupt
					TMR2 = 0;					// reset the counter for timer 2
					PR2 = TheDelayInfo[HEAD1_IDX].m_delayTimer;	// now program the Timer 2 duration for the delay

					#ifdef TRIG_TIMING_DEBUG_OUT_IN
						DbgOutInt(PR2);
						DbgOut("=PR2: tfDelayAsT2T3:: ");
					#endif

					// the trigger has been processed
					// do not re-program hardware until it is cleared
					// set it before we enable the timer
					TheDelayInfoWorking[HEAD1_IDX].m_flags |= tfTriggerON;

					T2CONbits.TON = 1;			// turn ON timer just a timer

				} else if ((TheDelayInfoWorking[HEAD1_IDX].m_flags & tfDelayAsT1) != 0) {
					OUTA_CONFIG.OCM = 0;	// disable single pulse mode
					T2CONbits.TON = 0;		// disable timer

					// the trigger has been processed
					// do not re-program hardware until it is cleared
					// set it before we enable the timer
					TheDelayInfoWorking[HEAD1_IDX].m_flags |= tfTriggerON;

					// set the flag that the delay counter is ON and active NOW
					// so every Timer1 period we will process the delay
					TheDelayInfoWorking[HEAD1_IDX].m_flags |= tfDelayAsT1ON;

					#ifdef TRIG_TIMING_DEBUG_OUT_IN
						DbgOut("=T1: tfDelayAsT1:: ");
					#endif
				}

				bIncTrig |= tcIncTrig1;
			} else {
				bIncTrig |= tcIncMissTrig1;

				#ifdef TRIG_TIMING_DEBUG_OUT_IN
					DbgOut(" H1=MISS:: ");
				#endif
			}
		}


		// only if the edge is the same and the timer is not running
		if (Trig1_Timer3_Enabled && theTrigInACpld == 1) {
			if ((TheDelayInfoWorking[HEAD2_IDX].m_flags & tfTriggerON) == 0) {
				#ifdef TRIG_TIMING_DEBUG_OUT_IN
					DbgOut(" H2=OK:: ");
				#endif

				// when we get the trigger here we can process only 3 possible conditions
				// first is if we use PWM module for both delay and width
				// second if we use T2/T3 for the delay
				// and third if we use T1 for the delay
				// the width controlling part will be done in interrupts for T1 or T2/T3
				//
				// the order of IF statements is important to minimise the delays

				if ((TheDelayInfo[HEAD2_IDX].m_flags & tfPWMOnly) != 0) {
					OUTB_CONFIG.OCM = 0x4;		// enable single pulse mode
					_T3IE = 1;					// enable timer 3 interrupt
					TMR3 = 0;					// reset the counter for timer 3

					#ifdef TRIG_TIMING_DEBUG_OUT_IN
						DbgOutInt(PR3);
						DbgOut("=PR3: tfPWMOnly:: ");
					#endif

					// the trigger has been processed
					// do not re-program hardware until it is cleared
					// set it before we enable the timer
					TheDelayInfoWorking[HEAD2_IDX].m_flags |= tfTriggerON;

					T3CONbits.TON = 1;		// turn ON timer
				} else if ((TheDelayInfoWorking[HEAD2_IDX].m_flags & tfDelayAsT2T3) != 0) {
					OUTB_CONFIG.OCM = 0;	// disable single pulse mode

					_T3IE = 1;					// enable timer 3 interrupt
					TMR3 = 0;					// reset the counter for timer 3
					PR3 = TheDelayInfo[HEAD2_IDX].m_delayTimer;	// now program the Timer 3 duration for the delay

					#ifdef TRIG_TIMING_DEBUG_OUT_IN
						DbgOutInt(PR3);
						DbgOut("=PR3: tfDelayAsT2T3:: ");
					#endif

					// the trigger has been processed
					// do not re-program hardware until it is cleared
					// set it before we enable the timer
					TheDelayInfoWorking[HEAD2_IDX].m_flags |= tfTriggerON;

					T3CONbits.TON = 1;			// turn ON timer just a timer

				} else if ((TheDelayInfoWorking[HEAD2_IDX].m_flags & tfDelayAsT1) != 0) {
					OUTB_CONFIG.OCM = 0;	// disable single pulse mode
					T3CONbits.TON = 0;		// disable timer

					// the trigger has been processed
					// do not re-program hardware until it is cleared
					// set it before we enable the timer
					TheDelayInfoWorking[HEAD2_IDX].m_flags |= tfTriggerON;

					// set the flag that the delay counter is ON and active NOW
					// so every Timer1 period we will process the delay
					TheDelayInfoWorking[HEAD2_IDX].m_flags |= tfDelayAsT1ON;

					#ifdef TRIG_TIMING_DEBUG_OUT_IN
						DbgOut(" =T1: tfDelayAsT1:: ");
					#endif
				}

				bIncTrig |= tcIncTrig2;
			} else {
				bIncTrig |= tcIncMissTrig2;
				#ifdef TRIG_TIMING_DEBUG_OUT_IN
					DbgOut(" H2=MISS:: ");
				#endif
			}
		}

	} else if (Trig1_IN_Last == theTrigInACpld && (Trig1_Timer2_Enabled || Trig1_Timer3_Enabled)) {
		bIncTrig |= tcIncMissTrig1;
	}


	// trig 2 is fired
	if (Trig2_IN_Last != theTrigInBCpld  && Trig2_IN_Enabled) {


		#ifdef TRIG_TIMING_DEBUG_OUT_IN
			DbgOut("Trig 2, OK:: ");
		#endif

		// only if the edge is the same and the timer is not running
		if (Trig2_Timer2_Enabled && theTrigInBCpld == 1) {
			if ((TheDelayInfoWorking[HEAD1_IDX].m_flags & tfTriggerON) == 0) {
				#ifdef TRIG_TIMING_DEBUG_OUT_IN
					DbgOut("H1=OK:: ");
				#endif

				// when we get the trigger here we can process only 3 possible conditions
				// first is if we use PWM module for both delay and width
				// second if we use T2/T3 for the delay
				// and third if we use T1 for the delay
				// the width controlling part will be done in interrupts for T1 or T2/T3
				//
				// the order of IF statements is important to minimise the delays

				if ((TheDelayInfo[HEAD1_IDX].m_flags & tfPWMOnly) != 0) {
					OUTA_CONFIG.OCM = 0x4;		// enable single pulse mode
					_T2IE = 1;					// enable timer 2 interrupt
					TMR2 = 0;					// reset the counter for timer 2

					#ifdef TRIG_TIMING_DEBUG_OUT_IN
						DbgOutInt(PR2);
						DbgOut("=PR2: tfPWMOnly:: ");
					#endif

					// the trigger has been processed
					// do not re-program hardware until it is cleared
					// set it before we enable the timer
					TheDelayInfoWorking[HEAD1_IDX].m_flags |= tfTriggerON;

					T2CONbits.TON = 1;		// turn ON timer
				} else if ((TheDelayInfoWorking[HEAD1_IDX].m_flags & tfDelayAsT2T3) != 0) {
					OUTA_CONFIG.OCM = 0;	// disable single pulse mode

					_T2IE = 1;					// enable timer 2 interrupt
					TMR2 = 0;					// reset the counter for timer 2
					PR2 = TheDelayInfo[HEAD1_IDX].m_delayTimer;	// now program the Timer 2 duration for the delay

					#ifdef TRIG_TIMING_DEBUG_OUT_IN
						DbgOutInt(PR2);
						DbgOut("=PR2: tfDelayAsT2T3:: ");
					#endif

					// the trigger has been processed
					// do not re-program hardware until it is cleared
					// set it before we enable the timer
					TheDelayInfoWorking[HEAD1_IDX].m_flags |= tfTriggerON;

					T2CONbits.TON = 1;			// turn ON timer just a timer

				} else if ((TheDelayInfoWorking[HEAD1_IDX].m_flags & tfDelayAsT1) != 0) {
					OUTA_CONFIG.OCM = 0;	// disable single pulse mode
					T2CONbits.TON = 0;		// disable timer

					// the trigger has been processed
					// do not re-program hardware until it is cleared
					// set it before we enable the timer
					TheDelayInfoWorking[HEAD1_IDX].m_flags |= tfTriggerON;

					// set the flag that the delay counter is ON and active NOW
					// so every Timer1 period we will process the delay
					TheDelayInfoWorking[HEAD1_IDX].m_flags |= tfDelayAsT1ON;

					#ifdef TRIG_TIMING_DEBUG_OUT_IN
						DbgOut("=T1: tfDelayAsT1:: ");
					#endif
				}


				bIncTrig |= tcIncTrig1;
			} else {
				bIncTrig |= tcIncMissTrig1;
				#ifdef TRIG_TIMING_DEBUG_OUT_IN
					DbgOut(" H1=MISS:: ");
				#endif
			}
		}

		// only if the edge is the same and the timer is not running
		if (Trig2_Timer3_Enabled && theTrigInBCpld == 1) {
			if ((TheDelayInfoWorking[HEAD2_IDX].m_flags & tfTriggerON) == 0) {
				#ifdef TRIG_TIMING_DEBUG_OUT_IN
					DbgOut("H2=OK:: ");
				#endif

				// when we get the trigger here we can process only 3 possible conditions
				// first is if we use PWM module for both delay and width
				// second if we use T2/T3 for the delay
				// and third if we use T1 for the delay
				// the width controlling part will be done in interrupts for T1 or T2/T3
				//
				// the order of IF statements is important to minimise the delays

				if ((TheDelayInfo[HEAD2_IDX].m_flags & tfPWMOnly) != 0) {
					OUTB_CONFIG.OCM = 0x4;		// enable single pulse mode
					_T3IE = 1;					// enable timer 3 interrupt
					TMR3 = 0;					// reset the counter for timer 3

					#ifdef TRIG_TIMING_DEBUG_OUT_IN
						DbgOutInt(PR3);
						DbgOut("=PR3: tfPWMOnly:: ");
					#endif

					// the trigger has been processed
					// do not re-program hardware until it is cleared
					// set it before we enable the timer
					TheDelayInfoWorking[HEAD2_IDX].m_flags |= tfTriggerON;

					T3CONbits.TON = 1;		// turn ON timer
				} else if ((TheDelayInfoWorking[HEAD2_IDX].m_flags & tfDelayAsT2T3) != 0) {
					OUTB_CONFIG.OCM = 0;	// disable single pulse mode

					_T3IE = 1;					// enable timer 3 interrupt
					TMR3 = 0;					// reset the counter for timer 3
					PR3 = TheDelayInfo[HEAD2_IDX].m_delayTimer;	// now program the Timer 3 duration for the delay

					#ifdef TRIG_TIMING_DEBUG_OUT_IN
						DbgOutInt(PR3);
						DbgOut("=PR3: tfDelayAsT2T3:: ");
					#endif

					// the trigger has been processed
					// do not re-program hardware until it is cleared
					// set it before we enable the timer
					TheDelayInfoWorking[HEAD2_IDX].m_flags |= tfTriggerON;

					T3CONbits.TON = 1;			// turn ON timer just a timer

				} else if ((TheDelayInfoWorking[HEAD2_IDX].m_flags & tfDelayAsT1) != 0) {
					OUTB_CONFIG.OCM = 0;	// disable single pulse mode
					T3CONbits.TON = 0;		// disable timer

					// the trigger has been processed
					// do not re-program hardware until it is cleared
					// set it before we enable the timer
					TheDelayInfoWorking[HEAD2_IDX].m_flags |= tfTriggerON;

					// set the flag that the delay counter is ON and active NOW
					// so every Timer1 period we will process the delay
					TheDelayInfoWorking[HEAD2_IDX].m_flags |= tfDelayAsT1ON;

					#ifdef TRIG_TIMING_DEBUG_OUT_IN
						DbgOut("=T1: tfDelayAsT1:: ");
					#endif
				}

				bIncTrig |= tcIncTrig2;
			} else {
				bIncTrig |= tcIncMissTrig2;
				#ifdef TRIG_TIMING_DEBUG_OUT_IN
					DbgOut(" H2=MISS:: ");
				#endif
			}
		}
	} else if (Trig2_IN_Last == theTrigInBCpld  && (Trig2_Timer2_Enabled || Trig2_Timer3_Enabled))  {
		bIncTrig |= tcIncMissTrig2;
	}


	#ifdef TRIG_TIMING_DEBUG_OUT_IN
		DbgOut("\r\n");
	#endif

	// store the current state of theTrigInACpld
	Trig1_IN_Last = theTrigInACpld;
	Trig2_IN_Last = theTrigInBCpld;

	if ((bIncTrig & tcIncTrig1) != 0) ++theTriggerCounter1;	// increment the trigger counter
	if ((bIncTrig & tcIncTrig2) != 0) ++theTriggerCounter2;	// increment the trigger counter
	if ((bIncTrig & tcIncMissTrig1) != 0) ++theTriggerMissedCounter1;	// increment the missing trigger counter
	if ((bIncTrig & tcIncMissTrig2) != 0) ++theTriggerMissedCounter2;	// increment the missing trigger counter
	++theInterruptCounter;
}


/**
 * The procedure processes the Timer 1 intervals
 *
 * When we get timer 1 event we will check the trigger flags and
 * decrement our delay and width counters
 *
 * When the counters reach 0 then we will do the trigger
 * processing
*/
//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
// Here is our lovely interrupt function that Timer 4 period change
//-----------------------------------------------------------------------------------------
void _ISR __attribute__ ((auto_psv)) _T4Interrupt (void)
{
	// increment timer counetr
	OS_Timer();

	// if the delay timer is enabled for head 1
	if (TheDelayInfoWorking[HEAD1_IDX].m_delayTimer != 0 && (TheDelayInfoWorking[HEAD1_IDX].m_flags & tfDelayAsT1ON) != 0) {
		if (--TheDelayInfoWorking[HEAD1_IDX].m_delayTimer == 0) {
			if ((TheDelayInfoWorking[HEAD1_IDX].m_flags & tfWidthAsT1) != 0) {
				TRIG_OUTA_PIC = 1;											// manualy raise the light trigger pin

				// reset the flag so we do not get here twice
				TheDelayInfoWorking[HEAD1_IDX].m_flags &= ~tfDelayAsT1ON;

				// start the timer 1 counting for width
				TheDelayInfoWorking[HEAD1_IDX].m_flags |= tfWidthAsT1ON;

				#ifdef TRIG_TIMING_DEBUG_OUT_T1
					DbgOut("T1: tfDelayAsT1ON::tfWidthAsT1\r\n");
				#endif


			}

			// now check if we need to control width through PWM
			if ((TheDelayInfoWorking[HEAD1_IDX].m_flags & tfWidthAsPWM) != 0) {
				OUTA_CONFIG.OCM = 0x4;	// enable single pulse mode

				_T2IE = 1;				// enable timer 2 interrupt
				PR2 = OUTA_TIME_STOP+2;	// now program the Timer 2 duration so as soon as we program that the timer 2 is ready
										//
				#ifdef TRIG_TIMING_DEBUG_OUT_T1
					DbgOutInt(PR2);
					DbgOut(" T1: tfDelayAsT1ON::tfWidthAsPWM\r\n");
				#endif

				// reset the flag so we do not get here twice
				TheDelayInfoWorking[HEAD1_IDX].m_flags &= ~tfDelayAsT1ON;

				T2CONbits.TON = 1;		// enable the timer 2
			}

		}
	}

	// if the width timer is enabled for head 1
	if (TheDelayInfoWorking[HEAD1_IDX].m_widthTimer != 0 && (TheDelayInfoWorking[HEAD1_IDX].m_flags & tfWidthAsT1ON) != 0) {
		if (--TheDelayInfoWorking[HEAD1_IDX].m_widthTimer == 0) {
			TRIG_OUTA_PIC = 0;		// manualy lower down the light trigger pin

			// restore the flags and get ready for next trigger
			TheDelayInfoWorking[HEAD1_IDX] = TheDelayInfo[HEAD1_IDX];
			#ifdef TRIG_TIMING_DEBUG_OUT_T1
				DbgOut("T1: tfWidthAsT1OFF\r\n");
			#endif
		}
	}


	// if the delay timer is enabled for head 2
	if (TheDelayInfoWorking[HEAD2_IDX].m_delayTimer != 0 && (TheDelayInfoWorking[HEAD2_IDX].m_flags & tfDelayAsT1ON) != 0) {
		if (--TheDelayInfoWorking[HEAD2_IDX].m_delayTimer == 0) {
			if ((TheDelayInfoWorking[HEAD2_IDX].m_flags & tfWidthAsT1) != 0) {
				TRIG_OUTB_PIC = 1;								// manualy raise the light trigger pin

				// reset the flag so we do not get here twice
				TheDelayInfoWorking[HEAD2_IDX].m_flags &= ~tfDelayAsT1ON;

				// start the timer 1 counting for width
				TheDelayInfoWorking[HEAD2_IDX].m_flags |= tfWidthAsT1ON;

				#ifdef TRIG_TIMING_DEBUG_OUT_T1
					DbgOut("T1H2: tfDelayAsT1ON::tfWidthAsT1\r\n");
				#endif
			}

			// now check if we need to control width through PWM
			if ((TheDelayInfoWorking[HEAD2_IDX].m_flags & tfWidthAsPWM) != 0) {
				OUTB_CONFIG.OCM = 0x4;	// enable single pulse mode

				_T3IE = 1;				// enable timer 3 interrupt
				PR3 = OUTB_TIME_STOP+2;	// now program the Timer 3 duration so as soon as we program that the timer 2 is ready
										//
				#ifdef TRIG_TIMING_DEBUG_OUT_T1
					DbgOutInt(PR3);
					DbgOut(" T1H2: tfDelayAsT1ON::tfWidthAsPWM\r\n");
				#endif

				// reset the flag so we do not get here twice
				TheDelayInfoWorking[HEAD2_IDX].m_flags &= ~tfDelayAsT1ON;

				T3CONbits.TON = 1;		// enable the timer 3
			}

		}
	}

	// if the width timer is enabled for head 2
	if (TheDelayInfoWorking[HEAD2_IDX].m_widthTimer != 0 && (TheDelayInfoWorking[HEAD2_IDX].m_flags & tfWidthAsT1ON) != 0) {
		if (--TheDelayInfoWorking[HEAD2_IDX].m_widthTimer == 0) {
			TRIG_OUTB_PIC = 0;		// manualy lower down the light trigger pin

			// restore the flags and get ready for next trigger
			TheDelayInfoWorking[HEAD2_IDX] = TheDelayInfo[HEAD2_IDX];
			#ifdef TRIG_TIMING_DEBUG_OUT_T1
				DbgOut("T1H2: tfWidthAsT1OFF\r\n");
			#endif
		}
	}

	_T4IF = 0;				// important to clear this bit
}

//-----------------------------------------------------------------------------------------
// Here is our lovely interrupt function that processes trigger input change
//-----------------------------------------------------------------------------------------
void _ISR __attribute__ ((auto_psv)) _CNInterrupt(void)
{
	#if 0
	static unsigned int i;
	// for debugging purpose
	_LATE6 = 1;
	for (i = 100; i > 0; --i) {
		Nop();
	}
	_LATE6 = 0;

	#endif

	processTriggerIn(TRIG_INA_CPLD, TRIG_INB_CPLD);

	_CNIF = 0;	// important to clear this bit
}

//-----------------------------------------------------------------------------------------
// Here is our lovely interrupt function that Time 2 period change
//-----------------------------------------------------------------------------------------
void _ISR __attribute__ ((auto_psv)) _T2Interrupt (void)
{
	T2CONbits.TON = 0;		// disable the timer 2

	OUTA_CONFIG.OCM = 0;	// disable single pulse mode

	TMR2 = 0;				// reset the counter for timer 2
	TRIG_OUTA_PIC = 0;		// make sure it is LOW

	// the first portion if we control the delay through the Timer 2/3
	// in this case we still need to do the width portion
	if ((TheDelayInfoWorking[HEAD1_IDX].m_flags & tfDelayAsT2T3) != 0) {

		// reset the flag so we do not get here twice
		TheDelayInfoWorking[HEAD1_IDX].m_flags &= ~tfDelayAsT2T3;

		// now check if we need to control width through PWM
		if ((TheDelayInfoWorking[HEAD1_IDX].m_flags & tfWidthAsPWM) != 0) {
			OUTA_CONFIG.OCM = 0x4;	// enable single pulse mode

			TMR2 = 0;					// reset the counter for timer 2
			PR2 = OUTA_TIME_STOP+2;		// now program the Timer 2 duration for the PWM width

			#ifdef TRIG_TIMING_DEBUG_OUT
				DbgOutInt(PR2);
				DbgOut(" T2: tfDelayAsT2T3::tfWidthAsPWM\r\n");
			#endif

			T2CONbits.TON = 1;			// turn ON timer just a timer
		}

		// we do the width control trough the timer 1
		else if ((TheDelayInfoWorking[HEAD1_IDX].m_flags & tfWidthAsT1) != 0) {
			TRIG_OUTA_PIC = 1;						// raise the trigger pin

			#ifdef TRIG_TIMING_DEBUG_OUT
				DbgOutInt(TheDelayInfoWorking[HEAD1_IDX].m_widthTimer);
				DbgOut(" T2: tfDelayAsT2T3::tfWidthAsT1\r\n");
			#endif
			TheDelayInfoWorking[HEAD1_IDX].m_flags |= tfWidthAsT1ON;	// start the timer 1 counting

		}

	}

	// we do PWM mode only both the delay and width already processed
	// restore the flags and get ready for next trigger
	else if ((TheDelayInfoWorking[HEAD1_IDX].m_flags & tfPWMOnly) != 0) {
		TheDelayInfoWorking[HEAD1_IDX] = TheDelayInfo[HEAD1_IDX];
		OUTA_CONFIG.OCM = 0x0;	// disable single pulse mode

		#ifdef TRIG_TIMING_DEBUG_OUT
			DbgOut("T2: tfPWMOnly\r\n");
		#endif
	}

	// we finished PWM mode for width, then restore the flags
	else if ((TheDelayInfoWorking[HEAD1_IDX].m_flags & tfWidthAsPWM) != 0) {
		TheDelayInfoWorking[HEAD1_IDX] = TheDelayInfo[HEAD1_IDX];
		#ifdef TRIG_TIMING_DEBUG_OUT
			DbgOut("T2: tfWidthAsPWM\r\n");
		#endif
	}
	#ifdef TRIG_TIMING_DEBUG_OUT
		DbgOut("T2\r\n");
	#endif


//	AdvanceToTheNextBank = 1;

//	outputString(&UartDbg, "A=0\n");

	_T2IF = 0;	// important to clear this bit
}

//-----------------------------------------------------------------------------------------
// Here is our lovely interrupt function that Time 3 period change
//-----------------------------------------------------------------------------------------
void _ISR __attribute__ ((auto_psv)) _T3Interrupt (void)
{
	T3CONbits.TON = 0;		// disable the timer 3

	OUTB_CONFIG.OCM = 0;	// disable single pulse mode

	TMR3 = 0;				// reset the counter for timer 3
	TRIG_OUTB_PIC = 0;		// make sure it is LOW

	// the first portion if we control the delay through the Timer 2/3
	// in this case we still need to do the width portion
	if ((TheDelayInfoWorking[HEAD2_IDX].m_flags & tfDelayAsT2T3) != 0) {

		// reset the flag so we do not get here twice
		TheDelayInfoWorking[HEAD2_IDX].m_flags &= ~tfDelayAsT2T3;

		// now check if we need to control width through PWM
		if ((TheDelayInfoWorking[HEAD2_IDX].m_flags & tfWidthAsPWM) != 0) {
			OUTB_CONFIG.OCM = 0x4;	// enable single pulse mode

			TMR3 = 0;					// reset the counter for timer 3
			PR3 = OUTB_TIME_STOP+2;		// now program the Timer 3 duration for the PWM width

			#ifdef TRIG_TIMING_DEBUG_OUT
				DbgOutInt(PR3);
				DbgOut(" T3H2: tfDelayAsT2T3::tfWidthAsPWM\r\n");
			#endif

			T3CONbits.TON = 1;			// turn ON timer just a timer
		}

		// we do the width control trough the timer 1
		else if ((TheDelayInfoWorking[HEAD2_IDX].m_flags & tfWidthAsT1) != 0) {
			TRIG_OUTB_PIC = 1;						// raise the trigger pin

			#ifdef TRIG_TIMING_DEBUG_OUT
				DbgOutInt(TheDelayInfoWorking[HEAD2_IDX].m_widthTimer);
				DbgOut(" T3H2: tfDelayAsT2T3::tfWidthAsT1\r\n");
			#endif
			TheDelayInfoWorking[HEAD2_IDX].m_flags |= tfWidthAsT1ON;	// start the timer 1 counting

		}

	}

	// we do PWM mode only both the delay and width already processed
	// restore the flags and get ready for next trigger
	else if ((TheDelayInfoWorking[HEAD2_IDX].m_flags & tfPWMOnly) != 0) {
		TheDelayInfoWorking[HEAD2_IDX] = TheDelayInfo[HEAD2_IDX];
		OUTB_CONFIG.OCM = 0x0;	// disable single pulse mode

		#ifdef TRIG_TIMING_DEBUG_OUT
			DbgOut("T3H2: tfPWMOnly\r\n");
		#endif
	}

	// we finished PWM mode for width, then restore the flags
	else if ((TheDelayInfoWorking[HEAD2_IDX].m_flags & tfWidthAsPWM) != 0) {
		TheDelayInfoWorking[HEAD2_IDX] = TheDelayInfo[HEAD2_IDX];
		#ifdef TRIG_TIMING_DEBUG_OUT
			DbgOut("T3H2: tfWidthAsPWM\r\n");
		#endif
	}
	#ifdef TRIG_TIMING_DEBUG_OUT
		DbgOut("T3\r\n");
	#endif

	_T3IF = 0;				// important to clear this bit
}


//-----------------------------------------------------------------------------------------
// the ID of the trigger this output head will trigger on. 1 - Trigger 1, 2 - Trigger 2, 3 - Trigger 1 or 2

void fireSoftTrigger(unsigned char aTriggerId)
{

#if 0
	DbgOut("fireSoftTrigger \r\n");
	processTriggerIn(0, 0);
	processTriggerIn(1, 1);
#endif

}

