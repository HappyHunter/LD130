////////////////////////////////////////////////////////////////////
//
// 	Alexandr Filenkov 2010
//
////////////////////////////////////////////////////////////////////

#include <p30fxxxx.h>
#include "osa.h"
#include "Common.h"
#include "Uart.h"
#include "Parser.h"

// uses watchdog timer
_FOSC(XT_PLL16 & CSW_FSCM_OFF);
_FWDT(WDT_OFF & WDTPSA_64 & WDTPSB_16);

void Init (void);

void TaskBlink();

#define TEST_TEST_TEST

//-----------------------------------------------------------------------------------------
// reads the character from UART buffer
// and advances the pointer
// if we reach the end of message the function returns 0
// it handles the wrap around of the message buffer
//
// Also it increments the current position of the parser pointer
//-----------------------------------------------------------------------------------------
unsigned char getChFromBuf(TUartBuff* pUart, unsigned short* pMsgBegin, unsigned short* pMsgEnd, unsigned char* pCh)
{
	unsigned char ch = 0;

	// if end of the message
	if (*pMsgBegin == *pMsgEnd)
		return 0;

	// read the character from the buffer
	*pCh = pUart->m_RXBuffer[*pMsgBegin];
	// increment the pointer
	++(*pMsgBegin);

	// check for wrap around
	if (*pMsgBegin >= RX_BUFFER_MAX)
		*pMsgBegin = 0;

	// increment the parser pointer
//	++pUart->m_RXErrorPos;

	return 1;
}

void Task_UART1 (void)
{
	char RXBuffer[MAX_VALUES_IN_PATTERN*STRING_VALUE_LENGTH + 1];
	unsigned short iPos;
	// store the begin and the end
	unsigned short msgBegin = 0;	// the begining of the message
	unsigned short msgEnd = 0;	// the end of the message

	unsigned short msgCmdBegin = 0;	// the begining of the message
	unsigned short msgCmdEnd = 0;		// the end of the message

	unsigned char ch = 0;		// the current character being processed

	Start_UART1();

	for (;;)
    {
        OS_Csem_Wait (Uart1_Msg);	// now wait here untill any data arrives to UART
		iPos = 0;

		// no characters arrived since last check
		if (Uart1.m_RXHead == Uart1.m_RXTail)
			continue;

		// store the pointers to the beginig and and of the current message
		// we will be working with the local variables
		// the idea is to copy the beginig and the end of the message to the local variable and
		// then work with them, while
		// there can be another message arriving to the UART
		msgCmdBegin = msgBegin = Uart1.m_RXHead;
		msgCmdEnd = msgEnd = Uart1.m_RXTail;
		// reset the error flags and error position
//		Uart1.m_RXErrorPos = 0;
//		Uart1.m_RXErrorFlags = 0;

		do {
			if (getChFromBuf(&Uart1, &msgCmdBegin, &msgCmdEnd, &ch)) {
				RXBuffer[iPos] = ch;
			}
		} while (ch != 0 && ch != '\n' && iPos < sizeof(RXBuffer));
		RXBuffer[iPos] = 0;
		outputString_UART1("-->");
		outputString_UART1(RXBuffer);
		outputString_UART1("\r\n");
    }

}
void Task_UART2 (void)
{
	Start_UART2();

	for (;;)
    {
        OS_Csem_Wait (Uart2_Msg);	// now wait here untill any data arrives to UART
    }

}



int main (void)
{
    //------------------------------------------------------------------------------
    //  Init the hardware
    //------------------------------------------------------------------------------
    Init();


    //------------------------------------------------------------------------------
    //  Init the components
    //------------------------------------------------------------------------------
	#ifndef TEST_TEST_TEST
	InitializeParser();
	#endif

    //------------------------------------------------------------------------------
    //  Init RTOS
    //------------------------------------------------------------------------------

    OS_Init();

    //------------------------------------------------------------------------------
    //  Create Tasks
    //------------------------------------------------------------------------------

	// priority 0 - Highest, 7 - lowest
    OS_Task_Create(2, Task_UART1);
    OS_Task_Create(2, Task_UART2);
    OS_Task_Create(2, TaskBlink);


    //------------------------------------------------------------------------------
    //  Enable interrupts and start OS
    //------------------------------------------------------------------------------

    OS_EI();
    OS_Run();

	return 0;
}



/********************************************************************************
 *                                                                              *
 *  Function:       Init                                                        *
 *                                                                              *
 *------------------------------------------------------------------------------*
 *                                                                              *
 *  description:    Initialize all the hardware:      							*
 *                                                                              *
 *  parameters:     void                                                        *
 *                                                                              *
 *  on return:      void                                                        *
 *                                                                              *
 *                                                                              *
 ********************************************************************************/
/*

Pin and register map of dsPIC30F6010A

	A					B							C							D						E					F					G
0						*20/RB0/CN2/AN0/EMUD/PGD								*58/RD0/OC1/EMUC2		*76/RE0/PWM1L		*72/RF0/C1RX		*75/RG0/C2RX
1						*19/RB1/CN3/AN1/EMUC/PGC	4/RC1/T2CK					*61/RD1/OC2/EMUD2		*77/RE1/PWMH1		*73/RF1/C1TX		*74/RG1/C2TX
2						18/RB2/CN4/1/AN2										*62/RD2/OC3				*78/RE2/PWM2L		*42/RF2/U1RX		*47/RG2/SCL
3						17/RB3/CN5/IDX/AN3			5/RC3/T4CK					*63/RD3/OC4				*79/RE3/PWM2H		*41/RF3/U1TX		*46/RG3/SDA
4						16/RB4/CN6/QEA/AN4										66/RD4/CN13/OC5			*80/RE4/PWM3L		*39/RF4/CN17/U2RX
5						15/RB5/CN7/QEB/AN5										*67/RD5/CN14/OC6		*1/RE5/PWM3H		*40/RF5/CN18/U2TX
6						21/RB6/AN6/OCFA											*68/RD6/CN15/OC7		*2/RE6/PWM4L		*45/RF6/SCK1/INT0	*6/RG6/CN8/SCK2
7						*22/RB7/AN7												69/RD7/CN16/UPDN/OC8	*3/RE7/PWM4H		*44/RF7/SDI1		*7/RG7/CN9/SDI2
8						*27/RB8/AN8												54/RD8/IC1				*13/RE8/FLTA/INT1	*43/RF8/EMUD3/SDO1	*8/RG8/CN10/SDO2
9	23/RA9/VREF-		*28/RB9/AN9												55/RD9/IC2				*14/RE9/FLTB/INT2						*10/RG9/CN11/SS2
10	24/RA10/VREF+		*29/RB10/AN10											56/RD10/IC3
11						30/RB11/AN11											57/RD11/IC4
12						33/RB12/AN12
13						34/RB13/AN13				59/RC13/CN1/SOSCI/EMUD1		*65/RD13/CN19/IC6
14	52/RA14/INT3		35/RB14/AN14				60/RC14/CN0/T1CKSOSCO/EMUC1	*37/RD14/CN20/IC7
15	53/RA15/INT4		36/RB15/CN12/OCFB/AN15		50/RC15/CLKO/OSC2			*38/RD15/CN21/IC8
16
*/

/*
	hardware modules and pins in use

	In Circuit Debuger/programmer
	20/RB0/CN2/AN0/EMUD/PGD
	19/RB1/CN3/AN1/EMUC/PGC
	9/MCLR
	12/VDD
	11/VSS


	UART 1:		RF2/42/RX	RF3/41/TX	 RX_LED:72/RF0/C1RX    TX_LED:73/RF1/C1TX
	UART 2:		RF5/40/TX	RF4/39/RX	 RX_LED:75/RG0/C2RX	   TX_LED:74/RG1/C2TX

	SPI 1:		RF6/45/SCK1		RF7/44/SDI1		RF8/43/SDO1
	SPI 1 Chip Select: RB7/22 Chip 1, RB8/27 Chip 2, RB9/28 Chip 3, RB10/29 Chip 4

	SPI2:		6/RG6/CN8/SCK2		7/RG7/CN9/SDI2		8/RG8/CN10/SDO2
	SPI 1 Chip Select: 	13/RE8/FLTA/INT1 CHIP1,		14/RE9/FLTB/INT2 CHIP2,		10/RG9/CN11/SS2 CHIP3

	I2C:   47/RG2/SCL		46/RG3/SDA

	Trigger 1:	RD5/67/CN14 Input  RD2/OC3/62 - output
	Trigger 2:	RD6/68/CN15	Input  RD3/OC4/63 - output

	SW1:		58/RD0/OC1/EMUC2	Reset UART 1 Baud Rate
	SW2:		61/RD1/OC2/EMUD2	Unused

	---------------------- TBD -----------------------
	Bank Select: RE0/76 bit 0 		RE1/77 Bit 1	 RD13/65/ Set Signal (TTL)
	Reset Bank Sequence: RD14/37 (TTL)
	Next Bank Sequence:  RD15/38 (TTL)

	Reserved

	78/RE2/PWM2L
	79/RE3/PWM2H
	80/RE4/PWM3L
	1/RE5/PWM3H
	2/RE6/PWM4L
	3/RE7/PWM4H


*/

void Init (void)
{
	_ADON = 0;			// disable analog module
	ADPCFG = 0xFFFF;	// all pins are digital input/ouput pins

	// pins Init section
	//----------------------------------------
	TRISE = 0;				// use all pins as output
	TRISD = 0;				// use all pins as output
	TRISF = 0;				// use all pins as output
	_TRISF0  = 1;			// Input Switch 1
	_TRISF1  = 1;			// Input Switch 2

	_LATB11 = 0;			//voltage control head 1 latched pin data: voltage off
	_LATB12 = 0;			//voltage control head 2 latched pin data: voltage off
	_TRISB11  = 0;			//voltage control head 1 set pin as output
	_TRISB12  = 0;			//voltage control head 1 set pin as output
}


void TaskBlink()
{
	unsigned short iPeriod = 0;
	unsigned char iPeriod2 = 0;

	_ADON = 0;			// disable analog module
	ADPCFG = 0xFFFF;	// all pins are digital input/ouput pins

	// pins Init section
	//----------------------------------------
	TRISE = 0;				// use all pins as output
	TRISD = 0;				// use all pins as output

	_TRISD4 = 0;
	_TRISD5 = 0;
	_LATD5 = 0;
	_LATD4 = 0;


	for (;;) {
		ClrWdt();
		if (++iPeriod == 0) {
			_LATD5 = !_LATD5;
			if (++iPeriod2 > 20) {
				_LATD4 = !_LATD4;
				iPeriod2 = 0;
//				outputString_UART1("Hello\r\n");
			}
		}
	}

}

