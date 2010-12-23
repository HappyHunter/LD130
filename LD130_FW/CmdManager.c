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
#include "HardwareController.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>


// the data variable to store hardware bank information
extern TBankInfo theBankInfo[4];
// the data variables to store the status of the head programming
extern TBankHeadStatus theHeadStatus[2];

//Command Patterns
const char * const ErrorStrings[] __attribute__ ((space(auto_psv))) =
{
	"",
	"ERR,00000001,outputId,Must be between 1 and 2\r\n",
	"ERR,00000002,voltage,Must be between 0 and 10000 Volts\r\n",
	"ERR,00000003,powerChanel1,Must be between 0 and 10000\% \r\n",
	"ERR,00000004,powerChanel2,Must be between 0 and 10000\% \r\n",
	"ERR,00000005,powerChanel3,Must be between 0 and 10000\% \r\n",
	"ERR,00000006,powerChanel4,Must be between 0 and 10000\% \r\n",
	"ERR,00000007,triggerEdge,Must be 0,1 or 2\r\n",
	"ERR,00000008,triggerId,Must be 1,2 or 3\r\n",
	"ERR,00000009,chanelAmplifier,Must be between 1 and 2\r\n",
};


void sendVersionReply();

void setHeadData();


void InitCmdManager (void)
{
    //------------------------------------------------------------------------------
    //  Init the components
    //------------------------------------------------------------------------------
	InitializeParser();

    //------------------------------------------------------------------------------
    //  Init hardware bank infos
    //------------------------------------------------------------------------------
	InitAllBankInfo();

}

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
			if (Uart1.m_RXHead >= RX1_BUFFER_MAX)
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
		if (ParseSentence(RXBuffer)) {
			DbgOut("\r\n>>CMD='");
			DbgOut(GetCmdName());
			DbgOut("'");


			if (strcmp(GetCmdName(), "getver") == 0) {
				sendVersionReply();
				continue;

			} else
			if (strcmp(GetCmdName(), "sethdata") == 0) {
				setHeadData();
				continue;

			}


			outputString_UART1(GetCmdName());
			outputString_UART1(":OK");
			outputString_UART1("\r\n");
		} else {
			outputString_UART1(RXBuffer);
			outputString_UART1(":ERR");
			outputString_UART1("\r\n");
		}
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


/**
* sendVersionReply() sends back the firmware version info
*
* Message input: "getver"
*
* Output: "Major.Minor.Build\r\n"
*
*/
//------------------------------------------------------------------------------
void sendVersionReply()
{
	outputString_UART1("version,");
	outputIntAsString_UART1(VERSION_MAJOR);
	outputChar_UART1(',');
	outputIntAsString_UART1(VERSION_MINOR);
	outputChar_UART1(',');
	outputIntAsString_UART1(VERSION_BUILD);
	outputString_UART1("\r\n");
}

/**
* setHeadData() programs the hardware with the parameters
* specified. It will use bank ID = 1 (index = 0) to store the
* parameters. If the current bank ID is not 1 then it will also
* select BankId=1 as current
*
*  Message input:
*  		"sethdata,outputId,voltage,powerChanel1,powerChanel2,powerChanel3,powerChanel4,strobeDelay,strobeWidth,triggerEdge,triggerId,chanelAmplifier"
*
*	unsigned short	outputId;		// 1 - head 1, 2 - Head 2
*  	unsigned short	voltage;		// 0 - 100 Volts
*  	unsigned short	powerChanel1; 	// 0 - 100 00% with fixed decimal point at 2 digits
*  	unsigned short	powerChanel2; 	// for example the power of 35.23% will be sent as 3523
*  	unsigned short	powerChanel3; 	// the power of 99.00% will be sent as 9900
*  	unsigned short	powerChanel4; 	// the power of 100.00% will be sent as 10000
*  	unsigned long	strobeDelay;	// the delay of outcoming light strobe in microseconds
*  	unsigned long	strobeWidth;	// the duration of outcoming light strobe in microseconds
*  	unsigned short	triggerEdge;	// the edge of incoming trigger to start counting, 0 - raising, 1 - falling, 2 - DC mode
*  	unsigned short	triggerId;		// the ID of the trigger this output head will trigger on. 1 - Trigger 1, 2 - Trigger 2, 3 - Trigger 1 or 2
*  	unsigned short	chanelAmplifier;// the amplification value 1-5
*
*
*  Output:
*    "hstatus,statusChanel1,statusChanel2,statusChanel3,statusChanel4\r\n"
*
*/
//------------------------------------------------------------------------------
void setHeadData()
{
	TBankHeadData theHeadData;

	unsigned short theOutputId;
	theOutputId = atoi(GetValueByName("outputId"));
	if (theOutputId < 1 || theOutputId > 2 || !IsValidInteger("outputId")) {
		outputString_UART1(ErrorStrings[1]);
		return ;
	}

	// copy the data into our temp variable
	// with set head data we will always use bankID=1 (index 0)
	theHeadData = theBankInfo[0].m_output[theOutputId-1];


	theHeadData.m_voltage = atoi(GetValueByName("voltage"));		// 0 - 100 Volts
	if (!IsValidInteger("voltage") || theHeadData.m_voltage > 10000) {
		outputString_UART1(ErrorStrings[2]);
		return ;
	}

	theHeadData.m_powerChanel1 = atoi(GetValueByName("powerChanel1"));		// 0 - 100 Volts
	if (!IsValidInteger("powerChanel1") || theHeadData.m_powerChanel1 > 10000) {
		outputString_UART1(ErrorStrings[3]);
		return ;
	}

	theHeadData.m_powerChanel2 = atoi(GetValueByName("powerChanel2"));		// 0 - 100 Volts
	if (!IsValidInteger("powerChanel2") || theHeadData.m_powerChanel2 > 10000) {
		outputString_UART1(ErrorStrings[4]);
		return ;
	}

	theHeadData.m_powerChanel3 = atoi(GetValueByName("powerChanel3"));		// 0 - 100 Volts
	if (!IsValidInteger("powerChanel3") || theHeadData.m_powerChanel3 > 10000) {
		outputString_UART1(ErrorStrings[5]);
		return ;
	}

	theHeadData.m_powerChanel4 = atoi(GetValueByName("powerChanel4"));		// 0 - 100 Volts
	if (!IsValidInteger("powerChanel4") || theHeadData.m_powerChanel4 > 10000) {
		outputString_UART1(ErrorStrings[6]);
		return ;
	}

	theHeadData.m_strobeDelay = atoi(GetValueByName("strobeDelay"));		// 0 - 100 Volts
	theHeadData.m_strobeWidth = atoi(GetValueByName("strobeWidth"));		// 0 - 100 Volts

	theHeadData.m_triggerEdge = atoi(GetValueByName("triggerEdge"));		// 0 - 100 Volts
	if (!IsValidInteger("triggerEdge") || theHeadData.m_triggerEdge > 2) {
		outputString_UART1(ErrorStrings[7]);
		return ;
	}

	theHeadData.m_triggerId = atoi(GetValueByName("triggerId"));		// 0 - 100 Volts
	if (!IsValidInteger("triggerId") || theHeadData.m_triggerId > 3) {
		outputString_UART1(ErrorStrings[8]);
		return ;
	}

	theHeadData.m_chanelAmplifier = atoi(GetValueByName("chanelAmplifier"));		// 0 - 100 Volts
	if (!IsValidInteger("chanelAmplifier") || theHeadData.m_chanelAmplifier > 3) {
		outputString_UART1(ErrorStrings[9]);
		return ;
	}


	// OK, the verification is completed copy the data back to main variable
	theBankInfo[0].m_output[theOutputId] = theHeadData;

	// and program the hardware using our bankID = 1
	#if 0
	programBank(&theBankInfo[0]);
	#endif

	// send back the reply
	outputString_UART1("hstatus,");
	outputIntAsString_UART1(theHeadStatus[theOutputId-1].m_chanelStatusFlags[0]);
	outputString_UART1(",");
	outputIntAsString_UART1(theHeadStatus[theOutputId-1].m_chanelStatusFlags[1]);
	outputString_UART1(",");
	outputIntAsString_UART1(theHeadStatus[theOutputId-1].m_chanelStatusFlags[2]);
	outputString_UART1(",");
	outputIntAsString_UART1(theHeadStatus[theOutputId-1].m_chanelStatusFlags[3]);
	outputString_UART1("\r\n");
}
