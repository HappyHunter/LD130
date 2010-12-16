////////////////////////////////////////////////////////////////////
//
// 	Alexandr Filenkov 2008
//
////////////////////////////////////////////////////////////////////


#ifndef Common_Types_090612
#define Common_Types_090612

//-----------------------------------------------------------------------------------------
// External oscillator 7.3728Mhz
// We use PLL 16 that gives us FOSC = 117.9648MHZ
// The system clock is divided by 4 to yield the internal instruction cycle clock, FCY=FOSC/4 29.4912MHZ
// FCY = 29.4912MHZ
//
// DO NOT FORGET TO CHANGE IT ACCORDING TO YOUR CRYSTAL CLOCK
//-----------------------------------------------------------------------------------------
// 7.37Mhz crystal in 16X PLL mode
#define A_FOSC_ 29491250UL

//-----------------------------------------------------------------------------------------
// the software version
//-----------------------------------------------------------------------------------------
#define VERSION_MAJOR 1
#define VERSION_MINOR 0
#define VERSION_BUILD 1

//-----------------------------------------------------------------------------------------
// the UART receiving buffer
//-----------------------------------------------------------------------------------------
#define RX_BUFFER_MAX 1024
typedef struct tag_UARTBuff
{
	unsigned short	m_RXHead;					// pointer to the head of the message
	unsigned short	m_RXTail;					// pointer to the end of the receiving buffer
	unsigned long 	m_baud_rate;				// stored current baud rate for the UART
//	unsigned long	m_timeout;					// when this counter reaches FFFF FFFF all the data in the buffer will be deleted
	unsigned short	m_UartID;					// the ID of the UART that handles this buffer
												//
	unsigned char 	m_RXBuffer[RX_BUFFER_MAX];	// the circular buffer where we will store the incoming characters from UART

} TUartBuff;

#if 1
#define DbgOut(X) outputString_UART2(X)
#else
#define DbgOut(X)
#endif

#endif
