// LD130i.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "LD130i.h"
#include "..\Intf\LD130Common.h"
#include "..\Intf\LD130Impl.h"

#ifdef _MANAGED
#pragma managed(push, off)
#endif

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
    return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

//-----------------------------------------------------------------------------------------

TLD130Impl* TheLD130Controllers[MAX_NUM_OF_CONTROLLERS+1] = {0};
DWORD TheLD130LastErrors[MAX_NUM_OF_CONTROLLERS+1] = {0};

TCriticalSection TheLock;

//-----------------------------------------------------------------------------------------
LPARAM isValidHandle(HANDLE aHandle)
{
	if (aHandle > 0 && LONG_PTR(aHandle) <= MAX_NUM_OF_CONTROLLERS)
		return LONG_PTR(aHandle);
	return 0;
}

//-----------------------------------------------------------------------------------------
extern "C" LD130I_API HANDLE LD130Open()
{
	unsigned int i;

	TCritical theLock(TheLock);

	for (i = 1; i <= MAX_NUM_OF_CONTROLLERS; ++i){
		if (TheLD130Controllers[i] == 0) {
			TLD130Impl* pLD130 = 0;
			try {
				pLD130 = new TLD130Impl;
				TheLD130Controllers[i] = pLD130;
				return (HANDLE)ULONG_PTR(i);
			} catch (...){
				TheLD130LastErrors[0] = ld130ExceptionError;
			}
			return 0;
		}
	}

	// cannot find free slot for LD130 controller
	TheLD130LastErrors[0] = ld130MaxNumOfControllersUsed;
	return 0;
}


//-----------------------------------------------------------------------------------------
LD130I_API BOOL LD130Close(HANDLE aHandle)
{
	LPARAM hIdx = isValidHandle(aHandle);
	if (hIdx == 0 || TheLD130Controllers[hIdx] == 0){
		TheLD130LastErrors[0] = ld130InvalidHandle;
		return FALSE;
	}

	TCritical theLock(TheLock);
	if (TheLD130Controllers[hIdx] != 0) {
		try {
			delete TheLD130Controllers[hIdx];
			TheLD130Controllers[hIdx] = 0;
			return TRUE;
		} catch (...){
			TheLD130LastErrors[0] = ld130ExceptionError;
		}
	}
	return FALSE;
}


//-----------------------------------------------------------------------------------------
LD130I_API BOOL LD130Init(HANDLE aHandle, int aComPort, int aSpeed, TLD130OnLogEvent onLogEvent, const void* anObject)
{
	LPARAM hIdx = isValidHandle(aHandle);
	if (hIdx == 0 || TheLD130Controllers[hIdx] == 0){
		TheLD130LastErrors[0] = ld130InvalidHandle;
		return FALSE;
	}

	TCritical theLock(TheLock);
	if (TheLD130Controllers[hIdx] != 0) {
		try {
			TCommandErrorOutput retCode = TheLD130Controllers[hIdx]->init(aComPort, aSpeed, onLogEvent, anObject);
			return retCode.hasError() != 0;
		} catch (...){
			TheLD130LastErrors[0] = ld130ExceptionError;
		}
	}
	return FALSE;
}


//-----------------------------------------------------------------------------------------
LD130I_API  BOOL LD130GetVersion(HANDLE aHandle, const char** verMajor, const char** verMinor,const char** verBuild)
{
	LPARAM hIdx = isValidHandle(aHandle);
	if (hIdx == 0 || TheLD130Controllers[hIdx] == 0){
		TheLD130LastErrors[0] = ld130InvalidHandle;
		return FALSE;
	}

	TCritical theLock(TheLock);
	if (TheLD130Controllers[hIdx] != 0) {
		try {
			TheLD130Controllers[hIdx]->getVersion(verMajor, verMinor,verBuild);
			return TRUE;
		} catch (...){
			TheLD130LastErrors[0] = ld130ExceptionError;
		}
	}
	return FALSE;
}

//-----------------------------------------------------------------------------------------
LD130I_API BOOL LD130SetBankHeadData(HANDLE aHandle, 		// Handle to Light Controller
				unsigned short aBankId,		// 1,2,3,4
				unsigned short aHeadId, 	// 1,2
				unsigned short aVoltage,	// 0 - 100 Volts
				unsigned short aPowerChanel1, 	// 0 - 100 00% with fixed decimal point at 2 digits
				unsigned short aPowerChanel2,	// for example the power of 35.23% will be sent as 3523
				unsigned short aPowerChanel3,	// the power of 99.00% will be sent as 9900
				unsigned short aPowerChanel4,	// the power of 100.00% will be sent as 10000
				unsigned long  aStrobeDelay,	// the delay of outcoming light strobe in microseconds
				unsigned long  aStrobeWidth,	// the duration of outcoming light strobe in microseconds
				unsigned short aTriggerEdge,	// the edge of incoming trigger to start counting, 0 - raising, 1 - falling, 2 - DC mode
				unsigned short aTriggerId,	// the ID of the trigger this output head will trigger on. 1 - Trigger 1, 2 - Trigger 2, 3 - Trigger 1 or 2
				unsigned short aChanelAmplifier	// the amplification value 1-5
				)
{
	LPARAM hIdx = isValidHandle(aHandle);
	if (hIdx == 0 || TheLD130Controllers[hIdx] == 0){
		TheLD130LastErrors[0] = ld130InvalidHandle;
		return FALSE;
	}

	TCritical theLock(TheLock);
	if (TheLD130Controllers[hIdx] != 0) {
		try {
			TCommandErrorOutput retCode = TheLD130Controllers[hIdx]->setBankHeadData(aBankId, aHeadId, aVoltage, aPowerChanel1, aPowerChanel2, aPowerChanel3, aPowerChanel4, aStrobeDelay, aStrobeWidth, aTriggerEdge, aTriggerId, aChanelAmplifier);
			return retCode.hasError() != 0;
		} catch (...){
			TheLD130LastErrors[0] = ld130ExceptionError;
		}
	}
	return FALSE;
}

//-----------------------------------------------------------------------------------------
LD130I_API BOOL LD130IsConnected(HANDLE aHandle)
{
	LPARAM hIdx = isValidHandle(aHandle);
	if (hIdx == 0 || TheLD130Controllers[hIdx] == 0){
		TheLD130LastErrors[0] = ld130InvalidHandle;
		return FALSE;
	}

	TCritical theLock(TheLock);
	if (TheLD130Controllers[hIdx] != 0) {
		try {
			TheLD130Controllers[hIdx]->isConnected();
			return TRUE;
		} catch (...){
			TheLD130LastErrors[0] = ld130ExceptionError;
		}
	}
	return FALSE;
}


//-----------------------------------------------------------------------------------------
LD130I_API BOOL LD130SetBankSequence(HANDLE aHandle, const char* aValue)
{
	LPARAM hIdx = isValidHandle(aHandle);
	if (hIdx == 0 || TheLD130Controllers[hIdx] == 0){
		TheLD130LastErrors[0] = ld130InvalidHandle;
		return FALSE;
	}

	TCritical theLock(TheLock);
	if (TheLD130Controllers[hIdx] != 0) {
		try {
			if (aValue) {
				TheLD130Controllers[hIdx]->setSequence(aValue);
			} else{
				TheLD130Controllers[hIdx]->setSequence("");
			}
			return TRUE;
		} catch (...){
			TheLD130LastErrors[0] = ld130ExceptionError;
		}
	}
	return FALSE;
}


//-----------------------------------------------------------------------------------------
LD130I_API unsigned short LD130GetActiveBank(HANDLE aHandle)
{
	LPARAM hIdx = isValidHandle(aHandle);
	if (hIdx == 0 || TheLD130Controllers[hIdx] == 0){
		TheLD130LastErrors[0] = ld130InvalidHandle;
		return 0;
	}

	TCritical theLock(TheLock);
	if (TheLD130Controllers[hIdx] != 0) {
		try {
			return TheLD130Controllers[hIdx]->getActiveBank();
		} catch (...){
			TheLD130LastErrors[0] = ld130ExceptionError;
		}
	}
	return 0;
}

