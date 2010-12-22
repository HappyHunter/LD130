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

/*
Trigger pins:
	Trigger 1:	RD5/67/CN14 Input  _LATD2/OC3/62 - output
	Trigger 2:	RD6/68/CN15	Input  _LATD3/OC4/63 - output

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
extern unsigned long TheFlashFlags;


unsigned char Trig1_IN_Enabled;		// The flag that enables trig IN signal
									// it will be set to disabled during any re-programming operations

unsigned char Trig1_IN;				// the initail state of trig1

unsigned char Trig1_Timer2_Enabled; // if we need to start timer 2 when we receive trigger 1
unsigned char Trig1_Timer2_Edge;	// on which edge to trigger the timer

unsigned char Trig1_Timer3_Enabled;	// if we need to start timer 3 when we receive trigger 1
unsigned char Trig1_Timer3_Edge;	// on which edge to start the timer


unsigned char Trig2_IN_Enabled;		// The flag that enables trig IN signal
									// it will be set to disabled during any re-programming operations

unsigned char Trig2_IN;

unsigned char Trig2_Timer2_Enabled; // if we need to start timer 2 when we receive trigger 1
unsigned char Trig2_Timer2_Edge;	// on which edge to trigger the timer

unsigned char Trig2_Timer3_Enabled;	// if we need to start timer 3 when we receive trigger 1
unsigned char Trig2_Timer3_Edge;	// on which edge to start the timer


unsigned char Trig1_IN_Loopback;	// initial state of loopback trigger

unsigned char AdvanceToTheNextBank = 1;	// flag that is set when we nned to advance to the next bank





static unsigned long getTimeInTicks(unsigned long aTimeInMicrosec)
{
	return ((unsigned long)((A_FOSC_/1000000UL) * aTimeInMicrosec)) ;
}

static unsigned long getTimeInTicksPre(unsigned long aTimeInMicrosec, unsigned long timerPreScaler)
{
	return ((unsigned long)((A_FOSC_/1000000UL) * aTimeInMicrosec) / (timerPreScaler ? timerPreScaler : 1)) ;
}


//-----------------------------------------------------------------------------------------
// stops execution by the delay specified
// since it involves division operation which takes 18 cycles be carefull on slow speeds
//
//-----------------------------------------------------------------------------------------
void delay_us(unsigned long aTimeInMicrosec)
{
	unsigned long i;
	for (i = (getTimeInTicks(aTimeInMicrosec) >> 1); i > 0; --i) {
		ClrWdt();
	}
}

void programOutputHead(TBankHeadData* pHeadData);

//-----------------------------------------------------------------------------------------
// initalizes the PIN for trigger 1
// It uses CN14 input located at RD5 input
// It also enables the interrupt on input change
//-----------------------------------------------------------------------------------------
void initTrigger1()
{
	_TRISD5 = 1;			// enable RD5 as input RD5 is shared with CN14
	_CN14IE = 1;			// enable interrupt on the pin status change
	_CNIF  = 0;				// clear CN notification flag
	_CN14PUE = 1;			// enable pullup for this pin (optional step only if external pullup is not present)
	IPC3bits.CNIP = 0x02;	// assign the priority level 2. 0 is the highest
	Trig1_IN = _RD5;		// read the current status of input
	_CNIE = 1;				// global enable interrupts on pin change

	Trig1_Timer2_Enabled = 0; 	// if we need to start timer 2 when we receive trigger 1
	Trig1_Timer2_Edge = 0;		// on which edge to trigger the timer

	Trig1_Timer3_Enabled = 0;	// if we need to start timer 3 when we receive trigger 1
	Trig1_Timer3_Edge = 0;		// on which edge to start the timer



	//AdvanceToTheNextBank = 0; //TODO check needed?

	Trig1_IN_Enabled = 1;	//trigger 1 input is now enabled

}


void initTrigger1Loopback(void)
{

	_TRISB15 = 1;			// enable RD5 as input RD5 is shared with CN15
	_CN12IE = 1;				// enable interrupt on the pin status change
	_CNIF  = 0;				// clear CN notification flag
	_CN12PUE = 1;			// enable pullup for this pin (optional step only if external pullup is not present)
	IPC3bits.CNIP = 0x02;	// assign the priority level 2. 0 is the highest
	Trig1_IN_Loopback = _RB15;// read the current status of input
	_CNIE = 1;				// global enable interrupts on pin change

	T3CONbits.TON = 0;		// disable the timer

	//used for voltage head control TRISB11 = 0;						// enable loopback output
	//used for voltage head control RB11	= !Trig1_IN_Loopback;	// inverted state
}


//-----------------------------------------------------------------------------------------
// initalizes the PIN for trigger 2
// It uses CN15 input located at RD6 input
// It also enables the interrupt on input change
//-----------------------------------------------------------------------------------------
void initTrigger2()
{
	_TRISD6 = 1;				// enable RD6 as input RD6 is shared with CN15
	_CN15IE = 1;				// enable interrupt on the pin status change
	_CNIF  = 0;				// clear CN notification flag
	_CN15PUE = 1;			// enable pullup for this pin (optional step only if external pullup is not present)
	IPC3bits.CNIP = 0x02;	// assign the priority level 2. 0 is the highest
	Trig2_IN = _RD6;			// read the current status of input
	_CNIE = 1;				// global enable interrupts on pin change

	Trig2_Timer2_Enabled = 0; 	// if we need to start timer 2 when we receive trigger 2
	Trig2_Timer2_Edge = 0;		// on which edge to trigger the timer

	Trig2_Timer3_Enabled = 0;	// if we need to start timer 3 when we receive trigger 2
	Trig2_Timer3_Edge = 0;		// on which edge to start the timer

	//Setup trigger 2 output (pin: _LATD2/OC4)
	_LATD3 = 0;				// clear latched trigger 2 output data

	T3CONbits.TON = 0;		// disable the timer 3

	//TODO setup OC4 ?

	Trig2_IN_Enabled = 1;	// the trigger 2 input is now enabled
}


//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
void programBank(const TBankInfo* pBankInfo)
{
	unsigned char old_Trig1_IN_Enabled;
	unsigned char old_Trig2_IN_Enabled;
	unsigned char period;

	old_Trig1_IN_Enabled = Trig1_IN_Enabled;
	old_Trig2_IN_Enabled = Trig2_IN_Enabled;


	// wait until the trigger timers OFF
	period = 0;
	while (T2CONbits.TON || T3CONbits.TON || _LATD2 || _LATD3) {
		ClrWdt();
		if ((++period % 30) == 0) {
			if (T2CONbits.TON)
				DbgOut("T2CONbits.TON\r\n");
			if (T3CONbits.TON)
				DbgOut("T3CONbits.TON\r\n");
			if (_LATD2)
				DbgOut("_LATD2\r\n");
			if (_LATD3)
				DbgOut("_LATD3\r\n");
		}
	}
	_T2IE = 0;				// disable timer 2 interrupt
	_T3IE = 0;				// disable timer 3 interrupt
	T2CONbits.TON = 0;		// disable timer 2
	T3CONbits.TON = 0;		// disable timer 3
	TMR2 = 0;				// reset the counter for timer 2
	TMR3 = 0;				// reset the counter for timer 3
	OC3CONbits.OCM = 0x0;	// disable pulse generator
	OC4CONbits.OCM = 0x0;	// disable pulse generator

	//Disable trigger inputs temporarily, will be restored later
	Trig1_IN_Enabled = 0;
	Trig2_IN_Enabled = 0;

	DbgOut("Programming bank = ");
	DbgOutInt(pBankInfo->m_id);
	DbgOut("\r\n");

	//remember the bank ID to not reprogram the hardware next time if the bank doesn't change
	BankLastUsedId = pBankInfo->m_id;

//		outputString(&Uart1, "pBankInfo->m_id = ");
//		outputIntAsString(&Uart1, pBankInfo->m_id);
//		outputString(&Uart1, "\r\n");
//
//		outputString(&Uart1, "pBankInfo->m_chanelPower[0] = ");
//		outputIntAsString(&Uart1, pBankInfo->m_output[0].m_chanelPower[0]);
//		outputString(&Uart1, "\r\n");
//
//		outputString(&Uart1, "pBankInfo->m_chanelPower[1] = ");
//		outputIntAsString(&Uart1, pBankInfo->m_output[0].m_chanelPower[1]);
//		outputString(&Uart1, "\r\n");


//	outputString(&Uart1, "programOutputHead=0\r\n");
	programOutputHead(&(pBankInfo->m_output[0]));
//	outputString(&Uart1, "programOutputHead=1\r\n");
	programOutputHead(&(pBankInfo->m_output[1]));

	if (1/*pBankInfo->m_output[0].m_triggerEdge == pBankInfo->m_output[1].m_triggerEdge &&
		pBankInfo->m_output[0].m_triggerID == pBankInfo->m_output[1].m_triggerID &&
		pBankInfo->m_output[0].m_strobeTimerPrescaler == pBankInfo->m_output[1].m_strobeTimerPrescaler*/
	) {
		T2CONbits.TON = 0;		// disable timer 2
		TMR2=0;					// reset the counter for timer 2
	//	T3CONbits.TON = 0;		// disable timer 3
	//	TMR3=0;					// reset the counter for timer 3

		// in this case we can use the same timer 2 source for both trigger outputs
		OC3CONbits.OCTSEL = 0;	// use timer 2 as a source for trigger 1 output
		OC4CONbits.OCTSEL = 0;	// use timer 2 as a source for trigger 2 output

		T2IE = 0;				// disable timer 2 interrupt
	//	T3IE = 0;				// disable timer 3 interrupt

		//timer 3 is not used
		Trig1_Timer3_Enabled = 0;
		Trig2_Timer3_Enabled = 0;

		// set the timer period as the maximum of two ending times
		// at the end of the period the timer interrupt will fire
		// in that interrupt we will reenable OC module
		PR2 = OC3RS > OC4RS ? OC3RS+4 : OC4RS+4;

		T2IF = 0;				// reset interrupt flag for timer 2
		T2IE = 1;				// enable timer 2 interrupt

		// Check whether we have to use trigger input 1
		if ((pBankInfo->m_output[0].m_triggerID & TriggerID1) != 0) {
			Trig1_Timer2_Edge    = pBankInfo->m_output[0].m_triggerEdge == 0 ? 1 : 0;		// the status of the pin when we should fire the timer
			Trig1_Timer2_Enabled = 1;
		}
		else {
			Trig1_Timer2_Enabled = 0;
		}

		// Check whether we have to use trigger input 2
		if ((pBankInfo->m_output[0].m_triggerID & TriggerID2) != 0) {
			Trig2_Timer2_Edge    = pBankInfo->m_output[0].m_triggerEdge == 0 ? 1 : 0;		// the status of the pin when we should fire the timer
			Trig2_Timer2_Enabled = 1;
		}
		else {
			Trig2_Timer2_Enabled = 0;
		}

	}
	else {
		// We need to decide which timer we will start on trigger 1
		// since we got here because the parameters are different so that
		// 2 outputs cannot use the same timer

		// output head 1 is attached to timer 2
		Trig1_Timer2_Enabled = (pBankInfo->m_output[0].m_triggerID & TriggerID1) && pBankInfo->m_output[0].m_strobeWidth != 0;
		Trig1_Timer2_Edge    = pBankInfo->m_output[0].m_triggerEdge == 0 ? 1 : 0;		// the status of the pin when we should fire the timer

		// output head 2 attached to timer 3
		Trig1_Timer3_Enabled = (pBankInfo->m_output[1].m_triggerID & TriggerID1) && pBankInfo->m_output[1].m_strobeWidth != 0;
		Trig1_Timer3_Edge    = pBankInfo->m_output[1].m_triggerEdge == 0 ? 1 : 0;		// the status of the pin when we should fire the timer


		// here we set which timers should be started on trigger 2
		Trig2_Timer2_Enabled = (pBankInfo->m_output[0].m_triggerID & TriggerID2) && pBankInfo->m_output[0].m_strobeWidth != 0;
		Trig2_Timer2_Edge    = pBankInfo->m_output[0].m_triggerEdge == 0 ? 1 : 0;		// the status of the pin when we should fire the timer

		// output head 2 attached to timer 3
		Trig2_Timer3_Enabled = (pBankInfo->m_output[1].m_triggerID & TriggerID2) && pBankInfo->m_output[1].m_strobeWidth != 0;
		Trig2_Timer3_Edge    = pBankInfo->m_output[1].m_triggerEdge == 0 ? 1 : 0;		// the status of the pin when we should fire the timer

	}
	Trig1_IN_Enabled = old_Trig1_IN_Enabled;
	Trig2_IN_Enabled = old_Trig2_IN_Enabled;

	outputString(&UartDbg, "programBank=done\r\n");
}

//-----------------------------------------------------------------------------------------
void setPulseInfoOutput1(TOutputHeadData* pHead)
{
	unsigned long interruptDelay;

	unsigned char old_Trig1_IN_Enabled;
	unsigned char old_Trig2_IN_Enabled;

	//Setup trigger 1 output (pin: _LATD2/OC3)

	//_LATD2 = 0;				//reset trigger 1 output
	//LATD2 = 0;				//clear latched trigger 1 output data

	//T2CONbits.TON = 0;		// disable the timer 2
	//OC3CONbits.OCTSEL = 0;	// use timer 2 as a source
	//OC3R = 0xFFF0;			// pulse start time from the timer start
	//OC3RS = 0xFFF7;			// pulse end time from the timer start
    //OC3CONbits.OCM = 0x4;	// enable single pulse mode


	//temporarily ignore input triggers
	old_Trig1_IN_Enabled = Trig1_IN_Enabled;
	old_Trig2_IN_Enabled = Trig2_IN_Enabled;
	Trig1_IN_Enabled = 0;
	Trig2_IN_Enabled = 0;

	OC3CONbits.OCM = 0;		//disable pulse module initially
	T2CONbits.TON = 0;		//disable the timer

	_LATD2 = 0; 				//reset head 1 pulse output
	OC3CONbits.OCTSEL = 0;	// use timer 2 as a source


	if (pHead->m_strobeTimerPrescaler == 0xffff) {
		if (getTimeInTicks(pHead->m_strobeDealy) + getTimeInTicks(pHead->m_strobeWidth) < 0xFFF0)
			pHead->m_strobeTimerPrescaler = 0;
		else if (getTimeInTicksPre(pHead->m_strobeDealy, 8) + getTimeInTicksPre(pHead->m_strobeWidth, 8) < 0xFFF0)
			pHead->m_strobeTimerPrescaler = 8;
		else if (getTimeInTicksPre(pHead->m_strobeDealy, 64) + getTimeInTicksPre(pHead->m_strobeWidth, 64) < 0xFFF0)
			pHead->m_strobeTimerPrescaler = 64;
		else
			pHead->m_strobeTimerPrescaler = 256;
	}

	// the fixed delay between we detect the trigger and ISR enables the timer
	// for  7.37Mhz crystal in 16X PLL mode
	// right now for trigger 1 it is 3us
	if (pHead->m_strobeTimerPrescaler == 0)
		interruptDelay = getTimeInTicks(3);
	else
		interruptDelay = 0;

	outputString(&UartDbg, " setPulseInfoOutput1 width=");
	outputIntAsString(&UartDbg, pHead->m_strobeWidth);
	outputString(&UartDbg, " delay=");
	outputIntAsString(&UartDbg, pHead->m_strobeDealy);
	outputString(&UartDbg, " scale=");
	outputIntAsString(&UartDbg, pHead->m_strobeTimerPrescaler);
	outputString(&UartDbg, " ISR delay=");
	outputIntAsString(&UartDbg, interruptDelay);


	switch (pHead->m_strobeTimerPrescaler) {
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

	// pulse time start
	OC3R =  getTimeInTicksPre(pHead->m_strobeDealy, pHead->m_strobeTimerPrescaler);

	// check if the delay is more than ISR delay, then adjust it for the ISR delay
	if (OC3R >= interruptDelay)
		OC3R -= interruptDelay;
	else
		OC3R = 1;

	if (OC3R == 0)			// the minimum delay is 1 timer tick
		OC3R = 1;

	outputString(&UartDbg, " OC3R=");
	outputIntAsString(&UartDbg, OC3R);

	OC3RS= OC3R + getTimeInTicksPre(pHead->m_strobeWidth, pHead->m_strobeTimerPrescaler);		// pulse time stop

	if (OC3RS < 2)			// the minimum width is 1 timer tick after the delay
		OC3RS = 2;

	outputString(&UartDbg, " OC3RS=");
	outputIntAsString(&UartDbg, OC3RS);
	outputString(&UartDbg, "\r\n");

	PR2 = OC3RS+4;			// this will also trigger an interrupt for timer 2
							// TODO: check! in this interrupt we will reenable this OC module

	//restore input trigger settings
	Trig1_IN_Enabled = old_Trig1_IN_Enabled;
	Trig2_IN_Enabled = old_Trig2_IN_Enabled;
}
//-----------------------------------------------------------------------------------------
void setPulseInfoOutput2(TOutputHeadData* pHead)
{
	unsigned long interruptDelay;

	unsigned char old_Trig1_IN_Enabled;
	unsigned char old_Trig2_IN_Enabled;

	old_Trig1_IN_Enabled = Trig1_IN_Enabled;
	old_Trig2_IN_Enabled = Trig2_IN_Enabled;


	Trig1_IN_Enabled = 0;
	Trig2_IN_Enabled = 0;
/*
	OC4CONbits.OCM = 0;	// disbale module

	// the fixed delay between we detect the trigger and ISR enables the timer
	// for  7.37Mhz crystal in 16X PLL mode
	// right now for trigger 1 it is 1.5us
	if (pHead->m_strobeTimerPrescaler == 0)
		interruptDelay = getTimeInTicks(1) + (getTimeInTicks(1) >> 1);
	else
		interruptDelay = 0;

	if (pHead->m_strobeTimerPrescaler == 0xffff) {
		if (getTimeInTicks(pHead->m_strobeDealy) + getTimeInTicks(pHead->m_strobeWidth) < 0xFFF0)
			pHead->m_strobeTimerPrescaler = 0;
		else if (getTimeInTicksPre(pHead->m_strobeDealy, 8) + getTimeInTicksPre(pHead->m_strobeWidth, 8) < 0xFFF0)
			pHead->m_strobeTimerPrescaler = 8;
		else if (getTimeInTicksPre(pHead->m_strobeDealy, 64) + getTimeInTicksPre(pHead->m_strobeWidth, 64) < 0xFFF0)
			pHead->m_strobeTimerPrescaler = 64;
		else
			pHead->m_strobeTimerPrescaler = 256;
	}

	OC4CONbits.OCTSEL = 1;	// use timer 3 as a source
	T3CONbits.TON = 0;		// disable the timer
	switch (pHead->m_strobeTimerPrescaler) {
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
	T3IF = 0;				// reset interrupt flag for timer 3
	T3IE = 1;				// enable timer 3 interrupt

//	_LATD3 = 0;				// set trigger OUT pin to 0,


	OC4R = getTimeInTicksPre(pHead->m_strobeDealy, pHead->m_strobeTimerPrescaler) ;			// pulse time start

	// check if the delay is more than 1.5 us, then adjust it for the ISR delay
	if (OC4R > interruptDelay)
		OC4R -= interruptDelay;

	if (OC4R == 0)			// the minimum delay is 1 timer tick
		OC4R = 1;

	OC4RS= OC4R + getTimeInTicksPre(pHead->m_strobeWidth, pHead->m_strobeTimerPrescaler);		// pulse time stop

	if (OC4RS < 2)			// the minimum width is 1 timer tick after the delay
		OC4RS = 2;

	PR3 = OC4RS;			// this will also trigger an interrupt for timer 3
							// in this interrupt we will reenable this OC module

	OC4CONbits.OCM = 0x4;	// enable single pulse mode
*/
	Trig1_IN_Enabled = old_Trig1_IN_Enabled;
	Trig2_IN_Enabled = old_Trig2_IN_Enabled;
}

//-----------------------------------------------------------------------------------------
// this function calculates the DAC current value to be programmed to LTC1660 DAC
//
// The DAC can be programmed from 0 - 1023
//
// unsigned long aPower 			- from 0% to 100% with 2 fixed decimal digits, so the value of 100% is represented as 10000
// unsigned long anAmplifierValue	- from 1 - 5
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

//-----------------------------------------------------------------------------------------
// this function calculates the DAC voltage value to be programmed to LTC1329 DAC
//
// The DAC can be programmed from 0 (100% voltage) - 255 (0% voltage)
//
// unsigned long aPower 			- from 0% to 100% in integer steps
//-----------------------------------------------------------------------------------------
unsigned char makeVoltageDACValue_LTC1329(unsigned long aPower)
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
//-----------------------------------------------------------------------------------------
short setCurrentDACValue_LTC1660(unsigned char aChip, unsigned char aChanel, unsigned short aValue)
{
	unsigned short workingChanelBin = 0x1000;

	switch (aChanel) {
		case 0:
			workingChanelBin = 0x1000;
			break;

		case 1:
			workingChanelBin = 0x2000;
			break;

		case 2:
			workingChanelBin = 0x3000;
			break;

		case 3:
			workingChanelBin = 0x4000;
			break;

		case 4:
			workingChanelBin = 0x5000;
			break;

		case 5:
			workingChanelBin = 0x6000;
			break;

		case 6:
			workingChanelBin = 0x7000;
			break;

		case 7:
			workingChanelBin = 0x8000;
			break;
	}

	if (aValue > 1023)
		aValue = 1023;

	// shift it to the left by 2 bits, since the last 2 bits are
	// not used in this DAC
	aValue <<= 2;

	aValue = workingChanelBin | aValue;


	//outputString(&Uart2, " IDAC ");
	//outputIntAsString(&Uart2, aChip);
	//outputString(&Uart2, " = 0x");
	//outputIntAsHexString(&Uart2, aValue);
	//outputString(&Uart2, "\r\n");
//

	// select slave device
	SPI1_ChipSelect_Single(aChip, 1);

	delay_us(10);
	SPI1_WriteRead(aValue);
	delay_us(10);



//	// get the high part of the value
//	valueHi = (unsigned char)(aValue >> 6);
//	valueHi &= 0x0F;
//	valueHi |= workingChanelBin;
//
//	// get the low part of the value
//	valueLo = (unsigned char)(aValue << 2);
//	valueLo &= 0xFF;



//	SPI1_WriteRead(valueHi);
//	SPI1_WriteRead(valueLo);


	// deselect slave device,
	// and this will also reprogram it with the value we sent
	SPI1_ChipSelect_Single(aChip, 0);

	return (1);
}
//-----------------------------------------------------------------------------------------
short setVoltageDACValue_LTC1329(unsigned char aChip, unsigned char aValue)
{
	//outputString(&Uart2, " VDAC ");
	//outputIntAsString(&Uart2, aChip);
	//outputString(&Uart2, " = 0x");
	//outputIntAsHexString(&Uart2, aValue);
	//outputString(&Uart2, "\r\n");

	SPI1_Set16BitMode(0);

	// select slave device
	SPI1_ChipSelect_Single(aChip, 1);

	delay_us(10);
	SPI1_WriteRead(aValue);
	delay_us(10);

	// deselect slave device,
	// and this will also reprogram it with the value we sent
	SPI1_ChipSelect_Single(aChip, 0);

	SPI1_Set16BitMode(1);

	return (1);
}

//-----------------------------------------------------------------------------------------
unsigned char headToCurrentChipSelect(unsigned char aHead)
{
	switch (aHead) {
		case 0:	return 0; // Chip 1
		case 1:	return 1; // Chip 2
	}
	return 255; //error
}
//-----------------------------------------------------------------------------------------
unsigned char headToVoltageChipSelect(unsigned char aHead)
{
	switch (aHead) {
		case 0:	return 2; // Chip 3
		case 1:	return 3; // Chip 4
	}
	return 255; //error
}

//-----------------------------------------------------------------------------------------
// Programs the DAC's chanel with the value specified
//-----------------------------------------------------------------------------------------
short setCurrentDACValue(unsigned char aHead, unsigned char aChanel, unsigned long aValue, unsigned long anAmplifierValue)
{
	return setCurrentDACValue_LTC1660(headToCurrentChipSelect(aHead), aChanel, makeCurrentDACValue_LTC1660(aValue, anAmplifierValue));
}

//-----------------------------------------------------------------------------------------
// Programs the DAC's chanel with the value specified
//-----------------------------------------------------------------------------------------
short setVoltageDACValue(unsigned char aHead, unsigned long aValue)
{
	switch (aHead) {
		case 0:
			_LATB11 = aValue > 0; //turn ON voltage for nonzero value
			break;
		case 1:
			_LATB12 = aValue > 0; //turn ON voltage for nonzero value
			break;
	}
	return setVoltageDACValue_LTC1329(headToVoltageChipSelect(aHead), makeVoltageDACValue_LTC1329(aValue));
}


//-----------------------------------------------------------------------------------------
// Programs the output head hardware with the parameters specified
//-----------------------------------------------------------------------------------------
void programOutputHead(TOutputHeadData* pHeadData)
{
	unsigned char old_Trig1_IN_Enabled;
	unsigned char old_Trig2_IN_Enabled;
	unsigned char noChannelPower;

	unsigned long value = 0;

	outputString(&UartDbg, "Programming output head...\r\n");
	//temporarily disable input triggers
	old_Trig1_IN_Enabled = Trig1_IN_Enabled;
	old_Trig2_IN_Enabled = Trig2_IN_Enabled;
	Trig1_IN_Enabled = 0;
	Trig2_IN_Enabled = 0;

	outputString(&UartDbg, "Init pulse module\r\n");

	if (pHeadData->m_id == 0)
		setPulseInfoOutput1(pHeadData);
	else
		setPulseInfoOutput2(pHeadData);

	outputString(&UartDbg, "Init head power\r\n");

//	outputString(&Uart1, "m_chanelPower[0] = ");
//	outputIntAsString(&Uart1, pHeadData->m_chanelPower[0]);
//	outputString(&Uart1, "\r\n");
//
//
//	outputString(&Uart1, "m_chanelPower[1] = ");
//	outputIntAsString(&Uart1, pHeadData->m_chanelPower[1]);
//	outputString(&Uart1, "\r\n");

	noChannelPower = 	pHeadData->m_chanelPower[0] == 0 &&
						pHeadData->m_chanelPower[1] == 0 &&
						pHeadData->m_chanelPower[2] == 0 &&
						pHeadData->m_chanelPower[3] == 0;

	setCurrentDACValue(pHeadData->m_id, 0, pHeadData->m_chanelPower[0], pHeadData->m_chanelAmplifier);
	setCurrentDACValue(pHeadData->m_id, 1, pHeadData->m_chanelPower[1], pHeadData->m_chanelAmplifier);
	setCurrentDACValue(pHeadData->m_id, 2, pHeadData->m_chanelPower[2], pHeadData->m_chanelAmplifier);
	setCurrentDACValue(pHeadData->m_id, 3, pHeadData->m_chanelPower[3], pHeadData->m_chanelAmplifier);
	setVoltageDACValue(pHeadData->m_id, noChannelPower ? 0 : pHeadData->m_voltage);


	if (pHeadData->m_chanelPower[0] > 5000)
		pHeadData->m_chanelStatusFlags[0] = 1;
	else
		pHeadData->m_chanelStatusFlags[0] = 0;

	if (pHeadData->m_chanelPower[1] > 5000)
		pHeadData->m_chanelStatusFlags[1] = 1;
	else
		pHeadData->m_chanelStatusFlags[1] = 0;

	if (pHeadData->m_chanelPower[2] > 5000)
		pHeadData->m_chanelStatusFlags[2] = 1;
	else
		pHeadData->m_chanelStatusFlags[2] = 0;

	if (pHeadData->m_chanelPower[3] > 5000)
		pHeadData->m_chanelStatusFlags[3] = 1;
	else
		pHeadData->m_chanelStatusFlags[3] = 0;

	//outputString(&UartDbg, "Enable pulse module\r\n");

	if (pHeadData->m_id == 0)
		OC3CONbits.OCM = 0x4;	// enable single pulse mode
	else
		OC4CONbits.OCM = 0x4;	// enable single pulse mode

	//restore input trigger settings
	Trig1_IN_Enabled = old_Trig1_IN_Enabled;
	Trig2_IN_Enabled = old_Trig2_IN_Enabled;

	outputString(&UartDbg, "Program output head done\r\n");
}

//-----------------------------------------------------------------------------------------
// Here is our lovely interrupt function that processes trigger input change - the name is unimportant.
//-----------------------------------------------------------------------------------------
static void interrupt isr_InputChanged(void) @INCH_VCTR
{
	unsigned char rd5  = _RD5;
	unsigned char rd6  = _RD6;
	unsigned char rb15 = _RB15;


	// trig 1 is fired
	if (Trig1_IN != rd5 && Trig1_IN_Enabled) {
//		outputString(&UartDbg, " rd5 = ");
//		outputIntAsString(&UartDbg, rd5);

		if (Trig1_Timer2_Enabled && rd5 == Trig1_Timer2_Edge && T2CONbits.TON == 0) {


			//outputString(&UartDbg, " rd5 = ");
			//outputIntAsString(&UartDbg, rd5);
			//outputString(&UartDbg, " Trig1_Timer2_Edge = ");
			//outputIntAsString(&UartDbg, Trig1_Timer2_Edge);
			//outputString(&UartDbg, " PR2 = ");
			//outputIntAsString(&UartDbg, PR2);
			//outputString(&UartDbg, " A=1\r\n");
			OC3CONbits.OCM = 0x4;	// enable single pulse mode
			T2CONbits.TON = 1;	// turn ON timer


		}

		if (Trig1_Timer3_Enabled && rd5 == Trig1_Timer3_Edge && T3CONbits.TON == 0) {
    		T3CONbits.TON = 1;	// turn ON timer
//			outputString(&Uart1, "T3CONbits.TON = 1 = 1\r\n");
			outputString(&UartDbg, "B=1\r\n");
		}



//		outputString(&Uart1, "\r\n");

	}

	// store the current state of rd5
	Trig1_IN = rd5;


	// trig 2 is fired
	if (Trig2_IN != rd6  && Trig2_IN_Enabled) {
//		outputString(&Uart1, "rd6 = ");
//		outputIntAsString(&Uart1, rd6);

		if (Trig2_Timer2_Enabled && rd6 == Trig2_Timer2_Edge && T2CONbits.TON == 0) {
//			outputString(&Uart1, "rd6 PR2 = ");
//			outputIntAsString(&Uart1, PR2);
//
//			outputString(&Uart1, " Trig2_IN = ");
//			outputIntAsString(&Uart1, Trig2_IN);


    		T2CONbits.TON = 1;	// turn ON timer

		}

//		outputString(&Uart1, "\r\n");
		if (Trig2_Timer3_Enabled && rd6 == Trig2_Timer3_Edge && T3CONbits.TON == 0) {
    		T3CONbits.TON = 1;	// turn ON timer
//			outputString(&Uart1, "RD6 T3CONbits.TON = 1 = 1\r\n");
		}

		// store the current state of rd6
		Trig2_IN = rd6;
	}

	//!!!if (Trig1_IN_Loopback != rb15) {
	//!!!	Trig1_IN_Loopback = rb15;
	//!!!	//used for voltage head control RB11	= !Trig1_IN_Loopback;	// inverted state
	//!!!}

	CNIF = 0;	// important to clear this bit

	//outputString(&UartDbg, "Z\r\n");
}

//-----------------------------------------------------------------------------------------
// Here is our lovely interrupt function that Time 2 period change - the name is unimportant.
//-----------------------------------------------------------------------------------------
static void interrupt isr_Timer2(void) @T2_VCTR
{
	T2CONbits.TON = 0;		// disable the timer 2

//	OC3CONbits.OCM = 0;		// disable single pulse mode
//	if (OC4CONbits.OCTSEL == 0)
//		OC4CONbits.OCM = 0;	// disable single pulse mode

	TMR2 = 0;				// reset the counter for timer 2

//	OC3CONbits.OCM = 0x4;	// enable single pulse mode

	// if timer 2 was selected as a source for output 2 then reenable it too
//	if (OC4CONbits.OCTSEL == 0)
//		OC4CONbits.OCM = 0x4;	// enable single pulse mode


//	AdvanceToTheNextBank = 1;

//	outputString(&UartDbg, "A=0\n");

	T2IF = 0;	// important to clear this bit
}

//-----------------------------------------------------------------------------------------
// Here is our lovely interrupt function that Time 2 period change - the name is unimportant.
static void interrupt isr_Timer3(void) @T3_VCTR
{
	T3CONbits.TON = 0;		// disable the timer 3
	OC4CONbits.OCM = 0;	// enable single pulse mode????
	TMR3 = 0;					// reset the counter for timer 3
	OC4CONbits.OCM = 0x4;	// enable single pulse mode???

	_LATD3 = 0;//???
	outputString(&UartDbg, "B=0\r\n");

	T3IF = 0;	// important to clear this bit
}


//-----------------------------------------------------------------------------------------
void resetAllDACs()
{
	short i = 0;
	for (i = 0; i < 8; ++i) {
		setCurrentDACValue(0, i, 0, 0);  //Reset current for head 1 on all channels
		setCurrentDACValue(1, i, 0, 0);  //Reset current for head 2 on all channels
	}
	setVoltageDACValue(0, 0); //Reset voltage for head 1
	setVoltageDACValue(1, 0); //Reset voltage for head 2
}


//-----------------------------------------------------------------------------------------
void fireSoftTrigger(unsigned char aTriggerId)
{
	unsigned char RD5_Old;
	unsigned char RD6_Old;

	RD5_Old = RD5;
	RD6_Old = RD6;

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
			_TRISD5 = 0;				// enable RD5 as input RD5 is shared with CN14
			#asm
			nop
			nop
			nop
			#endasm
    		RD5 = !RD5_Old;
//			outputString(&Uart1, "RD5 = Trig1_Timer2_Edge : ");
//			outputIntAsString(&Uart1, Trig1_Timer2_Edge);
//			outputString(&Uart1, " RD5_Old = ");
//			outputIntAsString(&Uart1, RD5_Old);
//			outputString(&Uart1, "\r\n");
		}

		if (Trig1_Timer3_Enabled && T3CONbits.TON == 0) {
			_TRISD5 = 0;				// enable RD5 as input RD5 is shared with CN14
			#asm
			nop
			nop
			nop
			#endasm
    		RD5 = !RD5_Old ;
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
		RD5 = RD5_Old;
		_TRISD5 = 1;				// enable RD5 as input RD5 is shared with CN14
	}
//
//	if (aTriggerId & 0x02) {
//		RD6 = RD6_Old;
//		_TRISD6 = 0;				// enable RD6 as input RD56
//	}

	#asm
	nop
	nop
	nop
	#endasm
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
		programBank(&BankInfo[ BankSequence[BankSequencePos] ]);
	}

	//restore the input trigger settings
	Trig1_IN_Enabled = old_Trig1_IN_Enabled;
	Trig2_IN_Enabled = old_Trig2_IN_Enabled;
}

//-----------------------------------------------------------------------------------------
void setActiveBank(unsigned short activeBank)
{
	//TODO implement
}