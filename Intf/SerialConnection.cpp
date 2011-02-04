////////////////////////////////////////////////////////////////////
//
// 	Alexandr Filenkov 2010
//
////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#pragma hdrstop

#include <string.h>
#include <stdio.h>
#include "SerialConnection.h"
#include <stdlib.h>

struct TSendLogger
{
	TSendLogger(const char* pCmd, const TCommandErrorOutput& retCode, TSerialOnLogEvent pLogEvent)
		: m_cmd(pCmd)
		, m_retCode(retCode)
		, m_pLogEvent(pLogEvent)
	{
	}

	~TSendLogger()
	{
		SYSTEMTIME sysTime;
		::GetSystemTime(&sysTime);
		char str[1024] = {0};
		_snprintf (str, sizeof(str), "%02d:%02d:%02d %03d %s: %s %s (%d)", sysTime.wHour,sysTime.wMinute,sysTime.wSecond,sysTime.wMilliseconds, m_cmd, m_retCode.m_errorId, (char*)m_retCode.m_errorDescription, m_retCode.m_invalidParameterIndex);
		if (m_pLogEvent)
			m_pLogEvent(m_cmd, str);
	}

private:
	const char* 				m_cmd;
	const TCommandErrorOutput&	m_retCode;

	TSerialOnLogEvent m_pLogEvent;
};

bool TSerialConMan::m_bParserInitialized = false;


//-----------------------------------------------------------------------------------------
TSerialConMan::TSerialConMan()
	: m_hComPort(0)
	, m_pLogEvent(0)
	, m_pParser(new tag_TCMDParser())
{
	m_overlappedOut.hEvent = CreateEvent(0, TRUE/*man reset event */, FALSE /* not signaled */, 0);
	m_overlappedOut.Internal = 0;
	m_overlappedOut.InternalHigh = 0;
	m_overlappedOut.Offset = 0;
	m_overlappedOut.OffsetHigh = 0;

	m_overlappedIn.hEvent = CreateEvent(0, TRUE/*man reset event */, FALSE /* not signaled */, 0);
	m_overlappedIn.Internal = 0;
	m_overlappedIn.InternalHigh = 0;
	m_overlappedIn.Offset = 0;
	m_overlappedIn.OffsetHigh = 0;

	if (!m_bParserInitialized) {
		::InitializeParser();
		m_bParserInitialized = true;
	}

	m_UartID = 0;
}

//-----------------------------------------------------------------------------------------
TSerialConMan::~TSerialConMan()
{
	delete m_pParser;
	m_pParser = 0;
	::CloseHandle(m_overlappedOut.hEvent);
	::CloseHandle(m_overlappedIn.hEvent);
}
//-----------------------------------------------------------------------------------------
TCommandErrorOutput TSerialConMan::init(int aComPort, int aSpeed, TSerialOnLogEvent pLogEvent)
{
	m_pLogEvent = pLogEvent;

	::CloseHandle(m_hComPort);
	char str[64] = {0};
	_snprintf (str, sizeof(str), "\\\\.\\COM%d", aComPort);

	m_hComPort = CreateFileA( str,
		GENERIC_READ | GENERIC_WRITE,
		0,    									// must be opened with exclusive-access
		0, 										// no security attributes
		OPEN_EXISTING, 							// must use OPEN_EXISTING
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,					// for overlapped IO
		NULL  									// hTemplate must be NULL for comm devices
	);

	if (m_hComPort != INVALID_HANDLE_VALUE) {
		DCB dcb;
		if (GetCommState(m_hComPort, &dcb)) {

			dcb.BaudRate = aSpeed;     	// set the baud rate
			dcb.ByteSize = 8;             // data size, xmit, and rcv
			dcb.Parity   = NOPARITY;      // no parity bit
			dcb.StopBits = ONESTOPBIT;    // one stop bit
			dcb.fOutxCtsFlow = FALSE;
			dcb.fOutxDsrFlow = FALSE;
			dcb.fDtrControl  = FALSE;
			dcb.fOutX  = FALSE;
			dcb.fInX  = FALSE;
			dcb.fRtsControl  = FALSE;

			bool bError = false;
			if (!SetCommState(m_hComPort, &dcb)) {
				bError = true; // TODO log the error
			}

			COMMTIMEOUTS comTimeouts = {0};
			comTimeouts.ReadIntervalTimeout = 20;
			comTimeouts.ReadTotalTimeoutMultiplier = 10;
			comTimeouts.ReadTotalTimeoutConstant = 100;
			comTimeouts.WriteTotalTimeoutMultiplier = 10;
			comTimeouts.WriteTotalTimeoutConstant = 100;

			SetCommTimeouts(m_hComPort, &comTimeouts);

			DWORD read;
//			DWORD ret;
			char tmpBuf[16];

			// read any bytes that are stuck in the com port from the previous life
			do {
				if (!::ReadFile (m_hComPort, &tmpBuf, sizeof(tmpBuf), &read, &m_overlappedIn)){
					if (GetLastError() != ERROR_IO_PENDING)
						bError = true;
				}

				if (::WaitForSingleObject(m_overlappedIn.hEvent, 200) != WAIT_OBJECT_0) {
					read = 0;
				}
				else if (!::GetOverlappedResult(m_hComPort, &m_overlappedIn, &read, FALSE)) {
					bError = true;	//TODO log the error
				}
			}
			while (read != 0 && !bError);

			CancelIo(m_hComPort);

			if (bError) {
				TCommandErrorOutput retCode;
				TSendLogger theLogger("init", retCode, m_pLogEvent);
				if (!retCode.hasError()) {
					strncpy(retCode.m_errorDescription, "Error initializing COM port", sizeof(retCode.m_errorDescription));
					strncpy(retCode.m_errorId, "80000009", sizeof(retCode.m_errorId));
				}

				::CloseHandle(m_hComPort);
				m_hComPort = 0;
				return retCode;
			}

			TCommandErrorOutput retCode = send("getver");

			if (retCode.hasError()) {
				::CloseHandle(m_hComPort);
				m_hComPort = 0;
				return retCode;
			}


			if (strcmp("version", GetCmdName()) != 0) {
				TSendLogger theLogger("getver", retCode, m_pLogEvent);
				strncpy(retCode.m_errorDescription, "unknow reply for get version", sizeof(retCode.m_errorDescription));
				strncpy(retCode.m_errorId, "80000010", sizeof(retCode.m_errorId));
				::CloseHandle(m_hComPort);
				m_hComPort = 0;
				return retCode;
			}

			return retCode;
		}
	}

	TCommandErrorOutput retCode;
	TSendLogger theLogger("Open port", retCode, m_pLogEvent);
	strncpy(retCode.m_errorDescription, "COM port cannot be opened", sizeof(retCode.m_errorDescription));
	strncpy(retCode.m_errorId, "80000011", sizeof(retCode.m_errorId));
	return retCode;
}


//-----------------------------------------------------------------------------------------
void TSerialConMan::close()
{
	::CloseHandle(m_hComPort);
	m_hComPort = 0;
}

//-----------------------------------------------------------------------------------------
bool TSerialConMan::isConnected() const
{
	return m_hComPort != INVALID_HANDLE_VALUE && m_hComPort != 0;
}

//-----------------------------------------------------------------------------------------
TCommandErrorOutput TSerialConMan::send(const char* pCmd) const
{
	TCommandErrorOutput retValue;

	try{
		TSendLogger theLogger(pCmd, retValue, m_pLogEvent);
		DWORD theSizeToSend = (DWORD)strlen(pCmd);


		if (theSizeToSend == 0) {
			strncpy(retValue.m_errorDescription, "empty command sent", sizeof(retValue.m_errorDescription));
			strncpy(retValue.m_errorId, "80000001", sizeof(retValue.m_errorId));
			return retValue;
		}

		strncpy(retValue.m_command, pCmd, sizeof(retValue.m_command));

		if (m_hComPort == INVALID_HANDLE_VALUE || m_hComPort == 0) {
			strncpy(retValue.m_errorDescription, "COM port is not opened", sizeof(retValue.m_errorDescription));
			strncpy(retValue.m_errorId, "80000002", sizeof(retValue.m_errorId));
			return retValue;
		}

		// Write command
		DWORD written;
		if (!::WriteFile(m_hComPort, pCmd, theSizeToSend, &written, &m_overlappedOut)){
			if (GetLastError() != ERROR_IO_PENDING){
				CancelIo(m_hComPort);
				strncpy(retValue.m_errorDescription, "Cannot write to port", sizeof(retValue.m_errorDescription));
				strncpy(retValue.m_errorId, "80000003", sizeof(retValue.m_errorId));
				return retValue;
			}
		}

		if (::WaitForSingleObject(m_overlappedOut.hEvent, 1000) != WAIT_OBJECT_0) {
			CancelIo(m_hComPort);
			strncpy(retValue.m_errorDescription, "Command sending timeout", sizeof(retValue.m_errorDescription));
			strncpy(retValue.m_errorId, "80000004", sizeof(retValue.m_errorId));
			return retValue;
		}

		else if (!::GetOverlappedResult(m_hComPort, &m_overlappedOut, &written, FALSE) || written != theSizeToSend) {
			strncpy(retValue.m_errorDescription, "Command sending error", sizeof(retValue.m_errorDescription));
			strncpy(retValue.m_errorId, "80000005", sizeof(retValue.m_errorId));
			return retValue;
		}

		// no terminator specified add one
		if (strchr (pCmd, '\n') == 0){
			theSizeToSend = 1;
			if (!::WriteFile(m_hComPort, "\n", theSizeToSend, &written, &m_overlappedOut)){
				if (GetLastError() != ERROR_IO_PENDING){
					CancelIo(m_hComPort);
					strncpy(retValue.m_errorDescription, "Cannot write to port", sizeof(retValue.m_errorDescription));
					strncpy(retValue.m_errorId, "80000003", sizeof(retValue.m_errorId));
					return retValue;
				}
			}

			if (::WaitForSingleObject(m_overlappedOut.hEvent, 1000) != WAIT_OBJECT_0) {
				CancelIo(m_hComPort);
				strncpy(retValue.m_errorDescription, "Command sending timeout", sizeof(retValue.m_errorDescription));
				strncpy(retValue.m_errorId, "80000004", sizeof(retValue.m_errorId));
				return retValue;
			}

			else if (!::GetOverlappedResult(m_hComPort, &m_overlappedOut, &written, FALSE) || written != theSizeToSend) {
				strncpy(retValue.m_errorDescription, "Command sending error", sizeof(retValue.m_errorDescription));
				strncpy(retValue.m_errorId, "80000005", sizeof(retValue.m_errorId));
				return retValue;
			}
		}

		Sleep(20);

		// Read the beginning of the reply. Can be a command error reply or a valid (optional) reply.
		DWORD read = 0;
		unsigned int cmdIdx = 0;
		char 		 cmdReply[4096] = {0};
		unsigned short theRetryCount = 0;

		do {
			if (!::ReadFile (m_hComPort, &cmdReply[cmdIdx], sizeof(cmdReply[cmdIdx]), &read, &m_overlappedIn)){
				if (GetLastError() != ERROR_IO_PENDING)    // read not delayed?
				{
					CancelIo(m_hComPort);
					strncpy(retValue.m_errorDescription, "Cannot start reading from com port", sizeof(retValue.m_errorDescription));
					strncpy(retValue.m_errorId, "80000006", sizeof(retValue.m_errorId));
					return retValue;
				}
			}

			if (::WaitForSingleObject(m_overlappedIn.hEvent, 5000) != WAIT_OBJECT_0) {
				CancelIo(m_hComPort);
				strncpy(retValue.m_errorDescription, "Command reply timeout", sizeof(retValue.m_errorDescription));
				strncpy(retValue.m_errorId, "80000007", sizeof(retValue.m_errorId));
				return retValue;
			}
			else if (!::GetOverlappedResult(m_hComPort, &m_overlappedIn, &read, FALSE)) {
				strncpy(retValue.m_errorDescription, "Command reply error", sizeof(retValue.m_errorDescription));
				strncpy(retValue.m_errorId, "80000008", sizeof(retValue.m_errorId));
				return retValue;
			}

			if (read) {
				if (cmdReply[cmdIdx] == 0 || cmdReply[cmdIdx] == '\n') {
					break;
				}
				++cmdIdx;
				theRetryCount = 0;
			} else if (++theRetryCount > 5) {
				break;
			}
		} while (cmdIdx < sizeof(cmdReply) / sizeof(cmdReply[0]));

		int replyParameters = ParseSentence(m_pParser, cmdReply);
		if (replyParameters == 0) {
			_snprintf(retValue.m_errorDescription, sizeof(retValue.m_errorDescription), "No Reply from controller (%s)", cmdReply);
//			strncpy(retValue.m_errorDescription, "No Reply from controller", sizeof(retValue.m_errorDescription));
			strncpy(retValue.m_errorId, "80000012", sizeof(retValue.m_errorId));
		} else

		if (strcmp("ERR", GetCmdName()) == 0) {
			strncpy(retValue.m_errorDescription, GetValueByName("errTxt"), sizeof(retValue.m_errorDescription));
			strncpy(retValue.m_errorId, GetValueByName("errId"), sizeof(retValue.m_errorId));
			retValue.m_invalidParameterIndex = atoi(GetValueByName("errPos"));
		}

	} catch (...){
		strncpy(retValue.m_errorDescription, "Unknown error while processing command", sizeof(retValue.m_errorDescription));
		strncpy(retValue.m_errorId, "88888888", sizeof(retValue.m_errorId));
	}
	return retValue;
}


//-----------------------------------------------------------------------------------------
const char *  TSerialConMan::GetValueByName(const char * pName) const
{
	return ::GetValueByName(m_pParser, pName);
}

//-----------------------------------------------------------------------------------------
const char *  TSerialConMan::GetCmdName() const
{
	return ::GetCmdName(m_pParser);
}

