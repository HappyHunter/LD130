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
#include "Monitoring.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>


const char * const ErrorStr __attribute__ ((space(auto_psv))) = "ERR,";
const char * const EndlStr __attribute__ ((space(auto_psv))) = "\r\n";

//Command Patterns
const char * const ErrorStrings[][2] __attribute__ ((space(auto_psv))) =
{
/*0000*/ {"", ""													 },
/*0001*/ {"00000001","outputId,Must be between 1 and 2" 			 },
/*0002*/ {"00000002","voltage,Must be between 0 and 10000 Volts" 	 },
/*0003*/ {"00000003","powerChanel1,Must be between 0 and 10000\%" 	 },
/*0004*/ {"00000004","powerChanel2,Must be between 0 and 10000\%" 	 },
/*0005*/ {"00000005","powerChanel3,Must be between 0 and 10000\%" 	 },
/*0006*/ {"00000006","powerChanel4,Must be between 0 and 10000\%" 	 },
/*0007*/ {"00000007","triggerEdge,Must be 0,1 or 2" 				 },
/*0008*/ {"00000008","triggerId,Must be 1,2 or 3" 					 },
/*0009*/ {"00000009","chanelAmplifier,Must be between 1 and 2" 		 },
/*0010*/ {"00000010","strobeDelay,is not valid integer" 			 },
/*0011*/ {"00000011","strobeWidth,is not valid integer" 			 },
/*0012*/ {"00000012","bankId,Must be between 1 and 4" 				 },
/*0013*/ {"00000013","Invalid magic number" 						 },
};

void outputErrorString_UART1(unsigned short anErrIdx)
{
	outputString_UART1(ErrorStr);
	outputString_UART1(ErrorStrings[anErrIdx][0]);
	outputChar_UART1(',');
	outputString_UART1(ErrorStrings[anErrIdx][1]);
	outputString_UART1(EndlStr);
}

/**
* Forward declarations for helper functions
*/
//-----------------------------------------------------------------------------------------
void sendVersionReply();
void setHeadData();
void sendHeadStatus(unsigned short anOutputId);
void sendHeadData();
void setBankData();
void sendBankData();
void setSequenceData();
void setSerialNumber();
void sendSeqData();
void sendStatus();
//-----------------------------------------------------------------------------------------

void InitCmdManager (void)
{
    //------------------------------------------------------------------------------
    //  Init the components
    //------------------------------------------------------------------------------
	InitializeParser();

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

	static const char* pTmpValue = 0;		// temp pointer to a value

	Start_UART1();

	for (;;)
    {
        OS_Csem_Wait (Uart1_Msg);	// now wait here untill any data arrives to UART

		// increment timer counetr
		OS_Timer();

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
			} else

			if (strcmp(GetCmdName(), "gethstatus") == 0) {
				pTmpValue = GetValueByName("outputId");		// 1 - head 1, 2 - Head 2
				if (IsValidInteger(pTmpValue) != 1) {
					outputErrorString_UART1(1);
					continue;
				}
				sendHeadStatus(atoi(pTmpValue));
				continue;
			} else

			if (strcmp(GetCmdName(), "gethdata") == 0) {
				sendHeadData();
				continue;
			} else

			if (strcmp(GetCmdName(), "setbankdata") == 0) {
				setBankData();
				continue;
			} else

			if (strcmp(GetCmdName(), "getbankdata") == 0) {
				sendBankData();
				continue;
			} else

			if (strcmp(GetCmdName(), "getcfgdata") == 0) {

				//"cfgdata,flags,activeBank",
				outputString_UART1("cfgdata,");
				outputIntAsString_UART1(getConfigFlags());
				outputChar_UART1(',');
				outputIntAsString_UART1(getActiveBank());
				outputString_UART1("\r\n");

				continue;
			} else

			if (strcmp(GetCmdName(), "softtrig") == 0) {
				pTmpValue = GetValueByName("triggerId");		// 1 - Trigger 1, 2 - Trigger 2, 3 - Trigger 1 or 2
				if (IsValidInteger(pTmpValue) != 1) {
					outputErrorString_UART1(1);
					continue;
				}
				fireSoftTrigger(atoi(pTmpValue));
				continue;
			} else


			if (strcmp(GetCmdName(), "setseqdata") == 0) {
				setSequenceData();
				continue;
			} else

			if (strcmp(GetCmdName(), "getseqdata") == 0) {
				sendSeqData();
				continue;
			} else

			if (strcmp(GetCmdName(), "setportspeed") == 0) {
				continue;
			} else

			if (strcmp(GetCmdName(), "getportspeed") == 0) {
				continue;
			} else


			if (strcmp(GetCmdName(), "writeeprom") == 0) {
				continue;
			} else

			if (strcmp(GetCmdName(), "loadeprom") == 0) {
				continue;
			} else

			if (strcmp(GetCmdName(), "cleareprom") == 0) {
				continue;
			} else

			if (strcmp(GetCmdName(), "setbank") == 0) {
				continue;
			} else

			if (strcmp(GetCmdName(), "getstatus") == 0) {
				sendStatus();
				continue;
			} else

			if (strcmp(GetCmdName(), "setsernum") == 0) {
				setSerialNumber();
				continue;
			}


			outputString_UART1(GetCmdName());
			outputString_UART1(":ERR,00000000,Unknown Command\r\n");
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

		// increment timer counetr
		OS_Timer();
    }

}


/**
* sendVersionReply() sends back the firmware version info
*
* Message input: "getver"
*
*
* Output:
* 	"version,sernum,vermajor,verminor,verbuild",
*
*/
//------------------------------------------------------------------------------
void sendVersionReply()
{
	outputString_UART1("version,");
	outputString_UART1("4A38A349-1108-4fd9-BE9B-70A1E477FBF5");
	outputChar_UART1(',');
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
*    "hstatus,bankId,statusChanel1,statusChanel2,statusChanel3,statusChanel4\r\n"
*
*/
//------------------------------------------------------------------------------
void setHeadData()
{
	const char* 		pValue;
	TBankHeadData 	theHeadData;

	unsigned short 	theOutputId;

	pValue = GetValueByName("outputId");		// 1 - head 1, 2 - Head 2
	theOutputId = atoi(pValue);
	if (IsValidInteger(pValue) != 1 || theOutputId < 1 || theOutputId > 2) {
		outputErrorString_UART1(1);
		return ;
	}

	// copy the data into our temp variable
	// with set head data we will always use bankID=1 (index 0)
	theHeadData = getBankInfo(1)->m_output[theOutputId-1];



	pValue = GetValueByName("voltage");			// 0 - 100 %
	theHeadData.m_voltage = atoi(pValue);
	if (!IsValidInteger(pValue) || theHeadData.m_voltage > 100) {
		outputErrorString_UART1(2);
		return ;
	}

	pValue = GetValueByName("powerChanel1");	//0 - 100 00% with fixed decimal point at 2 digits
	theHeadData.m_powerChanel1 = atoi(pValue);
	if (!IsValidInteger(pValue) || theHeadData.m_powerChanel1 > 10000) {
		outputErrorString_UART1(3);
		return ;
	}

	pValue = GetValueByName("powerChanel2");	//0 - 100 00% with fixed decimal point at 2 digits
	theHeadData.m_powerChanel2 = atoi(pValue);
	if (!IsValidInteger(pValue) || theHeadData.m_powerChanel2 > 10000) {
		outputErrorString_UART1(4);
		return ;
	}

	pValue = GetValueByName("powerChanel3");	//0 - 100 00% with fixed decimal point at 2 digits
	theHeadData.m_powerChanel3 = atoi(pValue);
	if (!IsValidInteger(pValue) || theHeadData.m_powerChanel3 > 10000) {
		outputErrorString_UART1(5);
		return ;
	}

	pValue = GetValueByName("powerChanel4");	//0 - 100 00% with fixed decimal point at 2 digits
	theHeadData.m_powerChanel4 = atoi(pValue);
	if (!IsValidInteger(pValue) || theHeadData.m_powerChanel4 > 10000) {
		outputErrorString_UART1(6);
		return ;
	}

	pValue = GetValueByName("strobeDelay");		// the delay of outcoming light strobe in microseconds
	theHeadData.m_strobeDelay = atoi(pValue);
	if (!IsValidInteger(pValue)) {
		outputErrorString_UART1(10);
		return ;
	}

	pValue = GetValueByName("strobeWidth");		// the duration of outcoming light strobe in microseconds
	theHeadData.m_strobeWidth = atoi(pValue);
	if (!IsValidInteger(pValue)) {
		outputErrorString_UART1(11);
		return ;
	}

	pValue = GetValueByName("triggerEdge");		// the edge of incoming trigger to start counting, 0 - raising, 1 - falling, 2 - DC mode
	theHeadData.m_triggerEdge = atoi(pValue);
	if (!IsValidInteger(pValue) || theHeadData.m_triggerEdge > 2) {
		outputErrorString_UART1(7);
		return ;
	}

	pValue = GetValueByName("triggerId");		// the ID of the trigger this output head will trigger on. 1 - Trigger 1, 2 - Trigger 2, 3 - Trigger 1 or 2
	theHeadData.m_triggerId = atoi(pValue);		// 0 - 100 Volts
	if (!IsValidInteger(pValue) || theHeadData.m_triggerId > 3) {
		outputErrorString_UART1(8);
		return ;
	}

	pValue = GetValueByName("chanelAmplifier");	// the amplification value 1-5
	theHeadData.m_chanelAmplifier = atoi(pValue);
	if (!IsValidInteger(pValue) || theHeadData.m_chanelAmplifier > 3) {
		outputErrorString_UART1(9);
		return ;
	}


	// OK, the verification is completed copy the data back to main variable
	getBankInfo(1)->m_output[theOutputId-1] = theHeadData;

	// and program the hardware using our bankID
	setActiveBank(1/*bankID=1*/, 1/* force re-programming*/);

	sendHeadStatus(theOutputId);
}


/**
 * Sends back the status of the head
 *
 * @parameters anOutputId 1 - head 1, 2 - Head 2
 */
//------------------------------------------------------------------------------
void sendHeadStatus(unsigned short anOutputId)
{
	if (anOutputId < 1 || anOutputId > 2) {
		outputErrorString_UART1(1);
		return;
	}

	// send back the reply
	outputString_UART1("hstatus,");
	outputIntAsString_UART1(getActiveBank());
	outputChar_UART1(',');
	outputIntAsString_UART1(getHeadStatus(anOutputId)->m_statusChanel1);
	outputChar_UART1(',');
	outputIntAsString_UART1(getHeadStatus(anOutputId)->m_statusChanel2);
	outputChar_UART1(',');
	outputIntAsString_UART1(getHeadStatus(anOutputId)->m_statusChanel3);
	outputChar_UART1(',');
	outputIntAsString_UART1(getHeadStatus(anOutputId)->m_statusChanel4);
	outputString_UART1("\r\n");
}

/**
* sendHeadData() sends back the data for the output ID specified
*  unsigned short	m_outputId;		// 1 - head 1, 2 - Head 2
*  unsigned short	m_voltage;		// 0 - 100 %
*  unsigned short	m_powerChanel1; // 0 - 100 00% with fixed decimal point at 2 digits
*  unsigned short	m_powerChanel2; // for example the power of 35.23% will be sent as 3523
*  unsigned short	m_powerChanel3; // the power of 99.00% will be sent as 9900
*  unsigned short	m_powerChanel4; // the power of 100.00% will be sent as 10000
*  unsigned long	m_strobeDelay;	// the delay of outcoming light strobe in microseconds
*  unsigned long	m_strobeWidth;	// the duration of outcoming light strobe in microseconds
*  unsigned short	m_triggerEdge;	// the edge of incoming trigger to start counting, 0 - raising, 1 - falling, 2 - DC mode
*  unsigned short	m_triggerId;	// the ID of the trigger this output head will trigger on. 1 - Trigger 1, 2 - Trigger 2, 3 - Trigger 1 or 2
*  unsigned short	m_chanelAmplifier;	 // the amplification value 1-5
*/
//------------------------------------------------------------------------------
void sendHeadData()
{
	const char* 		pValue;
	TBankHeadData 	theHeadData;

	unsigned short 	theOutputId;

	pValue = GetValueByName("outputId");		// 1 - head 1, 2 - Head 2
	theOutputId = atoi(pValue);
	if (IsValidInteger(pValue) != 1 || theOutputId < 1 || theOutputId > 2) {
		outputErrorString_UART1(1);
		return ;
	}

	// copy the data into our temp variable
	// with set head data we will always use bankID=1 (index 0)
	theHeadData = getBankInfo(1)->m_output[theOutputId-1];

	// send back the reply
    //"hdata,outputId,voltage,powerChanel1,powerChanel2,powerChanel3,powerChanel4,strobeDelay,strobeWidth,triggerEdge,triggerId,chanelAmplifier",
	outputString_UART1("hdata,");
	outputIntAsString_UART1(theHeadData.m_voltage);
	outputChar_UART1(',');
	outputIntAsString_UART1(theHeadData.m_powerChanel1);
	outputChar_UART1(',');
	outputIntAsString_UART1(theHeadData.m_powerChanel2);
	outputChar_UART1(',');
	outputIntAsString_UART1(theHeadData.m_powerChanel3);
	outputChar_UART1(',');
	outputIntAsString_UART1(theHeadData.m_powerChanel4);
	outputChar_UART1(',');
	outputIntAsString_UART1(theHeadData.m_strobeDelay);
	outputChar_UART1(',');
	outputIntAsString_UART1(theHeadData.m_strobeWidth);
	outputChar_UART1(',');
	outputIntAsString_UART1(theHeadData.m_triggerEdge);
	outputChar_UART1(',');
	outputIntAsString_UART1(theHeadData.m_triggerId);
	outputChar_UART1(',');
	outputIntAsString_UART1(theHeadData.m_chanelAmplifier);
	outputString_UART1("\r\n");
}

/**
* setBankData() sets the parameters for the bank specified
* The function will reprogram hardware if the current active
* bank is the same as we are programming
*
*  Message input:
*  "setbankdata,bankId,outputId,voltage,powerChanel1,powerChanel2,powerChanel3,powerChanel4,strobeDelay,strobeWidth,triggerEdge,triggerId,chanelAmplifier",
*
*  unsigned short	m_bankId;		// 1, 2, 3, 4
*  unsigned short	m_outputId;		// 1 - head 1, 2 - Head 2
*  unsigned short	m_voltage;		// 0 - 100 %
*  unsigned short	m_powerChanel1; // 0 - 100 00% with fixed decimal point at 2 digits
*  unsigned short	m_powerChanel2; // for example the power of 35.23% will be sent as 3523
*  unsigned short	m_powerChanel3; // the power of 99.00% will be sent as 9900
*  unsigned short	m_powerChanel4; // the power of 100.00% will be sent as 10000
*  unsigned long	m_strobeDelay;	// the delay of outcoming light strobe in microseconds
*  unsigned long	m_strobeWidth;	// the duration of outcoming light strobe in microseconds
*  unsigned short	m_triggerEdge;	// the edge of incoming trigger to start counting, 0 - raising, 1 - falling, 2 - DC mode
*  unsigned short	m_triggerId;	// the ID of the trigger this output head will trigger on. 1 - Trigger 1, 2 - Trigger 2, 3 - Trigger 1 or 2
*  unsigned short	m_chanelAmplifier;	 // the amplification value 1-5
*
*  Output:
*    "hstatus,bankId,statusChanel1,statusChanel2,statusChanel3,statusChanel4\r\n"
*
*/
//------------------------------------------------------------------------------
void setBankData()
{
	const char* 		pValue;
	TBankHeadData 	theHeadData;

	unsigned short 	theOutputId;
	unsigned short 	theBankId;

	pValue = GetValueByName("bankId");		// 1, 2, 3, 4
	theBankId = atoi(pValue);
	if (IsValidInteger(pValue) != 1 || theBankId < 1 || theBankId > MAX_NUM_OF_BANKS) {
		outputErrorString_UART1(12);
		return ;
	}

	pValue = GetValueByName("outputId");		// 1 - head 1, 2 - Head 2
	theOutputId = atoi(pValue);
	if (IsValidInteger(pValue) != 1 || theOutputId < 1 || theOutputId > 2) {
		outputErrorString_UART1(1);
		return ;
	}

	// copy the data into our temp variable
	// with set head data we will use bankID-1 as an index
	theHeadData = getBankInfo(theBankId)->m_output[theOutputId-1];


	pValue = GetValueByName("voltage");			// 0 - 100 %
	theHeadData.m_voltage = atoi(pValue);
	if (!IsValidInteger(pValue) || theHeadData.m_voltage > 100) {
		outputErrorString_UART1(2);
		return ;
	}

	pValue = GetValueByName("powerChanel1");	//0 - 100 00% with fixed decimal point at 2 digits
	theHeadData.m_powerChanel1 = atoi(pValue);
	if (!IsValidInteger(pValue) || theHeadData.m_powerChanel1 > 10000) {
		outputErrorString_UART1(3);
		return ;
	}

	pValue = GetValueByName("powerChanel2");	//0 - 100 00% with fixed decimal point at 2 digits
	theHeadData.m_powerChanel2 = atoi(pValue);
	if (!IsValidInteger(pValue) || theHeadData.m_powerChanel2 > 10000) {
		outputErrorString_UART1(4);
		return ;
	}

	pValue = GetValueByName("powerChanel3");	//0 - 100 00% with fixed decimal point at 2 digits
	theHeadData.m_powerChanel3 = atoi(pValue);
	if (!IsValidInteger(pValue) || theHeadData.m_powerChanel3 > 10000) {
		outputErrorString_UART1(5);
		return ;
	}

	pValue = GetValueByName("powerChanel4");	//0 - 100 00% with fixed decimal point at 2 digits
	theHeadData.m_powerChanel4 = atoi(pValue);
	if (!IsValidInteger(pValue) || theHeadData.m_powerChanel4 > 10000) {
		outputErrorString_UART1(6);
		return ;
	}

	pValue = GetValueByName("strobeDelay");		// the delay of outcoming light strobe in microseconds
	theHeadData.m_strobeDelay = atoi(pValue);
	if (!IsValidInteger(pValue)) {
		outputErrorString_UART1(10);
		return ;
	}

	pValue = GetValueByName("strobeWidth");		// the duration of outcoming light strobe in microseconds
	theHeadData.m_strobeWidth = atoi(pValue);
	if (!IsValidInteger(pValue)) {
		outputErrorString_UART1(11);
		return ;
	}

	pValue = GetValueByName("triggerEdge");		// the edge of incoming trigger to start counting, 0 - raising, 1 - falling, 2 - DC mode
	theHeadData.m_triggerEdge = atoi(pValue);
	if (!IsValidInteger(pValue) || theHeadData.m_triggerEdge > 2) {
		outputErrorString_UART1(7);
		return ;
	}

	pValue = GetValueByName("triggerId");		// the ID of the trigger this output head will trigger on. 1 - Trigger 1, 2 - Trigger 2, 3 - Trigger 1 or 2
	theHeadData.m_triggerId = atoi(pValue);		// 0 - 100 Volts
	if (!IsValidInteger(pValue) || theHeadData.m_triggerId > 3) {
		outputErrorString_UART1(8);
		return ;
	}

	pValue = GetValueByName("chanelAmplifier");	// the amplification value 1-5
	theHeadData.m_chanelAmplifier = atoi(pValue);
	if (!IsValidInteger(pValue) || theHeadData.m_chanelAmplifier > 3) {
		outputErrorString_UART1(9);
		return ;
	}


	// OK, the verification is completed copy the data back to main variable
	getBankInfo(theBankId)->m_output[theOutputId-1] = theHeadData;

	// and program the hardware using our bankID, forcing it to update the hardware
	if (getActiveBank() == theBankId) {
		setActiveBank(theBankId/*bankID*/, 1/* force re-programming*/);
	}

	// and send back the head status
	sendHeadStatus(theOutputId);
}

/**
 * sendBankData() sends back the specified bank and output head
 * data
 *
 *  Message input:
 *    "getbankdata,bankId,headId",
 *
 *  unsigned short	m_bankId;		// 1, 2, 3, 4
 *  unsigned short	m_outputId;		// 1 - head 1, 2 - Head 2
 *
 *  Output:
 *   "bankdata,bankId,outputId,voltage,powerChanel1,powerChanel2,powerChanel3,powerChanel4,strobeDelay,strobeWidth,triggerEdge,triggerId,chanelAmplifier",
*/
//------------------------------------------------------------------------------
void sendBankData()
{
	const char* 		pValue;
	TBankHeadData 	theHeadData;

	unsigned short 	theOutputId;
	unsigned short 	theBankId;

	pValue = GetValueByName("bankId");		// 1, 2, 3, 4
	theBankId = atoi(pValue);
	if (IsValidInteger(pValue) != 1 || theBankId < 1 || theBankId > MAX_NUM_OF_BANKS) {
		outputErrorString_UART1(12);
		return ;
	}

	pValue = GetValueByName("outputId");		// 1 - head 1, 2 - Head 2
	theOutputId = atoi(pValue);
	if (IsValidInteger(pValue) != 1 || theOutputId < 1 || theOutputId > 2) {
		outputErrorString_UART1(1);
		return ;
	}

	// copy the data into our temp variable
	theHeadData = getBankInfo(theBankId)->m_output[theOutputId-1];

	// send back the reply
    // "bankdata,bankId,outputId,voltage,powerChanel1,powerChanel2,powerChanel3,powerChanel4,strobeDelay,strobeWidth,triggerEdge,triggerId,chanelAmplifier",
	outputString_UART1("bankdata,");
	outputIntAsString_UART1(theBankId);
	outputChar_UART1(',');
	outputIntAsString_UART1(theOutputId);
	outputChar_UART1(',');
	outputIntAsString_UART1(theHeadData.m_voltage);
	outputChar_UART1(',');
	outputIntAsString_UART1(theHeadData.m_powerChanel1);
	outputChar_UART1(',');
	outputIntAsString_UART1(theHeadData.m_powerChanel2);
	outputChar_UART1(',');
	outputIntAsString_UART1(theHeadData.m_powerChanel3);
	outputChar_UART1(',');
	outputIntAsString_UART1(theHeadData.m_powerChanel4);
	outputChar_UART1(',');
	outputIntAsString_UART1(theHeadData.m_strobeDelay);
	outputChar_UART1(',');
	outputIntAsString_UART1(theHeadData.m_strobeWidth);
	outputChar_UART1(',');
	outputIntAsString_UART1(theHeadData.m_triggerEdge);
	outputChar_UART1(',');
	outputIntAsString_UART1(theHeadData.m_triggerId);
	outputChar_UART1(',');
	outputIntAsString_UART1(theHeadData.m_chanelAmplifier);
	outputString_UART1("\r\n");
}


/**
 * Sets the bank sequence data
 *  Message input:
 *    "setseqdata,s_10,s_20,s_30,s_40,s_50,s_60,s_70,s_80,s_90,s_100,s_110,s_120,s_130,s_140,s_150,s_160,s_170,s_180,s_190,s_200,s_210,s_220,s_230,s_240,s_250,s_260,s_270,s_280,s_290,s_300,s_310,s_320,s_330,s_340,s_350,s_360,s_370,s_380,s_390,s_400",
 *
 *    Each of the elemnts (s_10 ... s_400) length is 10
 *    characters. Each character is a digit ['1','2','3','4']
 *    representing the bank ID
 *
 * Output:
 *   "seqdatalen,seqlen",
 *
*/
//------------------------------------------------------------------------------
const char * const NameTemplates[] __attribute__ ((space(auto_psv))) =
{
	"s_10","s_20","s_30","s_40","s_50","s_60","s_70","s_80","s_90","s_100",
	"s_110","s_120","s_130","s_140","s_150","s_160","s_170","s_180","s_190","s_200",
	"s_210","s_220","s_230","s_240","s_250","s_260","s_270","s_280","s_290","s_300",
	"s_310","s_320","s_330","s_340","s_350","s_360","s_370","s_380","s_390","s_400",
};

//------------------------------------------------------------------------------
unsigned int setSequenceDataImpl(const char* aSeqName, unsigned short* anIndex)
{
	const char* pValue;
	pValue = GetValueByName(aSeqName);		// each character is a bank ID [1, 2, 3, 4]
	// if the elemnt is empty we will stop and return 0
	if (IsValidInteger(pValue) == 2) {
		return 0;
	}

    while(pValue && *pValue)
    {
		if (*pValue >= '1' && *pValue < ('1'+MAX_NUM_OF_BANKS)) {
			setBankSequenceAt((*pValue) - '0', *anIndex);
			++(*anIndex);
		}
        // go to next char
        ++pValue;
    }
	return(1);
}


//------------------------------------------------------------------------------
void setSequenceData()
{
	unsigned short	theIndex = 0;
	unsigned char	i;

	for (i = 0; i < 40; ++i) {
		if (setSequenceDataImpl(NameTemplates[i], &theIndex) == 0)
			break;
	}

	setBankSequenceLength(theIndex);


	outputString_UART1("seqdatalen,");
	outputIntAsString_UART1(theIndex);
	outputString_UART1("\r\n");
}


/**
 * Sets the controller serial number
 *  Message input:
 *   "setsernum,magic,s_10,s_20,s_40",
 *
 *    Each of the elemnts (s_10 ... s_400) length is 10
 *    characters. Each character is a digit ['1','2','3','4']
 *    representing the portion of a serial number
 *	  magic is a string == B7FC4945DD
 *
 * Output:
 *   "OK",
 *
*/
//------------------------------------------------------------------------------
void setSerialNumber()
{
	const char* pValue;
	char theSerialNumber[36];			// the controller serial number in form of GUID
	unsigned char theIndex = 0;
	unsigned char theGroup = 0;

	if (strcmp(GetValueByName("magic"), "B7FC4945DD") == 0) {

		for (theGroup = 0; theGroup < 4; ++theGroup) {
			pValue = GetValueByName(NameTemplates[theGroup]);
		    while(pValue && *pValue)
		    {
				theSerialNumber[theIndex] = *pValue;
		        // go to next char
		        ++pValue;
				++theIndex;
		    }
		}

		outputString_UART1("OK\r\n");
	}
	outputErrorString_UART1(13);
}


/**
 * sends back the existing sequence daata
 *  Message input:
 *   "getseqdata",
 *
 *    Each of the elemnts (s_10 ... s_400) length is 10
 *    characters. Each character is a digit ['1','2','3','4']
 *    representing the bank ID
 *
 * Output:
 *   "seqdata,curIdx,s_10,s_20,s_30,s_40,s_50,s_60,s_70,s_80,s_90,s_100,s_110,s_120,s_130,s_140,s_150,s_160,s_170,s_180,s_190,s_200,s_210,s_220,s_230,s_240,s_250,s_260,s_270,s_280,s_290,s_300,s_310,s_320,s_330,s_340,s_350,s_360,s_370,s_380,s_390,s_400",
 *
*/
//------------------------------------------------------------------------------
void sendSeqData()
{
	unsigned short i;
	unsigned short iEnd;


	iEnd = getBankSequenceLength();

	outputString_UART1("seqdatalen,");
	outputIntAsString_UART1(getCurrentBankSequencePosition());
	outputChar_UART1(',');
	for (i = 0; i < iEnd; ++i) {
		outputChar_UART1(getBankSequenceAt(i)+'0');
		if (((i+1)%10) == 0)
			outputChar_UART1(',');
	}
	outputString_UART1("\r\n");
}





/**
 * sends back the status of the controller and the temperature
 *  Message input:
 *   "getstatus",
 *
 * Output:
 *   "status,TH1,TH2,TAmb",
 *
*/
//------------------------------------------------------------------------------
void sendStatus()
{
	outputString_UART1("status,");
	outputFloatAsString_UART1(getTemperatureH1());
	outputChar_UART1(',');
	outputFloatAsString_UART1(getTemperatureH2());
	outputChar_UART1(',');
	outputFloatAsString_UART1(getTemperatureAmb());
	outputString_UART1("\r\n");
}

