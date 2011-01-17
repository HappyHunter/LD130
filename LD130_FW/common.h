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

#define USE_30_MHZ

#ifdef USE_30_MHZ
	// 7.37Mhz crystal in 16X PLL mode
	#define A_FOSC_ 29491250UL
#else
	// 7.37Mhz crystal in 8X PLL mode
	#define A_FOSC_ 14741250UL
#endif

#define A_BAUDE_RATE 9600

//-----------------------------------------------------------------------------------------
// the software version
//-----------------------------------------------------------------------------------------
#define VERSION_MAJOR 1
#define VERSION_MINOR 0
#define VERSION_BUILD 1

//-----------------------------------------------------------------------------------------
// the UART1 receiving buffer
//-----------------------------------------------------------------------------------------
typedef struct tag_UARTBuff1
{
	unsigned short	m_RXHead;					// pointer to the head of the message
	unsigned short	m_RXTail;					// pointer to the end of the receiving buffer
	unsigned long 	m_baud_rate;				// stored current baud rate for the UART
//	unsigned long	m_timeout;					// when this counter reaches FFFF FFFF all the data in the buffer will be deleted
	unsigned short	m_UartID;					// the ID of the UART that handles this buffer
												//
	#define RX1_BUFFER_MAX 1024
	unsigned char 	m_RXBuffer[RX1_BUFFER_MAX];	// the circular buffer where we will store the incoming characters from UART

} TUartBuff1;


//-----------------------------------------------------------------------------------------
// the UART2 receiving buffer
// To minimize memory usage it uses shorter version of RX buffer
//-----------------------------------------------------------------------------------------
typedef struct tag_UARTBuff2
{
	unsigned short	m_RXHead;					// pointer to the head of the message
	unsigned short	m_RXTail;					// pointer to the end of the receiving buffer
	unsigned long 	m_baud_rate;				// stored current baud rate for the UART
//	unsigned long	m_timeout;					// when this counter reaches FFFF FFFF all the data in the buffer will be deleted
	unsigned short	m_UartID;					// the ID of the UART that handles this buffer
												//
	#define RX2_BUFFER_MAX 512
	unsigned char 	m_RXBuffer[RX2_BUFFER_MAX];	// the circular buffer where we will store the incoming characters from UART

} TUartBuff2;





#if 1
#define DbgOut(X) outputString_UART2(X)
#define DbgOutInt(X) outputIntAsString_UART2(X)
#else
#define DbgOut(X)
#define DbgOutInt(X)
#endif

#endif
