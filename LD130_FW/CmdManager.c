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
	static char RXBuffer[MAX_VALUES_IN_PATTERN*STRING_VALUE_LENGTH + 1];
	// position of the last character in RXBuffer
	static unsigned short iPos;
	// store the begin and the end
	static unsigned short msgCmdEnd = 0;	// the end of the message

	static unsigned char ch = 0;			// the current character being processed

	Start_UART1();

	for (;;)
    {
        OS_Csem_Wait (Uart1_Msg);	// now wait here untill any data arrives to UART

		// store the pointers to the beginig and and of the current message
		// we will be working with the local variables
		// the idea is to copy the end of the message to the local variable and
		// then work with it, while
		// there can be another message arriving to the UART
		msgCmdEnd = Uart1.m_RXTail;

		// clear the pointer to the beginning of a buffer
		iPos = 0;

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
			if (isalnum (ch) || ch == ','|| ch == '.') {
				RXBuffer[iPos] = ch;
				++iPos;
			}

		} while (ch != '\n' && iPos < (sizeof(RXBuffer)-1));

		// terminate the string
		RXBuffer[iPos] = 0;

		// parse the incoming text
		outputString_UART1(">");
		outputString_UART1(RXBuffer);
		outputString_UART1(">");
		if (ParseSentence(RXBuffer)) {
			outputString_UART1(GetCmdName());
			outputString_UART1(":OK");
		} else {
			outputString_UART1(RXBuffer);
			outputString_UART1(":ERR");
		}
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


