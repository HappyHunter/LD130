////////////////////////////////////////////////////////////////////
//
// 	Alexandr Filenkov 2010
//
////////////////////////////////////////////////////////////////////

#ifndef UART_095322
#define UART_095322

#include "common.h"

void Start_UART1 (void);
void Start_UART2 (void);

void outputChar_UART1(char ch);
void outputString_UART1(const char* pCh);
void outputIntAsHexString_UART1(unsigned long aValue);
void outputIntAsString_UART1(unsigned long aValue);
void outputFloatAsString_UART1(double aValue);

void outputChar_UART2(char ch);
void outputString_UART2(const char* pCh);
void outputIntAsHexString_UART2(unsigned long aValue);
void outputIntAsString_UART2(unsigned long aValue);
void outputFloatAsString_UART2(double aValue);


extern OST_CSEM  Uart1_Msg;
extern OST_CSEM  Uart2_Msg;

extern TUartBuff1 Uart1;
extern TUartBuff2 Uart2;

//#define UartDbg Uart2
//
//extern unsigned long Uart1_Baud_Rate;	// stored rate for UART 1
//extern unsigned long Uart2_Baud_Rate;	// stored rate for UART 2
//
//void initUART1(TUartBuff* pUart);
//void initUART2(TUartBuff* pUart);
//
//void setBaudRate(TUartBuff* pUart, unsigned long aBaudRate);
//
//void outputChar(TUartBuff* pUart, char ch);
//void outputString(TUartBuff* pUart, const char* pCh);
//void outputInt(TUartBuff* pUart, unsigned int aValue);
//void outputShort(TUartBuff* pUart, unsigned short aValue);
//void outputData(TUartBuff* pUart, const char* pCh, unsigned long aSize);
//
//
//void outputIntAsString(TUartBuff* pUart, unsigned long aValue);
//void outputIntAsHexString(TUartBuff* pUart, unsigned long aValue);

#endif
