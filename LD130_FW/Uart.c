////////////////////////////////////////////////////////////////////
//
// 	Alexandr Filenkov 2010
//
////////////////////////////////////////////////////////////////////


#include <p30fxxxx.h>
#include "osa.h"
#include "common.h"
#include "Uart.h"

unsigned short Baud_Rate(unsigned long A_BAUD_) { return ((unsigned short)(A_FOSC_/(16UL * A_BAUD_) -1)); }

// prototype functions
void outputIntAsHexString(int aPort, unsigned long aValue);
void outputIntAsString(int aPort, unsigned long aValue);

TUartBuff Uart1;

// the counting message semaphore for UART 1
// every time we detect the end of message
// we will increment the semaphore
OST_CSEM  Uart1_Msg;



/********************************************************************************
 *                                                                              *
 *  Function:       Task_UART1                                                  *
 *                                                                              *
 *------------------------------------------------------------------------------*
 *                                                                              *
 *  description:    Initialize the hardware for UART 1 and then processes the 	*
 *  				messages as they arrive 									*
 *  																			*
 *  parameters:     void                                                        *
 *                                                                              *
 *  on return:      void                                                        *
 *                                                                              *
 *                                                                              *
 ********************************************************************************/
void Start_UART1 (void)
{
	unsigned long Uart1_Baud_Rate;	// stored rate for UART 1

	// create a semaphore
	OS_Csem_Create(Uart1_Msg);


	//-----------------------------------------------------------------------------------------
	// Initializes UART 1 with:
	// 8 bit data
	// no parity
	// 1 stop bit
	// 9600 baud
	//
	// The function works with UART specific registers, for UART 2
	// it has to be changed
	//-----------------------------------------------------------------------------------------

	U1MODEbits.UARTEN = 0;          // disable UART 1

	// Once enabled, the UxTX and UxRX pins are configured as an
	// output and an input respectively, overriding the TRIS
	// and LAT register bit settings for the corresponding I/O
	// port pins. The UxTX pin is at logic ‘1’ when no
	// transmission is taking place.

	// PDSEL<1:0> - Parity and data selection bits
	//      00 – 8-bit data, no parity
	//      01 – 8-bit data, even parity
	//      10 – 8-bit data, odd parity
	//      11 – 9-bit data, no parity
	U1MODEbits.PDSEL = 0;

	// STOP selection bit:
	//      STSEL=0 one STOP bit
	//      STSEL=1 two STOP bits
	U1MODEbits.STSEL=0;

	// Transmission interrupt mode selection bit:
	//      UTXISEL=0 interrupt when a character is tranferred to the transmit shift register
	//      UTXISEL=1 interrupt when a character is tranferred to the transmit shift register and the transmit buffer becomes empty
	U1STAbits.UTXISEL = 1;

	//URXISEL<1:0> - Receive interrupt mode selection bits
	//      0x – interrupt flag bit is set when a charatcer is received
	//      10 - interrupt flag bit is set when receive buffer is ? full (3 locations full)
	//      11 - interrupt flag bit is set when receive buffer is full (all 4 locations full)
	U1STAbits.URXISEL =0;


	// if the SW1 is signaled
	// means that we need to reset UART 1 baud rate to 0
//	if (RF0) {
//		// we check SW1 switch for some time before
//		// we confirm the reset status
//		unsigned short i;
//		for (i = 0; i < 0xFFFF; ++i) {
//			CLRWDT();
//			if (!RF0)
//				break;
//		}
//		if (i == 0xFFFF) {
//			// save 9600 as a baud rate in ERPOM
//		}
//	}

	Uart1_Baud_Rate = 9600;

	// TBD read eprom
	U1BRG = Baud_Rate(Uart1_Baud_Rate);

	U1STAbits.OERR = 0;
	U1STAbits.FERR = 0;
	U1STAbits.PERR = 0;

	U1MODEbits.UARTEN = 1;      // enable UART 1, this will configure TX as output and RX as input

	U1STAbits.UTXEN = 1;        // enable TX function, should be after U1_UARTEN

	_U1TXIE = 0;             	// global enable/disable TX interrupt
	_U1RXIE = 1;             	// gloabl enable/disable RX interrupt

	Uart1.m_RXHead = 0;        	// init RX head
	Uart1.m_RXTail = 0;        	// init RX tail
	Uart1.m_UartID = 1;        	// the ID is 1
	Uart1.m_baud_rate = Uart1_Baud_Rate;

}


//-----------------------------------------------------------------------------------------
// Here is our lovely interrupt function on character receive - the name is important.
//
// This interrupt is called for every character received from UART
// It checks for overflow errors and then reads data from UART hardware buffer to
// our software circular buffer
//
//-----------------------------------------------------------------------------------------

void _ISR __attribute__ ((auto_psv)) _U1RXInterrupt(void)
{
	if (U1STAbits.OERR) {
		U1STAbits.OERR = 0;
	} else if (!U1STAbits.FERR) {

		// now start reading the data from UART
		do {
			// while there is any data in the buffer
			// read it completely
			while (U1STAbits.URXDA) {
				unsigned char ch;

				// read the character from FIFO
				ch = U1RXREG;

				Uart1.m_RXBuffer[Uart1.m_RXTail] = ch;

				// check if it is endl symbol, if yes then increment message semaphore counter
				if (ch == 0x0d)
				{
					// signal the processing task that the message is ready
					OS_Csem_Signal_I(Uart1_Msg);
				}

				// increment the tail
				++Uart1.m_RXTail;

				// if we reach the end of the buffer set it to 0
				if (Uart1.m_RXTail >= RX_BUFFER_MAX)
					Uart1.m_RXTail = 0;
			}

		} while(U1STAbits.URXDA);
	}
	_U1RXIF = 0;
}


//-----------------------------------------------------------------------------------------
// outputs the char provided to UART
//-----------------------------------------------------------------------------------------
void outputChar_UART1(char ch)
{
	while (U1STAbits.UTXBF) {
		ClrWdt();
	}
	U1TXREG = ch;
}

//-----------------------------------------------------------------------------------------
// outputs the string provided to UART
//-----------------------------------------------------------------------------------------
void outputString_UART1(const char* pCh)
{
	for (; pCh && *pCh; ++pCh) {
		outputChar_UART1(*pCh);
	}
}

//-----------------------------------------------------------------------------------------
// outputs the integer as a hexadecimal string to UART
//-----------------------------------------------------------------------------------------
void outputIntAsHexString_UART1(unsigned long aValue)
{
	outputIntAsHexString(1, aValue);
}

//-----------------------------------------------------------------------------------------
// outputs the integer as a decimal string to UART
//-----------------------------------------------------------------------------------------
void outputIntAsString_UART1(unsigned long aValue)
{
	outputIntAsString(1, aValue);
}


/********************************************************************************
 *                                                                              *
 *  Function:       UART2                                                       *
 *                                                                              *
 ********************************************************************************/

TUartBuff Uart2;

// the counting message semaphore for UART 1
// every time we detect the end of message
// we will increment the semaphore
OST_CSEM  Uart2_Msg;



/********************************************************************************
 *                                                                              *
 *  Function:       Task_UART2                                                  *
 *                                                                              *
 *------------------------------------------------------------------------------*
 *                                                                              *
 *  description:    Initialize the hardware for UART 1 and then processes the 	*
 *  				messages as they arrive 									*
 *  																			*
 *  parameters:     void                                                        *
 *                                                                              *
 *  on return:      void                                                        *
 *                                                                              *
 *                                                                              *
 ********************************************************************************/
void Start_UART2 (void)
{
	unsigned long Uart2_Baud_Rate;	// stored rate for UART 1

	OS_Csem_Create(Uart2_Msg);


	//-----------------------------------------------------------------------------------------
	// Initializes UART 1 with:
	// 8 bit data
	// no parity
	// 1 stop bit
	// 9600 baud
	//
	// The function works with UART specific registers, for UART 2
	// it has to be changed
	//-----------------------------------------------------------------------------------------

	U2MODEbits.UARTEN = 0;          // disable UART 1

	// Once enabled, the UxTX and UxRX pins are configured as an
	// output and an input respectively, overriding the TRIS
	// and LAT register bit settings for the corresponding I/O
	// port pins. The UxTX pin is at logic ‘1’ when no
	// transmission is taking place.

	// PDSEL<1:0> - Parity and data selection bits
	//      00 – 8-bit data, no parity
	//      01 – 8-bit data, even parity
	//      10 – 8-bit data, odd parity
	//      11 – 9-bit data, no parity
	U2MODEbits.PDSEL = 0;

	// STOP selection bit:
	//      STSEL=0 one STOP bit
	//      STSEL=1 two STOP bits
	U2MODEbits.STSEL=0;

	// Transmission interrupt mode selection bit:
	//      UTXISEL=0 interrupt when a character is tranferred to the transmit shift register
	//      UTXISEL=1 interrupt when a character is tranferred to the transmit shift register and the transmit buffer becomes empty
	U2STAbits.UTXISEL = 1;

	//URXISEL<1:0> - Receive interrupt mode selection bits
	//      0x – interrupt flag bit is set when a charatcer is received
	//      10 - interrupt flag bit is set when receive buffer is ? full (3 locations full)
	//      11 - interrupt flag bit is set when receive buffer is full (all 4 locations full)
	U2STAbits.URXISEL =0;


	// if the SW1 is signaled
	// means that we need to reset UART 1 baud rate to 0
//	if (RF0) {
//		// we check SW1 switch for some time before
//		// we confirm the reset status
//		unsigned short i;
//		for (i = 0; i < 0xFFFF; ++i) {
//			CLRWDT();
//			if (!RF0)
//				break;
//		}
//		if (i == 0xFFFF) {
//			// save 9600 as a baud rate in ERPOM
//		}
//	}

	Uart2_Baud_Rate = 9600;

	// TBD read eprom
	U2BRG = Baud_Rate(Uart2_Baud_Rate);

	U2STAbits.OERR = 0;
	U2STAbits.FERR = 0;
	U2STAbits.PERR = 0;

	U2MODEbits.UARTEN = 1;      // enable UART 1, this will configure TX as output and RX as input

	U2STAbits.UTXEN = 1;        // enable TX function, should be after U1_UARTEN

	_U2TXIE = 0;             	// global enable/disable TX interrupt
	_U2RXIE = 1;             	// gloabl enable/disable RX interrupt

	Uart2.m_RXHead = 0;        	// init RX head
	Uart2.m_RXTail = 0;        	// init RX tail
	Uart2.m_UartID = 2;        	// the ID is 2
	Uart2.m_baud_rate = Uart2_Baud_Rate;

}


//-----------------------------------------------------------------------------------------
// Here is our lovely interrupt function on character receive - the name is important.
//
// This interrupt is called for every character received from UART
// It checks for overflow errors and then reads data from UART hardware buffer to
// our software circular buffer
//
//-----------------------------------------------------------------------------------------

void _ISR __attribute__ ((auto_psv)) _U2RXInterrupt(void)
{
	if (U2STAbits.OERR) {
		U2STAbits.OERR = 0;
	} else if (!U2STAbits.FERR) {

		// now start reading the data from UART
		do {
			// while there is any data in the buffer
			// read it completely
			while (U2STAbits.URXDA) {
				unsigned char ch;

				// read the character from FIFO
				ch = U2RXREG;

				Uart2.m_RXBuffer[Uart2.m_RXTail] = ch;

				// check if it is endl symbol, if yes then increment message semaphore counter
				if (ch == 0x0d)
				{
					// signal the processing task that the message is ready
					OS_Csem_Signal_I(Uart2_Msg);
				}

				// increment the tail
				++Uart2.m_RXTail;

				// if we reach the end of the buffer set it to 0
				if (Uart2.m_RXTail >= RX_BUFFER_MAX)
					Uart2.m_RXTail = 0;
			}

		} while(U2STAbits.URXDA);
	}
	_U2RXIF = 0;
}

//-----------------------------------------------------------------------------------------
// outputs the char provided to UART
//-----------------------------------------------------------------------------------------
void outputChar_UART2(char ch)
{
	while (U2STAbits.UTXBF) {
		ClrWdt();
		OS_Yield(); // Unconditional context switching
	}
	U2TXREG = ch;
}

//-----------------------------------------------------------------------------------------
// outputs the string provided to UART
//-----------------------------------------------------------------------------------------
void outputString_UART2(const char* pCh)
{
	for (; pCh && *pCh; ++pCh) {
		outputChar_UART2(*pCh);
	}
}

//-----------------------------------------------------------------------------------------
// outputs the integer as a hexadecimal string to UART
//-----------------------------------------------------------------------------------------
void outputIntAsHexString_UART2(unsigned long aValue)
{
	outputIntAsHexString(2, aValue);
}

//-----------------------------------------------------------------------------------------
// outputs the integer as a decimal string to UART
//-----------------------------------------------------------------------------------------
void outputIntAsString_UART2(unsigned long aValue)
{
	outputIntAsString(2, aValue);
}

////-----------------------------------------------------------------------------------------
//void setBaudRate(TUartBuff* pUart, unsigned long aBaudRate)
//{
//    if (aBaudRate == 0)
//        return ;
//
//    if (aBaudRate > (unsigned long)(115200))
//        return ;
//
//    if (pUart->m_UartID == 1) {
//        U1_UARTEN = 0;          // disable UART 1
//
//        Uart1_Baud_Rate = aBaudRate;
//        U1BRG = Baud_Rate(aBaudRate);
//
//        U1_UARTEN = 1;          // enable UART 1, this will configure TX as output and RX as input
//        U1_UTXEN = 1;           // enable TX function, should be after U1_UARTEN
//        U1TXIE = 0;             // global enable/disable TX interrupt
//        U1RXIE = 1;             // gloabl enable/disable RX interrupt
//    }
//    else if (pUart->m_UartID == 2) {
//        U2_UARTEN = 0;          // disable UART 2
//
//        Uart2_Baud_Rate = aBaudRate;
//        U2BRG = Baud_Rate(aBaudRate);
//
//        U2_UARTEN = 1;          // enable UART 2, this will configure TX as output and RX as input
//        U2_UTXEN = 1;           // enable TX function, should be after U2_UARTEN
//        U2TXIE = 0;             // global enable/disable TX interrupt
//        U2RXIE = 1;             // gloabl enable/disable RX interrupt
//    }
//}

//-----------------------------------------------------------------------------------------
// outputs the integer to the UART provided. The function converts ineteger to a string
// and then	outputs a string to UART
//-----------------------------------------------------------------------------------------
void outputIntAsString(int aPort, unsigned long aValue)
{
	const char* pCh = 0;
	char buf[14];
	buf[13] = 0;

	buf[12] = (aValue % 10) + '0';
	aValue = aValue / 10;

	buf[11] = (aValue % 10) + '0';
	aValue = aValue / 10;

	buf[10] = (aValue % 10) + '0';
	aValue = aValue / 10;

	buf[9] = (aValue % 10) + '0';
	aValue = aValue / 10;

	buf[8] = (aValue % 10) + '0';
	aValue = aValue / 10;

	buf[7] = (aValue % 10) + '0';
	aValue = aValue / 10;

	buf[6] = (aValue % 10) + '0';
	aValue = aValue / 10;

	buf[5] = (aValue % 10) + '0';
	aValue = aValue / 10;

	buf[4] = (aValue % 10) + '0';
	aValue = aValue / 10;

	buf[3] = (aValue % 10) + '0';
	aValue = aValue / 10;

	buf[2] = (aValue % 10) + '0';
	aValue = aValue / 10;

	buf[1] = (aValue % 10) + '0';
	aValue = aValue / 10;

	buf[0] = (aValue % 10) + '0';


	// strip the leading zeros
	for (pCh = buf; pCh && *pCh == '0'; ++pCh)
		;

	// in case if all are zeros, then still output 0
	if (*pCh == 0){
//		buf[0] = '0';	assume that there is '0' already there
		buf[1] = 0;
		pCh = buf;
	}

	if (aPort == 1)
		outputString_UART1(pCh);
	else
		outputString_UART2(pCh);
}

//-----------------------------------------------------------------------------------------
void outputIntAsHexString(int aPort, unsigned long aValue)
{
	const char* pCh = 0;
	char buf[11];
	unsigned char i;
	buf[10] = 0;

	for (i = 10; i != 0; --i) {
		buf[i-1] = (aValue % 16) + '0';
		if (buf[i-1] > '9') buf[i-1] = buf[i-1] - '9' + 'A' - 1;
		aValue = aValue / 16;
	}

	// strip the leading zeros
	for (pCh = buf; pCh && *pCh == '0'; ++pCh)
		;

	// in case if all are zeros, then still output 0
	if (*pCh == 0){
//		buf[0] = '0';	assume that there is '0' already there
		buf[1] = 0;
		pCh = buf;
	}

	if (aPort == 1)
		outputString_UART1(pCh);
	else
		outputString_UART2(pCh);
}



