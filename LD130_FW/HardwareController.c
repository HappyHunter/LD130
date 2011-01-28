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
static THeadStatus theHeadStatus[MAX_NUM_OF_HEADS];

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
static unsigned char 	BankSequence[512];	// the sequence of IDs of the banks
static unsigned short 	BankSequencePos = 0;	// the current position in the sequence
static unsigned short 	BankSequenceEnd = 0;	// the position AFTER the last valid element, can be used as a sequence length
static unsigned short 	BankLastUsedId  = 1;	// the last used bank ID, we will check if ID is the same no need to reprogram the hardware


// The flags from the flash that control some hardware stuff
static unsigned long TheFlashFlags;


/**
 * The trigger counter, just for displaying purpose
 */
static unsigned long theTriggerCounter1 = 0;
static unsigned long theTriggerCounter2 = 0;

/**
Trigger pins:
	Trigger 1:	RD5/67/CN14 Input  _LATD3/OC4/63 - output
	Trigger 2:	RD6/68/CN15	Input  _LATD2/OC3/62 - output

There are two options for triggering. First the simplest and commonly used when
both output heads are using the same trigger input and the same trigger edge and
the delay+width are smaller that max timer count of 65535 (with 40Mhz it is 1.638 msec)
In this case we will assign output 1 and output 2 to use the same timer source, timer 2.

So when we receive the trigger either 1 or 2 as specified in the parameters, we will just
enable timer 2 only.

In more complicated cases timer 2 is assigned to the output 1
and timer 3 is assigned to output 2. So if both outputs are set to be triggered by trig 1
the interrupt will first start timer 2 and then timer 3. So there will be slight delay.
*/


void setPulseInfoOutput1(TBankInfo* pInfo);
void setPulseInfoOutput2(TBankInfo* pInfo);

//extern unsigned long TheFlashFlags;


//-----------------------------------------------------------------------------------------
// section that describes the trigger 1 behaviour

unsigned char Trig1_IN_Enabled;		// The flag that enables trig IN signal
									// it will be set to disabled during any re-programming operations

unsigned char Trig1_IN_Last;				// the initail state of trig1

unsigned char Trig1_Timer2_Enabled; // if we need to start timer 2 when we receive trigger 1
unsigned char Trig1_Timer2_Edge;	// on which edge to trigger the timer

unsigned char Trig1_Timer3_Enabled;	// if we need to start timer 3 when we receive trigger 1
unsigned char Trig1_Timer3_Edge;	// on which edge to start the timer


//-----------------------------------------------------------------------------------------
// section that describes the trigger 2 behaviour

unsigned char Trig2_IN_Enabled;		// The flag that enables trig IN signal
									// it will be set to disabled during any re-programming operations

unsigned char Trig2_IN_Last;

unsigned char Trig2_Timer2_Enabled; // if we need to start timer 2 when we receive trigger 2
unsigned char Trig2_Timer2_Edge;	// on which edge to trigger the timer

unsigned char Trig2_Timer3_Enabled;	// if we need to start timer 3 when we receive trigger 2
unsigned char Trig2_Timer3_Edge;	// on which edge to start the timer


unsigned char AdvanceToTheNextBank = 1;	// flag that is set when we nned to advance to the next bank




//-----------------------------------------------------------------------------------------
static unsigned long getTimeInTicks(unsigned long aTimeInMicrosec)
{
	return ((unsigned long)((A_FOSC_/1000000UL) * aTimeInMicrosec)) ;
}

static unsigned long getTimeInTicksPre(unsigned long aTimeInMicrosec, unsigned long timerPreScaler)
{
	return ((unsigned long)((A_FOSC_/1000000UL) * aTimeInMicrosec) / (timerPreScaler ? timerPreScaler : 1)) ;
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
 * stops execution by the delay specified
 * since it involves division operation which takes 18 cycles be
 * carefull on slow speeds
*/
//-----------------------------------------------------------------------------------------
void delay_us(unsigned long aTimeInMicrosec)
{
	static unsigned long i;
	for (i = (getTimeInTicks(aTimeInMicrosec) >> 1); i > 0; --i) {
		Nop();
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
			theBankInfo[i - 1].m_strobeTimerPrescaler[j-1] = 0xffff; //To be computed later from strobeDelay and strobeWidth
		}
	}

	for (i = 1; i <= MAX_NUM_OF_HEADS; ++i) {
		theHeadStatus[i-1].m_statusChanel1 = 0;
		theHeadStatus[i-1].m_statusChanel2 = 0;
		theHeadStatus[i-1].m_statusChanel3 = 0;
		theHeadStatus[i-1].m_statusChanel4 = 0;
	}
}

//-----------------------------------------------------------------------------------------
// initalizes the PIN for trigger 1
// It uses CN14 input located at RD5 input (TRIG_INA_CPLD)
// It also enables the interrupt on input change
//-----------------------------------------------------------------------------------------
void initTrigger1()
{
	TRIG_INA_CPLD_TRIS = 1;	// enable RD5 as input TRIG_INA_CPLD is shared with CN14
	_CNIF  = 0;				// clear CN notification flag
	_CN14IE = 1;			// enable interrupt on the pin status change
	_CNIF  = 0;				// clear CN notification flag again
	_CN14PUE = 1;			// enable pullup for this pin (optional step only if external pullup is not present)
							//
	IPC3bits.CNIP = 0x02;	// assign the priority level 2. 0 is the highest
	Trig1_IN_Last = TRIG_INA_CPLD;// read the current status of input
	_CNIE = 1;				// global enable interrupts on pin change

	Trig1_Timer2_Enabled = 0; 	// if we need to start timer 2 when we receive trigger 1
	Trig1_Timer2_Edge = 0;		// on which edge to trigger the timer

	Trig1_Timer3_Enabled = 0;	// if we need to start timer 3 when we receive trigger 1
	Trig1_Timer3_Edge = 0;		// on which edge to start the timer

	//Setup trigger 1 output (pin: TRIG_OUTA_PIC)
	TRIG_OUTA_PIC_TRIS = 0;	// set the port as output
	TRIG_OUTA_PIC = 0;		// clear latched trigger 1 output data

	T2CONbits.TON = 0;		// disable the timer 2

	//AdvanceToTheNextBank = 0; //TODO check needed?

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
	TRIG_INB_CPLD_TRIS = 1;	// enable RD6 as input TRIG_INB_CPLD_TRIS is shared with CN15
	_CN15IE = 1;			// enable interrupt on the pin status change
	_CNIF  = 0;				// clear CN notification flag
	_CN15PUE = 1;			// enable pullup for this pin (optional step only if external pullup is not present)
	IPC3bits.CNIP = 0x02;	// assign the priority level 2. 0 is the highest
	Trig2_IN_Last = TRIG_INB_CPLD;// read the current status of input
	_CNIE = 1;				// global enable interrupts on pin change

	Trig2_Timer2_Enabled = 0; 	// if we need to start timer 2 when we receive trigger 2
	Trig2_Timer2_Edge = 0;		// on which edge to trigger the timer

	Trig2_Timer3_Enabled = 0;	// if we need to start timer 3 when we receive trigger 2
	Trig2_Timer3_Edge = 0;		// on which edge to start the timer

	//Setup trigger 2 output (pin: TRIG_OUTB_PIC)
	TRIG_OUTB_PIC_TRIS = 0;	// set the port as output
	TRIG_OUTB_PIC = 0;		// clear latched trigger 2 output data

	T3CONbits.TON = 0;		// disable the timer 3

	Trig2_IN_Enabled = 1;	// the trigger 2 input is now enabled

	theTriggerCounter2 = 0;	// reset the trigger counter
}


/**
 * Program the hardware with the bank info supplied
*/
//-----------------------------------------------------------------------------------------
void programBank(TBankInfo* pBankInfo)
{
	unsigned char old_Trig1_IN_Enabled;
	unsigned char old_Trig2_IN_Enabled;
	unsigned char period;

	old_Trig1_IN_Enabled = Trig1_IN_Enabled;
	old_Trig2_IN_Enabled = Trig2_IN_Enabled;


	// wait until the trigger timers OFF
	period = 0;
	while (T2CONbits.TON || T3CONbits.TON || TRIG_OUTA_PIC || TRIG_OUTB_PIC) {
		ClrWdt();
		if ((++period % 30) == 0) {
			if (T2CONbits.TON)
				DbgOut("T2CONbits.TON=ON\r\n");
			if (T3CONbits.TON)
				DbgOut("T3CONbits.TON=ON\r\n");
			if (TRIG_OUTA_PIC)
				DbgOut("TRIG_OUTA_PIC=ON\r\n");
			if (TRIG_OUTB_PIC)
				DbgOut("TRIG_OUTB_PIC=ON\r\n");
		}
	}

	_T2IE = 0;				// disable timer 2 interrupt
	_T3IE = 0;				// disable timer 3 interrupt
	T2CONbits.TON = 0;		// disable timer 2
	T3CONbits.TON = 0;		// disable timer 3
	TMR2 = 0;				// reset the counter for timer 2
	TMR3 = 0;				// reset the counter for timer 3
	OUTA_CONFIG.OCM = 0x0;	// disable pulse generator, OUTA
	OUTB_CONFIG.OCM = 0x0;	// disable pulse generator, OUTB

	//Disable trigger inputs temporarily, will be restored later
	Trig1_IN_Enabled = 0;
	Trig2_IN_Enabled = 0;

	DbgOut("Programming bank = ");
	DbgOutInt(pBankInfo->m_id);
	DbgOut("\r\n");

	//remember the bank ID to not reprogram the hardware next time if the bank doesn't change
	BankLastUsedId = pBankInfo->m_id;

	programOutputHead(pBankInfo, 1);

//	programOutputHead(pBankInfo, 2);

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
	Trig1_Timer2_Enabled = (pBankInfo->m_output[0].m_triggerId & TriggerID1) && pBankInfo->m_output[0].m_strobeWidth != 0;
	Trig1_Timer2_Edge    = pBankInfo->m_output[0].m_triggerEdge == TriggerRaising ? 1 : 0;		// the status of the pin when we should fire the timer

	// output head 2 always attached to timer 3
	// so check if we need to trigger it on TriggerID1
	Trig1_Timer3_Enabled = (pBankInfo->m_output[1].m_triggerId & TriggerID1) && pBankInfo->m_output[1].m_strobeWidth != 0;
	Trig1_Timer3_Edge    = pBankInfo->m_output[1].m_triggerEdge == TriggerRaising ? 1 : 0;		// the status of the pin when we should fire the timer


	// output head 1 is always attached to timer 2
	// so check if we need to trigger it on TriggerID2 this time
	Trig2_Timer2_Enabled = (pBankInfo->m_output[0].m_triggerId & TriggerID2) && pBankInfo->m_output[0].m_strobeWidth != 0;
	Trig2_Timer2_Edge    = pBankInfo->m_output[0].m_triggerEdge == TriggerRaising ? 1 : 0;		// the status of the pin when we should fire the timer

	// output head 2 is always attached to timer 3
	// so check if we need to trigger it on TriggerID2 this time
	Trig2_Timer3_Enabled = (pBankInfo->m_output[1].m_triggerId & TriggerID2) && pBankInfo->m_output[1].m_strobeWidth != 0;
	Trig2_Timer3_Edge    = pBankInfo->m_output[1].m_triggerEdge == TriggerRaising ? 1 : 0;		// the status of the pin when we should fire the timer


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

	#if 0
	// Check whether we have to use trigger input 1
	// since both heads use the same trigger IDs we just check head0
	if ((pBankInfo->m_output[0].m_triggerId & TriggerID1) != 0) {
		Trig1_Timer2_Edge    = pBankInfo->m_output[0].m_triggerEdge == 0 ? 1 : 0;		// the status of the pin when we should fire the timer
		Trig1_Timer2_Enabled = 1;
	}
	else {
		Trig1_Timer2_Enabled = 0;
	}

	// Check whether we have to use trigger input 2
	if ((pBankInfo->m_output[0].m_triggerId & TriggerID2) != 0) {
		Trig2_Timer2_Edge    = pBankInfo->m_output[0].m_triggerEdge == 0 ? 1 : 0;		// the status of the pin when we should fire the timer
		Trig2_Timer2_Enabled = 1;
	}
	else {
		Trig2_Timer2_Enabled = 0;
	}
	#endif

	Trig1_IN_Enabled = old_Trig1_IN_Enabled;
	Trig2_IN_Enabled = old_Trig2_IN_Enabled;

	DbgOut("programBank=done\r\n");
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
	unsigned char old_Trig1_IN_Enabled;
	unsigned char old_Trig2_IN_Enabled;
	unsigned char noChannelPower;


	if (anOutputId < 1) anOutputId = 1;
	if (anOutputId > 2) anOutputId = 2;

	DbgOut("Programming output head...\r\n");
	//temporarily disable input triggers
	old_Trig1_IN_Enabled = Trig1_IN_Enabled;
	old_Trig2_IN_Enabled = Trig2_IN_Enabled;
	Trig1_IN_Enabled = 0;
	Trig2_IN_Enabled = 0;

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

	DbgOut("Enable pulse module\r\n");

	if (anOutputId == 1)
		OUTA_CONFIG.OCM = 0x4;	// enable single pulse mode
	else
		OUTB_CONFIG.OCM = 0x4;	// enable single pulse mode

	//restore input trigger settings
	Trig1_IN_Enabled = old_Trig1_IN_Enabled;
	Trig2_IN_Enabled = old_Trig2_IN_Enabled;

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
// the ID of the trigger this output head will trigger on. 1 - Trigger 1, 2 - Trigger 2, 3 - Trigger 1 or 2

void fireSoftTrigger(unsigned char aTriggerId)
{
	unsigned char RD5_Old;
	unsigned char RD6_Old;

	RD5_Old = TRIG_INA_CPLD;
	RD6_Old = _RD6;

//	outputString(&Uart1, "ST \r\n");
//
//	outputString(&Uart1, " Trig2_IN = ");
//	outputIntAsString(&Uart1, Trig2_IN);
//
//	outputString(&Uart1, " LATD6 = ");
//	outputIntAsString(&Uart1, LATD6);
//
//	outputString(&Uart1, "\r\n");

	if (aTriggerId & 0x01) {
		if (Trig1_Timer2_Enabled && T2CONbits.TON == 0) {
			TRIG_INA_CPLD_TRIS = 0;				// enable RD5 as input RD5 is shared with CN14
			asm("nop");
			asm("nop");
			asm("nop");
    		TRIG_INA_CPLD = !RD5_Old;
//			outputString(&Uart1, "RD5 = Trig1_Timer2_Edge : ");
//			outputIntAsString(&Uart1, Trig1_Timer2_Edge);
//			outputString(&Uart1, " RD5_Old = ");
//			outputIntAsString(&Uart1, RD5_Old);
//			outputString(&Uart1, "\r\n");
		}

		if (Trig1_Timer3_Enabled && T3CONbits.TON == 0) {
			TRIG_INA_CPLD_TRIS = 0;				// enable RD5 as input RD5 is shared with CN14
			asm("nop");
			asm("nop");
			asm("nop");
    		TRIG_INA_CPLD = !RD5_Old ;
//			outputString(&Uart1, "RD5 = Trig1_Timer3_Edge : ");
//			outputIntAsString(&Uart1, Trig1_Timer3_Edge);
//			outputString(&Uart1, " RD5_Old = ");
//			outputIntAsString(&Uart1, RD5_Old);
//			outputString(&Uart1, "\r\n");
		}

	}

//	if (aTriggerId & 0x02) {
//		if (Trig2_Timer2_Enabled && T2CONbits.TON == 0) {
//			_TRISD6 = 0;				// enable RD5 as input RD5 is shared with CN14
//			#asm
//			nop
//			nop
//			nop
//			#endasm
//    		LATD6 = !RD6_Old;
//			outputString(&Uart1, "RD6 = Trig2_Timer2_Edge : ");
//			outputIntAsString(&Uart1, Trig2_Timer2_Edge);
//			outputString(&Uart1, "\r\n");
//		}
//
//		if (Trig2_Timer3_Enabled && T3CONbits.TON == 0) {
//			_TRISD6 = 0;				// enable RD5 as input RD5 is shared with CN14
//			#asm
//			nop
//			nop
//			nop
//			#endasm
//    		LATD6 = !RD6_Old;
//			outputString(&Uart1, "RD6 = Trig2_Timer3_Edge : ");
//			outputIntAsString(&Uart1, Trig2_Timer3_Edge);
//			outputString(&Uart1, "\r\n");
//		}
//	}

	delay_us(50);

	if (aTriggerId & 0x01) {
		TRIG_INA_CPLD = RD5_Old;
		TRIG_INA_CPLD_TRIS = 1;				// enable RD5 as input RD5 is shared with CN14
		TRIG_INA_CPLD = RD5_Old;
	}
//
//	if (aTriggerId & 0x02) {
//		RD6 = RD6_Old;
//		_TRISD6 = 0;				// enable RD6 as input RD56
//	}

	asm("nop");
	asm("nop");
	asm("nop");
//	outputString(&Uart1, "STD\r\n");
}

//-----------------------------------------------------------------------------------------
void processNextSequence(void)
{
	unsigned char old_Trig1_IN_Enabled;
	unsigned char old_Trig2_IN_Enabled;

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
THeadStatus* getHeadStatus(unsigned char anOutputId)
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

	static unsigned char old_Trig1_IN_Enabled;
	static unsigned char old_Trig2_IN_Enabled;

	if (pInfo->m_output[0].m_triggerEdge == TriggerDC)
		return;

	//temporarily ignore input triggers
	old_Trig1_IN_Enabled = Trig1_IN_Enabled;
	old_Trig2_IN_Enabled = Trig2_IN_Enabled;

	Trig1_IN_Enabled = 0;
	Trig2_IN_Enabled = 0;


	OUTA_CONFIG.OCM = 0;	//disable pulse module initially
	T2CONbits.TON = 0;		//disable the timer

	TRIG_OUTA_PIC = 0; 		//set pulse output to low for output A
	OUTA_CONFIG.OCTSEL = 0;	// use timer 2 as a source


	if (pInfo->m_strobeTimerPrescaler[0] == 0xffff) {
		if (getTimeInTicks(pInfo->m_output[0].m_strobeDelay) + getTimeInTicks(pInfo->m_output[0].m_strobeWidth) < 0xFFF0)
			pInfo->m_strobeTimerPrescaler[0] = 0;
		else if (getTimeInTicksPre(pInfo->m_output[0].m_strobeDelay, 8) + getTimeInTicksPre(pInfo->m_output[0].m_strobeWidth, 8) < 0xFFF0)
			pInfo->m_strobeTimerPrescaler[0] = 8;
		else if (getTimeInTicksPre(pInfo->m_output[0].m_strobeDelay, 64) + getTimeInTicksPre(pInfo->m_output[0].m_strobeWidth, 64) < 0xFFF0)
			pInfo->m_strobeTimerPrescaler[0] = 64;
		else
			pInfo->m_strobeTimerPrescaler[0] = 256;
	}

	// the fixed delay between we detect the trigger and ISR enables the timer
	// for  7.37Mhz crystal in 16X PLL mode
	// right now for head 1 it is 3us
	if (pInfo->m_strobeTimerPrescaler[0] == 0)
		interruptDelay = getTimeInTicks(3);
	else
		interruptDelay = 0;

	DbgOut(" setPulseInfoOutput1 width=");
	DbgOutInt(pInfo->m_output[0].m_strobeWidth);
	DbgOut(" delay=");
	DbgOutInt(pInfo->m_output[0].m_strobeDelay);
	DbgOut(" scale=");
	DbgOutInt(pInfo->m_strobeTimerPrescaler[0]);
	DbgOut(" ISR delay=");
	DbgOutInt(interruptDelay);


	switch (pInfo->m_strobeTimerPrescaler[0]) {
		case 0:
			T2CONbits.TCKPS=0x0;		// use prescaler *1:1* 1:8 1:64 1:256
			break;

		case 8:
			T2CONbits.TCKPS=0x1;		// use prescaler 1:1 *1:8* 1:64 1:256
			break;

		case 64:
			T2CONbits.TCKPS=0x2;		// use prescaler 1:1 1:8 *1:64* 1:256
			break;

		case 256:
			T2CONbits.TCKPS=0x3;		// use prescaler 1:1 1:8 1:64 *1:256*
			break;
	}

	T2CONbits.TCS=0;		// use instruction clock
	TMR2=0;					// reset the counter for timer 2
	_T2IF = 0;				// reset interrupt flag for timer 2
	_T2IE = 1;				// enable timer 2 interrupt

	// pulse time START
	OUTA_TIME_START =  getTimeInTicksPre(pInfo->m_output[0].m_strobeDelay, pInfo->m_strobeTimerPrescaler[0]);

	// check if the delay is more than ISR delay, then adjust it for the ISR delay
	if (OUTA_TIME_START >= interruptDelay)
		OUTA_TIME_START -= interruptDelay;
	else
		OUTA_TIME_START = 1;

	if (OUTA_TIME_START == 0)			// the minimum delay is 1 timer tick
		OUTA_TIME_START = 1;

	DbgOut(" OUTA_TIME_START=");
	DbgOutInt(OUTA_TIME_START);


	// pulse time STOP
	OUTA_TIME_STOP= OUTA_TIME_START + getTimeInTicksPre(pInfo->m_output[0].m_strobeWidth, pInfo->m_strobeTimerPrescaler[0]);

	if (OUTA_TIME_STOP < 2)			// the minimum width is 1 timer tick after the delay
		OUTA_TIME_STOP = 2;

	DbgOut(" OUTA_TIME_STOP=");
	DbgOutInt(OUTA_TIME_STOP);

	PR2 = OUTA_TIME_STOP+2;	// now program the Timer 2 duration so as soon as we program that the timer 2 is ready
							// we will start the timer in the input interrupt, when we detect TRIG_INA
							//
							// TODO: check! in this interrupt we will reenable this OC module

	//restore input trigger settings
	Trig1_IN_Enabled = old_Trig1_IN_Enabled;
	Trig2_IN_Enabled = old_Trig2_IN_Enabled;

	DbgOut("\r\n");
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
 *
*/
//-----------------------------------------------------------------------------------------
void setPulseInfoOutput2(TBankInfo* pInfo)
{
	static unsigned long interruptDelay;

	static unsigned char old_Trig1_IN_Enabled;
	static unsigned char old_Trig2_IN_Enabled;

	if (pInfo->m_output[1].m_triggerEdge == TriggerDC)
		return;

	//temporarily ignore input triggers
	old_Trig1_IN_Enabled = Trig1_IN_Enabled;
	old_Trig2_IN_Enabled = Trig2_IN_Enabled;

	Trig1_IN_Enabled = 0;
	Trig2_IN_Enabled = 0;


	OUTB_CONFIG.OCM = 0;	//disable pulse module initially
	T3CONbits.TON = 0;		//disable the timer

	TRIG_OUTB_PIC = 0; 		//set pulse output to low for output B
	OUTB_CONFIG.OCTSEL = 1;	// use timer 3 as a source


	if (pInfo->m_strobeTimerPrescaler[1] == 0xffff) {
		if (getTimeInTicks(pInfo->m_output[1].m_strobeDelay) + getTimeInTicks(pInfo->m_output[1].m_strobeWidth) < 0xFFF0)
			pInfo->m_strobeTimerPrescaler[1] = 0;
		else if (getTimeInTicksPre(pInfo->m_output[1].m_strobeDelay, 8) + getTimeInTicksPre(pInfo->m_output[1].m_strobeWidth, 8) < 0xFFF0)
			pInfo->m_strobeTimerPrescaler[1] = 8;
		else if (getTimeInTicksPre(pInfo->m_output[1].m_strobeDelay, 64) + getTimeInTicksPre(pInfo->m_output[1].m_strobeWidth, 64) < 0xFFF0)
			pInfo->m_strobeTimerPrescaler[1] = 64;
		else
			pInfo->m_strobeTimerPrescaler[1] = 256;
	}

	// the fixed delay between we detect the trigger and ISR enables the timer
	// for  7.37Mhz crystal in 16X PLL mode
	// right now and for head 2 it is 6us
	if (pInfo->m_strobeTimerPrescaler[1] == 0)
		interruptDelay = getTimeInTicks(6);
	else
		interruptDelay = 0;

	DbgOut(" setPulseInfoOutput2 width=");
	DbgOutInt(pInfo->m_output[1].m_strobeWidth);
	DbgOut(" delay=");
	DbgOutInt(pInfo->m_output[1].m_strobeDelay);
	DbgOut(" scale=");
	DbgOutInt(pInfo->m_strobeTimerPrescaler[1]);
	DbgOut(" ISR delay=");
	DbgOutInt(interruptDelay);


	switch (pInfo->m_strobeTimerPrescaler[1]) {
		case 0:
			T3CONbits.TCKPS=0x0;		// use prescaler *1:1* 1:8 1:64 1:256
			break;

		case 8:
			T3CONbits.TCKPS=0x1;		// use prescaler 1:1 *1:8* 1:64 1:256
			break;

		case 64:
			T3CONbits.TCKPS=0x2;		// use prescaler 1:1 1:8 *1:64* 1:256
			break;

		case 256:
			T3CONbits.TCKPS=0x3;		// use prescaler 1:1 1:8 1:64 *1:256*
			break;
	}

	T3CONbits.TCS=0;		// use instruction clock
	TMR3=0;					// reset the counter for timer 3
	_T3IF = 0;				// reset interrupt flag for timer 3
	_T3IE = 1;				// enable timer 3 interrupt

	// pulse time START
	OUTB_TIME_START =  getTimeInTicksPre(pInfo->m_output[1].m_strobeDelay, pInfo->m_strobeTimerPrescaler[1]);

	// check if the delay is more than ISR delay, then adjust it for the ISR delay
	if (OUTB_TIME_START >= interruptDelay)
		OUTB_TIME_START -= interruptDelay;
	else
		OUTB_TIME_START = 1;

	if (OUTB_TIME_START == 0)			// the minimum delay is 1 timer tick
		OUTB_TIME_START = 1;

	DbgOut(" OUTB_TIME_START=");
	DbgOutInt(OUTB_TIME_START);


	// pulse time STOP
	OUTB_TIME_STOP= OUTB_TIME_START + getTimeInTicksPre(pInfo->m_output[1].m_strobeWidth, pInfo->m_strobeTimerPrescaler[1]);

	if (OUTB_TIME_STOP < 2)			// the minimum width is 1 timer tick after the delay
		OUTB_TIME_STOP = 2;

	DbgOut(" OUTB_TIME_STOP=");
	DbgOutInt(OUTB_TIME_STOP);

	PR3 = OUTB_TIME_STOP+2;	// now program the Timer 3 duration so as soon as we program that the timer 3 is ready
							// we will start the timer in the input interrupt, when we detect TRIG_INB
							//
							// TODO: check! in this interrupt we will reenable this OC module

	//restore input trigger settings
	Trig1_IN_Enabled = old_Trig1_IN_Enabled;
	Trig2_IN_Enabled = old_Trig2_IN_Enabled;

	DbgOut("\r\n");
}


//-----------------------------------------------------------------------------------------
// Here is our lovely interrupt function that processes trigger input change
//-----------------------------------------------------------------------------------------
void _ISR __attribute__ ((auto_psv)) _CNInterrupt(void)
{
	static unsigned char theTrigInACpld;
	static unsigned char theTrigInBCpld;

	static unsigned char bIncTrig1;
	static unsigned char bIncTrig2;

	theTrigInACpld  = TRIG_INA_CPLD;	// Current status of IO
	theTrigInBCpld  = TRIG_INB_CPLD;

	bIncTrig1 = bIncTrig2 = 0;

	// trig 1 is fired
	if (Trig1_IN_Last != theTrigInACpld && Trig1_IN_Enabled) {

		// only if the edge is the same and the timer is not running
		if (Trig1_Timer2_Enabled && theTrigInACpld == Trig1_Timer2_Edge && T2CONbits.TON == 0) {

			OUTA_CONFIG.OCM = 0x4;	// enable single pulse mode
			T2CONbits.TON = 1;		// turn ON timer

			bIncTrig1 = 1;
		}

		// only if the edge is the same and the timer is not running
		if (Trig1_Timer3_Enabled && theTrigInACpld == Trig1_Timer3_Edge && T3CONbits.TON == 0) {
			OUTB_CONFIG.OCM = 0x4;	// enable single pulse mode
    		T3CONbits.TON = 1;		// turn ON timer

			bIncTrig1 = 1;
		}
	}

	// trig 2 is fired
	if (Trig2_IN_Last != theTrigInBCpld  && Trig2_IN_Enabled) {

		// only if the edge is the same and the timer is not running
		if (Trig2_Timer2_Enabled && theTrigInBCpld == Trig2_Timer2_Edge && T2CONbits.TON == 0) {

			OUTA_CONFIG.OCM = 0x4;	// enable single pulse mode
    		T2CONbits.TON = 1;		// turn ON timer
			bIncTrig2 = 1;
		}

		// only if the edge is the same and the timer is not running
		if (Trig2_Timer3_Enabled && theTrigInBCpld == Trig2_Timer3_Edge && T3CONbits.TON == 0) {

			OUTB_CONFIG.OCM = 0x4;	// enable single pulse mode
    		T3CONbits.TON = 1;	// turn ON timer
			bIncTrig2 = 1;
		}

	}
	// store the current state of theTrigInACpld
	Trig1_IN_Last = theTrigInACpld;
	Trig2_IN_Last = theTrigInBCpld;

	if (bIncTrig1) ++theTriggerCounter1;	// increment the trigger counter
	if (bIncTrig2) ++theTriggerCounter2;	// increment the trigger counter

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

	OUTA_CONFIG.OCM = 0x4;	// enable single pulse mode

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
							//
	TMR3 = 0;				// reset the counter for timer 3
	TRIG_OUTB_PIC = 0;		// make sure it is LOW

	OUTB_CONFIG.OCM = 0x4;	// enable single pulse mode

	_T3IF = 0;				// important to clear this bit
}


