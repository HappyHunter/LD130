////////////////////////////////////////////////////////////////////
//
// 	Alexandr Filenkov 2010
//
////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#pragma hdrstop

#include "LD130Impl.h"
#include <stdio.h>

int ld130_min(int X1, int X2) { return  X1 < X2 ? X1 : X2; }
int ld130_max(int X1, int X2) { return  X1 > X2 ? X1 : X2; }

//-----------------------------------------------------------------------------------------
TLD130Impl::TLD130Impl()
	: m_activeBank (0)
	, m_sequenceIndex(0)
	, m_cfgFlags(0)
	, m_onLogEvent(0)
	, m_onLogObject(0)
{
	#if UNDER_CE
		::InitializeCriticalSection(&m_lock);
	#elif _WIN32_WINNT >= 0x0403
		::InitializeCriticalSectionAndSpinCount(&m_lock, 5000);

	#else
		::InitializeCriticalSection(&m_lock);
	#endif
}

//-----------------------------------------------------------------------------------------
TLD130Impl::~TLD130Impl()
{
	::DeleteCriticalSection(&m_lock);
}

//-----------------------------------------------------------------------------------------
TCommandErrorOutput TLD130Impl::init(int aComPort, int aSpeed,TLD130OnLogEvent onLogEvent, const void* anObject)
{
	TCritical theLock(m_lock);

	m_onLogEvent = onLogEvent;
	m_onLogObject = anObject;
	TCommandErrorOutput retCode = m_conMan.init(aComPort, aSpeed, onLogEvent);

	m_verMajor.clear();
	m_verMinor.clear();
	m_verBuild.clear();


	if (!retCode.hasError()) {
		m_verMajor = m_conMan.GetValueByName("vermajor");
		m_verMinor = m_conMan.GetValueByName("verminor");
		m_verBuild = m_conMan.GetValueByName("verbuild");
		readFromController();
	}
    return retCode;
}

//-----------------------------------------------------------------------------------------
bool TLD130Impl::isConnected() const
{
	TCritical theLock(m_lock);
	return m_conMan.isConnected() && !m_verMajor.empty() && !m_verMinor.empty() && !m_verBuild.empty();
}


//-----------------------------------------------------------------------------------------
void TLD130Impl::close()
{
	TCritical theLock(m_lock);
	m_conMan.close();
}


//-----------------------------------------------------------------------------------------
TCommandErrorOutput TLD130Impl::softTrig(int aTrigId)
{
	TCritical theLock(m_lock);
  	char cmdBuf[1024] = {0};

	// "softtrig,triggerId"
  	_snprintf (cmdBuf, sizeof(cmdBuf) / sizeof(cmdBuf[0]), "softtrig,%d\n", aTrigId);

	return m_conMan.send(cmdBuf);
}

//-----------------------------------------------------------------------------------------
void TLD130Impl::readFromController()
{
	TCritical theLock(m_lock);
	getConfigDataImpl();

	for (int i = 0; i < sizeof(m_bankData)/sizeof(m_bankData[0]); ++i) {
		getBankDataImpl(i);
	}

	getBankSequenceImpl();

	getStatus();
}

//-----------------------------------------------------------------------------------------
void TLD130Impl::getConfigDataImpl()
{
	TCritical theLock(m_lock);
  	char cmdBuf[1024] = {0};

	// "getcfgdata"
  	_snprintf (cmdBuf, sizeof(cmdBuf) / sizeof(cmdBuf[0]), "getcfgdata\n");

	// "cfgdata,flags,activeBank",
	TCommandErrorOutput retCode = m_conMan.send(cmdBuf);
	if (!retCode.hasError()) {
		m_activeBank = atoi(m_conMan.GetValueByName("activeBank"));
		m_cfgFlags = atoi(m_conMan.GetValueByName("flags"));
	}
}


//---------------------------------------------------------------------------
void TLD130Impl::getBankDataImpl(unsigned short aBankId)
{
	TCritical theLock(m_lock);
  	char cmdBuf[1024] = {0};

	//-----------------------------------------
	// Head 1

	// "getbankdata,bankId,headId"
  	_snprintf (cmdBuf, sizeof(cmdBuf) / sizeof(cmdBuf[0]), "getbankdata,%d,%d\n", aBankId, 1);

	// "bankdata,bankId,outputId,voltage,powerChanel1,powerChanel2,powerChanel3,powerChanel4,strobeDelay,strobeWidth,triggerEdge,triggerId,chanelAmplifier",
	TCommandErrorOutput retCode = m_conMan.send(cmdBuf);
	if (!retCode.hasError()) {
		m_bankData[aBankId].m_headData[0].m_bankId		= atoi(m_conMan.GetValueByName("bankId"));		// 1, 2, 3, 4
		m_bankData[aBankId].m_headData[0].m_outputId	= atoi(m_conMan.GetValueByName("outputId"));		// 1 - head 1, 2 - Head 2
		m_bankData[aBankId].m_headData[0].m_voltage		= atoi(m_conMan.GetValueByName("voltage"));		// 0 - 100 Volts
		m_bankData[aBankId].m_headData[0].m_powerChanel1= atoi(m_conMan.GetValueByName("powerChanel1")); // 0 - 100 00% with fixed decimal point at 2 digits
		m_bankData[aBankId].m_headData[0].m_powerChanel2= atoi(m_conMan.GetValueByName("powerChanel2")); // for example the power of 35.23% will be sent as 3523
		m_bankData[aBankId].m_headData[0].m_powerChanel3= atoi(m_conMan.GetValueByName("powerChanel3")); // the power of 99.00% will be sent as 9900
		m_bankData[aBankId].m_headData[0].m_powerChanel4= atoi(m_conMan.GetValueByName("powerChanel4")); // the power of 100.00% will be sent as 10000
		m_bankData[aBankId].m_headData[0].m_strobeDelay	= atoi(m_conMan.GetValueByName("strobeDelay"));	// the delay of outcoming light strobe in microseconds
		m_bankData[aBankId].m_headData[0].m_strobeWidth	= atoi(m_conMan.GetValueByName("strobeWidth"));	// the duration of outcoming light strobe in microseconds
		m_bankData[aBankId].m_headData[0].m_triggerEdge	= atoi(m_conMan.GetValueByName("triggerEdge"));	// the edge of incoming trigger to start counting, 0 - raising, 1 - falling, 2 - DC mode
		m_bankData[aBankId].m_headData[0].m_triggerId	= atoi(m_conMan.GetValueByName("triggerId"));	// the ID of the trigger this output head will trigger on. 1 - Trigger 1, 2 - Trigger 2, 3 - Trigger 1 or 2
		m_bankData[aBankId].m_headData[0].m_chanelAmplifier= atoi(m_conMan.GetValueByName("chanelAmplifier"));	 // the amplification value 1-5

	}

	//-----------------------------------------
	// Head 2

	// "getbankdata,bankId,headId"
  	_snprintf (cmdBuf, sizeof(cmdBuf) / sizeof(cmdBuf[0]), "getbankdata,%d,%d\n", aBankId, 2);

	// "bankdata,bankId,outputId,voltage,powerChanel1,powerChanel2,powerChanel3,powerChanel4,strobeDelay,strobeWidth,triggerEdge,triggerId,chanelAmplifier",
	retCode = m_conMan.send(cmdBuf);
	if (!retCode.hasError()) {
		m_bankData[aBankId].m_headData[1].m_bankId		= atoi(m_conMan.GetValueByName("bankId"));		// 1, 2, 3, 4
		m_bankData[aBankId].m_headData[1].m_outputId	= atoi(m_conMan.GetValueByName("outputId"));		// 1 - head 1, 2 - Head 2
		m_bankData[aBankId].m_headData[1].m_voltage		= atoi(m_conMan.GetValueByName("voltage"));		// 0 - 100 Volts
		m_bankData[aBankId].m_headData[1].m_powerChanel1= atoi(m_conMan.GetValueByName("powerChanel1")); // 0 - 100 00% with fixed decimal point at 2 digits
		m_bankData[aBankId].m_headData[1].m_powerChanel2= atoi(m_conMan.GetValueByName("powerChanel2")); // for example the power of 35.23% will be sent as 3523
		m_bankData[aBankId].m_headData[1].m_powerChanel3= atoi(m_conMan.GetValueByName("powerChanel3")); // the power of 99.00% will be sent as 9900
		m_bankData[aBankId].m_headData[1].m_powerChanel4= atoi(m_conMan.GetValueByName("powerChanel4")); // the power of 100.00% will be sent as 10000
		m_bankData[aBankId].m_headData[1].m_strobeDelay	= atoi(m_conMan.GetValueByName("strobeDelay"));	// the delay of outcoming light strobe in microseconds
		m_bankData[aBankId].m_headData[1].m_strobeWidth	= atoi(m_conMan.GetValueByName("strobeWidth"));	// the duration of outcoming light strobe in microseconds
		m_bankData[aBankId].m_headData[1].m_triggerEdge	= atoi(m_conMan.GetValueByName("triggerEdge"));	// the edge of incoming trigger to start counting, 0 - raising, 1 - falling, 2 - DC mode
		m_bankData[aBankId].m_headData[1].m_triggerId	= atoi(m_conMan.GetValueByName("triggerId"));	// the ID of the trigger this output head will trigger on. 1 - Trigger 1, 2 - Trigger 2, 3 - Trigger 1 or 2
		m_bankData[aBankId].m_headData[1].m_chanelAmplifier= atoi(m_conMan.GetValueByName("chanelAmplifier"));	 // the amplification value 1-5
	}

}

//---------------------------------------------------------------------------
void TLD130Impl::getBankSequenceImpl()
{
	TCritical theLock(m_lock);
  	char cmdBuf[1024] = {0};
	// "getseqdata"
  	_snprintf (cmdBuf, sizeof(cmdBuf) / sizeof(cmdBuf[0]), "getseqdata\n");

	// "seqdata,curIdx,seqdata",
	TCommandErrorOutput retCode = m_conMan.send(cmdBuf);
	if (!retCode.hasError()) {
		m_sequenceIndex = atoi(m_conMan.GetValueByName("curIdx"));		// the index of the position currently programmed to the controller
		m_sequence = string(m_conMan.GetValueByName("s_10")) +
					 string(m_conMan.GetValueByName("s_20")) +
					 string(m_conMan.GetValueByName("s_30")) +
					 string(m_conMan.GetValueByName("s_40")) +
					 string(m_conMan.GetValueByName("s_50")) +
					 string(m_conMan.GetValueByName("s_60")) +
					 string(m_conMan.GetValueByName("s_70")) +
					 string(m_conMan.GetValueByName("s_80")) +
					 string(m_conMan.GetValueByName("s_90")) +
					 string(m_conMan.GetValueByName("s_100")) +
					 string(m_conMan.GetValueByName("s_110")) +
					 string(m_conMan.GetValueByName("s_120")) +
					 string(m_conMan.GetValueByName("s_130")) +
					 string(m_conMan.GetValueByName("s_140")) +
					 string(m_conMan.GetValueByName("s_150")) +
					 string(m_conMan.GetValueByName("s_160")) +
					 string(m_conMan.GetValueByName("s_170")) +
					 string(m_conMan.GetValueByName("s_180")) +
					 string(m_conMan.GetValueByName("s_190")) +
					 string(m_conMan.GetValueByName("s_200")) +
					 string(m_conMan.GetValueByName("s_210")) +
					 string(m_conMan.GetValueByName("s_220")) +
					 string(m_conMan.GetValueByName("s_230")) +
			         string(m_conMan.GetValueByName("s_240")) +
			         string(m_conMan.GetValueByName("s_250")) +
					 string(m_conMan.GetValueByName("s_260")) +
					 string(m_conMan.GetValueByName("s_270")) +
					 string(m_conMan.GetValueByName("s_280")) +
					 string(m_conMan.GetValueByName("s_290")) +
					 string(m_conMan.GetValueByName("s_300")) +
					 string(m_conMan.GetValueByName("s_310")) +
					 string(m_conMan.GetValueByName("s_320")) +
					 string(m_conMan.GetValueByName("s_330")) +
					 string(m_conMan.GetValueByName("s_340")) +
					 string(m_conMan.GetValueByName("s_350")) +
					 string(m_conMan.GetValueByName("s_360")) +
					 string(m_conMan.GetValueByName("s_370")) +
					 string(m_conMan.GetValueByName("s_380")) +
					 string(m_conMan.GetValueByName("s_390")) +
					 string(m_conMan.GetValueByName("s_400"))
		 ;
	}
}

//---------------------------------------------------------------------------
TBankHeadData  TLD130Impl::getBankHeadData(unsigned short aBankId/*1,2,3,4*/, unsigned short aHeadId /*1,2*/) const
{
	TCritical theLock(m_lock);
	aBankId 	 = ld130_min(ld130_max(aBankId,		1), 	MAX_NUM_OF_BANKS);	// 1,2,3,4
	aHeadId 	 = ld130_min(ld130_max(aHeadId, 	1), 	2);                 // 1,2

	return m_bankData[aBankId-1].m_headData[aHeadId-1];
}

//---------------------------------------------------------------------------
THeadStatus  TLD130Impl::getHeadStatus(unsigned short aHeadId /*1,2*/)
{
	TCritical theLock(m_lock);
	aHeadId 	 = ld130_min(ld130_max(aHeadId, 	1), 	2);                 // 1,2

  	char cmdBuf[1024] = {0};

	// "gethstatus,headId"
  	_snprintf (cmdBuf, sizeof(cmdBuf) / sizeof(cmdBuf[0]), "gethstatus,%d\n", aHeadId);

    // "hstatus,statusChanel1,statusChanel2,statusChanel3,statusChanel4",
	TCommandErrorOutput retCode = m_conMan.send(cmdBuf);
	if (!retCode.hasError()) {
	    m_headStatus[aHeadId-1].m_statusChanel1 = atoi(m_conMan.GetValueByName("statusChanel1"));
	    m_headStatus[aHeadId-1].m_statusChanel2 = atoi(m_conMan.GetValueByName("statusChanel2"));
	    m_headStatus[aHeadId-1].m_statusChanel3 = atoi(m_conMan.GetValueByName("statusChanel3"));
	    m_headStatus[aHeadId-1].m_statusChanel4 = atoi(m_conMan.GetValueByName("statusChanel4"));
	}
	return m_headStatus[aHeadId-1];
}

//---------------------------------------------------------------------------
TCommandErrorOutput TLD130Impl::setBankHeadData(unsigned short aBankId,			//1,2,3,4
									unsigned short aHeadId, 		//1,2
								    unsigned short aVoltage,		// 0 - 100 Volts
								    unsigned short aPowerChanel1, 	// 0 - 100 00% with fixed decimal point at 2 digits
								    unsigned short aPowerChanel2,	// for example the power of 35.23% will be sent as 3523
								    unsigned short aPowerChanel3,	// the power of 99.00% will be sent as 9900
								    unsigned short aPowerChanel4,	// the power of 100.00% will be sent as 10000
								    unsigned long  aStrobeDelay,	// the delay of outcoming light strobe in microseconds
								    unsigned long  aStrobeWidth,	// the duration of outcoming light strobe in microseconds
								    unsigned short aTriggerEdge,	// the edge of incoming trigger to start counting, 0 - raising, 1 - falling, 2 - DC mode
								    unsigned short aTriggerId,		// the ID of the trigger this output head will trigger on. 1 - Trigger 1, 2 - Trigger 2, 3 - Trigger 1 or 2
								    unsigned short aChanelAmplifier	 // the amplification value 1-5
)
{
	TCritical theLock(m_lock);
	aBankId 	 = ld130_min(ld130_max(aBankId,		1), 	MAX_NUM_OF_BANKS);	// 1,2,3,4
	aHeadId 	 = ld130_min(ld130_max(aHeadId, 	1), 	2);                 // 1,2

	TBankHeadData theData;

	theData.m_bankId		= aBankId;			//1,2,3,4
	theData.m_outputId		= aHeadId; 			//1,2
	theData.m_voltage		= aVoltage;			// 0 - 100 Volts
	theData.m_powerChanel1	= aPowerChanel1; 	// 0 - 100 00% with fixed decimal point at 2 digits
	theData.m_powerChanel2	= aPowerChanel2;	// for example the power of 35.23% will be sent as 3523
	theData.m_powerChanel3	= aPowerChanel3;	// the power of 99.00% will be sent as 9900
	theData.m_powerChanel4	= aPowerChanel4;	// the power of 100.00% will be sent as 10000
	theData.m_strobeDelay	= aStrobeDelay;		// the delay of outcoming light strobe in microseconds
	theData.m_strobeWidth	= aStrobeWidth;		// the duration of outcoming light strobe in microseconds
	theData.m_triggerEdge	= aTriggerEdge;		// the edge of incoming trigger to start counting, 0 - raising, 1 - falling, 2 - DC mode
	theData.m_triggerId		= aTriggerId;		// the ID of the trigger this output head will trigger on. 1 - Trigger 1, 2 - Trigger 2, 3 - Trigger 1 or 2
	theData.m_chanelAmplifier=aChanelAmplifier;	// the amplification value 1-5
    return setBankHeadData(theData);
}

//---------------------------------------------------------------------------
TCommandErrorOutput TLD130Impl::setBankHeadData(TBankHeadData aData)
{
	TCritical theLock(m_lock);

	// update error checking for properties
	aData.m_bankId 		 = ld130_min(ld130_max(aData.m_bankId, 		1), 	MAX_NUM_OF_BANKS);	// 1,2,3,4
	aData.m_outputId 	 = ld130_min(ld130_max(aData.m_outputId, 		1), 	2);                 // 1,2
	aData.m_voltage 	 = ld130_min(ld130_max(aData.m_voltage, 		0), 	100);               // 0 - 100 Volts
	aData.m_powerChanel1 = ld130_min(ld130_max(aData.m_powerChanel1,	0), 	10000);			    // 0 - 100 00% with fixed decimal point at 2 digits
	aData.m_powerChanel2 = ld130_min(ld130_max(aData.m_powerChanel2,	0), 	10000);             // for example the power of 35.23% will be sent as 3523
	aData.m_powerChanel3 = ld130_min(ld130_max(aData.m_powerChanel3,	0), 	10000);             // the power of 99.00% will be sent as 9900
	aData.m_powerChanel4 = ld130_min(ld130_max(aData.m_powerChanel4,	0), 	10000);             // the power of 100.00% will be sent as 10000
	aData.m_strobeDelay  = ld130_min(ld130_max(aData.m_strobeDelay, 	0),  	0xFFFFFFFF);        // the delay of outcoming light strobe in microseconds
	aData.m_strobeWidth  = ld130_min(ld130_max(aData.m_strobeWidth, 	0),  	0xFFFFFFFF);        // the duration of outcoming light strobe in microseconds
	aData.m_triggerEdge  = ld130_min(ld130_max(aData.m_triggerEdge, 	0),  	2);                 // the edge of incoming trigger to start counting, 0 - raising, 1 - falling, 2 - DC mode
	aData.m_triggerId    = ld130_min(ld130_max(aData.m_triggerId, 	1),  	3);                 // the ID of the trigger this output head will trigger on. 1 - Trigger 1, 2 - Trigger 2, 3 - Trigger 1 or 2
	aData.m_chanelAmplifier= ld130_min(ld130_max(aData.m_chanelAmplifier, 	1),  	5);         // the amplification value 1-5

	// save the data localy
	m_bankData[aData.m_bankId-1].m_headData[aData.m_outputId-1] = aData;

  	char cmdBuf[1024] = {0};

	// "setbankdata,bankId,outputId,voltage,powerChanel1,powerChanel2,powerChanel3,powerChanel4,strobeDelay,strobeWidth,triggerEdge,triggerId,chanelAmplifier",
  	_snprintf (cmdBuf, sizeof(cmdBuf) / sizeof(cmdBuf[0]), "setbankdata,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
		aData.m_bankId,			// 1,2,3,4
		aData.m_outputId, 		// 1,2
		aData.m_voltage,		// 0 - 100 Volts
		aData.m_powerChanel1, 	// 0 - 100 00% with fixed decimal point at 2 digits
		aData.m_powerChanel2,	// for example the power of 35.23% will be sent as 3523
		aData.m_powerChanel3,	// the power of 99.00% will be sent as 9900
		aData.m_powerChanel4,	// the power of 100.00% will be sent as 10000
		aData.m_strobeDelay,	// the delay of outcoming light strobe in microseconds
		aData.m_strobeWidth,	// the duration of outcoming light strobe in microseconds
		aData.m_triggerEdge,	// the edge of incoming trigger to start counting, 0 - raising, 1 - falling, 2 - DC mode
		aData.m_triggerId,		// the ID of the trigger this output head will trigger on. 1 - Trigger 1, 2 - Trigger 2, 3 - Trigger 1 or 2
		aData.m_chanelAmplifier	// the amplification value 1-5
	);

	return m_conMan.send(cmdBuf);
}

//---------------------------------------------------------------------------
TCommandErrorOutput TLD130Impl::setActiveBank(unsigned short aBankId)
{
	TCritical theLock(m_lock);

	if (aBankId < 1) aBankId = 1;
	if (aBankId > MAX_NUM_OF_BANKS) aBankId = MAX_NUM_OF_BANKS;

  	char cmdBuf[1024] = {0};
    //"setbank,bankId",
  	_snprintf (cmdBuf, sizeof(cmdBuf) / sizeof(cmdBuf[0]), "setbank,%d\n", aBankId);
	m_activeBank = aBankId;
	return m_conMan.send(cmdBuf);
}

//---------------------------------------------------------------------------
TCommandErrorOutput TLD130Impl::setSequence(const string& aValue)
{
	TCritical theLock(m_lock);

	//"setseqdata,s_10,s_20,s_30,s_40,s_50,s_60,s_70,s_80,s_90,s_100,s_110,s_120,s_130,s_140,s_150,s_160,s_170,s_180,s_190,s_200,s_210,s_220,s_230,s_240,s_250,s_260,s_270,s_280,s_290,s_300,s_310,s_320,s_330,s_340,s_350,s_360,s_370,s_380,s_390,s_400",

	string theCmd = "setseqdata,";

	// clear our internal sequence first
	m_sequence.clear();

	// we need to split the bank data in groups of 10
	int iIndex = 0;
	for (unsigned int iStr = 0; iStr < aValue.length(); ++iStr) {

		// some validity of data
		if (aValue[iStr] >= '1' && aValue[iStr] < ('1' + MAX_NUM_OF_BANKS)) {
			if ((++iIndex % 10) == 0){
				theCmd += ',';
			}
			theCmd += aValue[iStr];

			// update our internal sequence as well
			m_sequence += aValue[iStr];
		}
	}

	theCmd += '\n';
	return m_conMan.send(theCmd.c_str());
}


//---------------------------------------------------------------------------
TLD130Status TLD130Impl::getStatus()
{
	TCritical theLock(m_lock);

	TLD130Status retVal;
  	char cmdBuf[1024] = {0};

	// "gethstatus,headId"
  	_snprintf (cmdBuf, sizeof(cmdBuf) / sizeof(cmdBuf[0]), "getstatus\n");

    // "status,TH1,TH2,TAmb",
	TCommandErrorOutput retCode = m_conMan.send(cmdBuf);
	if (!retCode.hasError()) {
	    retVal.m_temperatureH1 = (float)atof(m_conMan.GetValueByName("TH1"));
	    retVal.m_temperatureH2 = (float)atof(m_conMan.GetValueByName("TH2"));
	    retVal.m_temperatureAmb = (float)atof(m_conMan.GetValueByName("TAmb"));
	}
	return retVal;
}

