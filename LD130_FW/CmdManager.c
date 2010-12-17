////////////////////////////////////////////////////////////////////
//
// 	Alexandr Filenkov 2010
//
////////////////////////////////////////////////////////////////////

#include <p30fxxxx.h>
#include "CmdManager.h"
#include "osa.h"
#include "Common.h"
#include "Uart.h"
#include "Parser.h"
#include <stdio.h>
#include <ctype.h>

//------------------------------------------------------------------------------
void Task_UART1 (void)
{
	char RXBuffer[MAX_VALUES_IN_PATTERN*STRING_VALUE_LENGTH + 1];
	unsigned short iPos;
	// store the begin and the end
	unsigned short msgCmdEnd = 0;		// the end of the message

	unsigned char ch = 0;		// the current character being processed

	Start_UART1();

	for (;;)
    {
        OS_Csem_Wait (Uart1_Msg);	// now wait here untill any data arrives to UART

		// clear the pointer to the beginning of a buffer
		iPos = 0;

		// store the pointers to the beginig and and of the current message
		// we will be working with the local variables
		// the idea is to copy the beginig and the end of the message to the local variable and
		// then work with them, while
		// there can be another message arriving to the UART
		msgCmdEnd = Uart1.m_RXTail;

		do {
			// if end of the message
			if (Uart1.m_RXHead == msgCmdEnd)
				break;

			// read the character from the buffer
			ch = Uart1.m_RXBuffer[Uart1.m_RXHead];

			// increment the pointer
			++Uart1.m_RXHead;

			// check for wrap around
			if (Uart1.m_RXHead >= RX_BUFFER_MAX)
				Uart1.m_RXHead = 0;

			// only alphanumeric characters or comma ',' or period '.'
			if (isalnum (ch) || ch == ','|| ch == '.')
				RXBuffer[iPos++] = ch;

		} while (ch != '\n' && iPos < sizeof(RXBuffer));

		// terminate the string
		RXBuffer[iPos] = 0;

		// echo it back for debug
		outputString_UART1("-->");
		outputString_UART1(RXBuffer);
		outputString_UART1("\r\n");
    }

}

//------------------------------------------------------------------------------
void Task_UART2 (void)
{
	Start_UART2();

	for (;;)
    {
        OS_Csem_Wait (Uart2_Msg);	// now wait here untill any data arrives to UART
    }

}


