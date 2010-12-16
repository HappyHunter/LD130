////////////////////////////////////////////////////////////////////
//
// 	Alexandr Filenkov 2010
//
////////////////////////////////////////////////////////////////////


#ifndef SerialConMan_201012141650
#define SerialConMan_201012141650

#include "CommonCmd.h"
/*
* NOT THREAD SAFE CLASS
*/
typedef void (*TSerialOnLogEvent)(const char* pCmd, const char* pLogStr);
class TSerialConMan
{
public:

	TSerialConMan();
	~TSerialConMan();

	TCommandErrorOutput send(const char* pCmd);

	TCommandErrorOutput init(int aComPort, int aSpeed = 9600, TSerialOnLogEvent pLogEvent = 0);

	static const char * GetValueByName(const char * pName);
	static const char * GetCmdName();

private:
	HANDLE 		m_hComPort;
	int			m_UartID;

	OVERLAPPED	m_overlappedOut;
	OVERLAPPED	m_overlappedIn;

	TSerialOnLogEvent m_pLogEvent;
};
#endif
