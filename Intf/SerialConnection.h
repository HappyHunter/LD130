////////////////////////////////////////////////////////////////////
//
// 	Alexandr Filenkov 2010
//
////////////////////////////////////////////////////////////////////


#ifndef SerialConMan_201012141650
#define SerialConMan_201012141650

#include "CommonCmd.h"
#include "Parser.h"

/*
* NOT THREAD SAFE CLASS
*/
typedef void (*TSerialOnLogEvent)(const void* anObject, const char* pCmd, const char* pLogStr);

class TSerialConMan
{
public:

	TSerialConMan();
	~TSerialConMan();

	bool isConnected() const;

	TCommandErrorOutput send(const char* pCmd) const;

	TCommandErrorOutput init(int aComPort, int aSpeed = 9600, TSerialOnLogEvent pLogEvent = 0, const void* anObject=0);
	void close();

	const char * GetValueByName(const char * pName) const;
	const char * GetCmdName() const;

private:
	HANDLE 		m_hComPort;
	int			m_UartID;

	mutable OVERLAPPED	m_overlappedOut;
	mutable OVERLAPPED	m_overlappedIn;

	TSerialOnLogEvent 	m_pLogEvent;
	const void*			m_onLogObject;

	TCMDParser*	m_pParser;

	static bool		m_bParserInitialized;
};
#endif
