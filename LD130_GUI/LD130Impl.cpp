////////////////////////////////////////////////////////////////////
//
// 	Alexandr Filenkov 2010
//
////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#pragma hdrstop

#include "LD130Impl.h"
#include <stdio.h>

//-----------------------------------------------------------------------------------------
TLD130Impl::TLD130Impl()
	: m_activeBank (0)
	, m_sequenceIndex(0)
	, m_cfgFlags(0)
	, m_onLogEvent(0)
{
}

//-----------------------------------------------------------------------------------------
TLD130Impl::~TLD130Impl()
{
}

//-----------------------------------------------------------------------------------------
TCommandErrorOutput TLD130Impl::init(int aComPort, int aSpeed,TLD130OnLogEvent onLogEvent)
{
	m_onLogEvent = onLogEvent;
	TCommandErrorOutput retCode = m_conMan.init(aComPort, aSpeed, onLogEvent);

	m_verMajor.clear();
	m_verMinor.clear();
	m_verBuild.clear();


	if (!retCode.hasError()) {
		m_verMajor = TSerialConMan::GetValueByName("vermajor");
		m_verMinor = TSerialConMan::GetValueByName("verminor");
		m_verBuild = TSerialConMan::GetValueByName("verbuild");
		readFromController();
	}
    return retCode;
}

//-----------------------------------------------------------------------------------------
bool TLD130Impl::isConnected() const
{
	return m_conMan.isConnected() && !m_verMajor.empty() && !m_verMinor.empty() && !m_verBuild.empty();
}


//-----------------------------------------------------------------------------------------
void TLD130Impl::close()
{
	m_conMan.close();
}


//-----------------------------------------------------------------------------------------
TCommandErrorOutput TLD130Impl::softTrig(int aTrigId)
{
  	char cmdBuf[1024] = {0};

	// "softtrig,triggerId"
  	_snprintf (cmdBuf, sizeof(cmdBuf) / sizeof(cmdBuf[0]), "softtrig,%d\n", aTrigId);

	return m_conMan.send(cmdBuf);
}

//-----------------------------------------------------------------------------------------
void TLD130Impl::readFromController()
{
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
  	char cmdBuf[1024] = {0};

	// "getcfgdata"
  	_snprintf (cmdBuf, sizeof(cmdBuf) / sizeof(cmdBuf[0]), "getcfgdata\n");

	// "cfgdata,flags,activeBank",
	TCommandErrorOutput retCode = m_conMan.send(cmdBuf);
	if (!retCode.hasError()) {
		m_activeBank = atoi(TSerialConMan::GetValueByName("activeBank"));
		m_cfgFlags = atoi(TSerialConMan::GetValueByName("flags"));
	}
}


//---------------------------------------------------------------------------
void TLD130Impl::getBankDataImpl(unsigned short aBankId)
{
  	char cmdBuf[1024] = {0};

	//-----------------------------------------
	// Head 1

	// "getbankdata,bankId,headId"
  	_snprintf (cmdBuf, sizeof(cmdBuf) / sizeof(cmdBuf[0]), "getbankdata,%d,%d\n", aBankId, 1);

	// "bankdata,bankId,outputId,voltage,powerChanel1,powerChanel2,powerChanel3,powerChanel4,strobeDelay,strobeWidth,triggerEdge,triggerId,chanelAmplifier",
	TCommandErrorOutput retCode = m_conMan.send(cmdBuf);
	if (!retCode.hasError()) {
		m_bankData[aBankId].m_headData[0].m_bankId		= atoi(TSerialConMan::GetValueByName("bankId"));		// 1, 2, 3, 4
		m_bankData[aBankId].m_headData[0].m_outputId	= atoi(TSerialConMan::GetValueByName("outputId"));		// 1 - head 1, 2 - Head 2
		m_bankData[aBankId].m_headData[0].m_voltage		= atoi(TSerialConMan::GetValueByName("voltage"));		// 0 - 100 Volts
		m_bankData[aBankId].m_headData[0].m_powerChanel1= atoi(TSerialConMan::GetValueByName("powerChanel1")); // 0 - 100 00% with fixed decimal point at 2 digits
		m_bankData[aBankId].m_headData[0].m_powerChanel2= atoi(TSerialConMan::GetValueByName("powerChanel2")); // for example the power of 35.23% will be sent as 3523
		m_bankData[aBankId].m_headData[0].m_powerChanel3= atoi(TSerialConMan::GetValueByName("powerChanel3")); // the power of 99.00% will be sent as 9900
		m_bankData[aBankId].m_headData[0].m_powerChanel4= atoi(TSerialConMan::GetValueByName("powerChanel4")); // the power of 100.00% will be sent as 10000
		m_bankData[aBankId].m_headData[0].m_strobeDelay	= atoi(TSerialConMan::GetValueByName("strobeDelay"));	// the delay of outcoming light strobe in microseconds
		m_bankData[aBankId].m_headData[0].m_strobeWidth	= atoi(TSerialConMan::GetValueByName("strobeWidth"));	// the duration of outcoming light strobe in microseconds
		m_bankData[aBankId].m_headData[0].m_triggerEdge	= atoi(TSerialConMan::GetValueByName("triggerEdge"));	// the edge of incoming trigger to start counting, 0 - raising, 1 - falling, 2 - DC mode
		m_bankData[aBankId].m_headData[0].m_triggerId	= atoi(TSerialConMan::GetValueByName("triggerId"));	// the ID of the trigger this output head will trigger on. 1 - Trigger 1, 2 - Trigger 2, 3 - Trigger 1 or 2
		m_bankData[aBankId].m_headData[0].m_chanelAmplifier= atoi(TSerialConMan::GetValueByName("chanelAmplifier"));	 // the amplification value 1-5

	}

	//-----------------------------------------
	// Head 2

	// "getbankdata,bankId,headId"
  	_snprintf (cmdBuf, sizeof(cmdBuf) / sizeof(cmdBuf[0]), "getbankdata,%d,%d\n", aBankId, 2);

	// "bankdata,bankId,outputId,voltage,powerChanel1,powerChanel2,powerChanel3,powerChanel4,strobeDelay,strobeWidth,triggerEdge,triggerId,chanelAmplifier",
	retCode = m_conMan.send(cmdBuf);
	if (!retCode.hasError()) {
		m_bankData[aBankId].m_headData[1].m_bankId		= atoi(TSerialConMan::GetValueByName("bankId"));		// 1, 2, 3, 4
		m_bankData[aBankId].m_headData[1].m_outputId	= atoi(TSerialConMan::GetValueByName("outputId"));		// 1 - head 1, 2 - Head 2
		m_bankData[aBankId].m_headData[1].m_voltage		= atoi(TSerialConMan::GetValueByName("voltage"));		// 0 - 100 Volts
		m_bankData[aBankId].m_headData[1].m_powerChanel1= atoi(TSerialConMan::GetValueByName("powerChanel1")); // 0 - 100 00% with fixed decimal point at 2 digits
		m_bankData[aBankId].m_headData[1].m_powerChanel2= atoi(TSerialConMan::GetValueByName("powerChanel2")); // for example the power of 35.23% will be sent as 3523
		m_bankData[aBankId].m_headData[1].m_powerChanel3= atoi(TSerialConMan::GetValueByName("powerChanel3")); // the power of 99.00% will be sent as 9900
		m_bankData[aBankId].m_headData[1].m_powerChanel4= atoi(TSerialConMan::GetValueByName("powerChanel4")); // the power of 100.00% will be sent as 10000
		m_bankData[aBankId].m_headData[1].m_strobeDelay	= atoi(TSerialConMan::GetValueByName("strobeDelay"));	// the delay of outcoming light strobe in microseconds
		m_bankData[aBankId].m_headData[1].m_strobeWidth	= atoi(TSerialConMan::GetValueByName("strobeWidth"));	// the duration of outcoming light strobe in microseconds
		m_bankData[aBankId].m_headData[1].m_triggerEdge	= atoi(TSerialConMan::GetValueByName("triggerEdge"));	// the edge of incoming trigger to start counting, 0 - raising, 1 - falling, 2 - DC mode
		m_bankData[aBankId].m_headData[1].m_triggerId	= atoi(TSerialConMan::GetValueByName("triggerId"));	// the ID of the trigger this output head will trigger on. 1 - Trigger 1, 2 - Trigger 2, 3 - Trigger 1 or 2
		m_bankData[aBankId].m_headData[1].m_chanelAmplifier= atoi(TSerialConMan::GetValueByName("chanelAmplifier"));	 // the amplification value 1-5
	}

}

//---------------------------------------------------------------------------
void TLD130Impl::getBankSequenceImpl()
{
  	char cmdBuf[1024] = {0};
	// "getseqdata"
  	_snprintf (cmdBuf, sizeof(cmdBuf) / sizeof(cmdBuf[0]), "getseqdata\n");

	// "seqdata,curIdx,seqdata",
	TCommandErrorOutput retCode = m_conMan.send(cmdBuf);
	if (!retCode.hasError()) {
		m_sequenceIndex = atoi(TSerialConMan::GetValueByName("curIdx"));		// the index of the position currently programmed to the controller
		m_sequence = string(TSerialConMan::GetValueByName("s_10")) +
					 string(TSerialConMan::GetValueByName("s_20")) +
					 string(TSerialConMan::GetValueByName("s_30")) +
					 string(TSerialConMan::GetValueByName("s_40")) +
					 string(TSerialConMan::GetValueByName("s_50")) +
					 string(TSerialConMan::GetValueByName("s_60")) +
					 string(TSerialConMan::GetValueByName("s_70")) +
					 string(TSerialConMan::GetValueByName("s_80")) +
					 string(TSerialConMan::GetValueByName("s_90")) +
					 string(TSerialConMan::GetValueByName("s_100")) +
					 string(TSerialConMan::GetValueByName("s_110")) +
					 string(TSerialConMan::GetValueByName("s_120")) +
					 string(TSerialConMan::GetValueByName("s_130")) +
					 string(TSerialConMan::GetValueByName("s_140")) +
					 string(TSerialConMan::GetValueByName("s_150")) +
					 string(TSerialConMan::GetValueByName("s_160")) +
					 string(TSerialConMan::GetValueByName("s_170")) +
					 string(TSerialConMan::GetValueByName("s_180")) +
					 string(TSerialConMan::GetValueByName("s_190")) +
					 string(TSerialConMan::GetValueByName("s_200")) +
					 string(TSerialConMan::GetValueByName("s_210")) +
					 string(TSerialConMan::GetValueByName("s_220")) +
					 string(TSerialConMan::GetValueByName("s_230")) +
			         string(TSerialConMan::GetValueByName("s_240")) +
			         string(TSerialConMan::GetValueByName("s_250")) +
					 string(TSerialConMan::GetValueByName("s_260")) +
					 string(TSerialConMan::GetValueByName("s_270")) +
					 string(TSerialConMan::GetValueByName("s_280")) +
					 string(TSerialConMan::GetValueByName("s_290")) +
					 string(TSerialConMan::GetValueByName("s_300")) +
					 string(TSerialConMan::GetValueByName("s_310")) +
					 string(TSerialConMan::GetValueByName("s_320")) +
					 string(TSerialConMan::GetValueByName("s_330")) +
					 string(TSerialConMan::GetValueByName("s_340")) +
					 string(TSerialConMan::GetValueByName("s_350")) +
					 string(TSerialConMan::GetValueByName("s_360")) +
					 string(TSerialConMan::GetValueByName("s_370")) +
					 string(TSerialConMan::GetValueByName("s_380")) +
					 string(TSerialConMan::GetValueByName("s_390")) +
					 string(TSerialConMan::GetValueByName("s_400"))
		 ;
	}
}

//---------------------------------------------------------------------------
TBankHeadData  TLD130Impl::getBankHeadData(unsigned short aBankId/*1,2,3,4*/, unsigned short aHeadId /*1,2*/) const
{
	aBankId 	 = std::min(std::max(aBankId,		1), 	MAX_NUM_OF_BANKS);	// 1,2,3,4
	aHeadId 	 = std::min(std::max(aHeadId, 	1), 	2);                 // 1,2

	return m_bankData[aBankId-1].m_headData[aHeadId-1];
}

//---------------------------------------------------------------------------
THeadStatus  TLD130Impl::getHeadStatus(unsigned short aHeadId /*1,2*/)
{
	aHeadId 	 = std::min(std::max(aHeadId, 	1), 	2);                 // 1,2

  	char cmdBuf[1024] = {0};

	// "gethstatus,headId"
  	_snprintf (cmdBuf, sizeof(cmdBuf) / sizeof(cmdBuf[0]), "gethstatus,%d\n", aHeadId);

    // "hstatus,statusChanel1,statusChanel2,statusChanel3,statusChanel4",
	TCommandErrorOutput retCode = m_conMan.send(cmdBuf);
	if (!retCode.hasError()) {
	    m_headStatus[aHeadId-1].m_statusChanel1 = atoi(TSerialConMan::GetValueByName("statusChanel1"));
	    m_headStatus[aHeadId-1].m_statusChanel2 = atoi(TSerialConMan::GetValueByName("statusChanel2"));
	    m_headStatus[aHeadId-1].m_statusChanel3 = atoi(TSerialConMan::GetValueByName("statusChanel3"));
	    m_headStatus[aHeadId-1].m_statusChanel4 = atoi(TSerialConMan::GetValueByName("statusChanel4"));
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
	aBankId 	 = std::min(std::max(aBankId,		1), 	MAX_NUM_OF_BANKS);	// 1,2,3,4
	aHeadId 	 = std::min(std::max(aHeadId, 	1), 	2);                 // 1,2

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

	// update error checking for properties
	aData.m_bankId 		 = std::min(std::max(aData.m_bankId, 		1), 	MAX_NUM_OF_BANKS);	// 1,2,3,4
	aData.m_outputId 	 = std::min(std::max(aData.m_outputId, 		1), 	2);                 // 1,2
	aData.m_voltage 	 = std::min(std::max(aData.m_voltage, 		0), 	100);               // 0 - 100 Volts
	aData.m_powerChanel1 = std::min(std::max(aData.m_powerChanel1,	0), 	10000);			    // 0 - 100 00% with fixed decimal point at 2 digits
	aData.m_powerChanel2 = std::min(std::max(aData.m_powerChanel2,	0), 	10000);             // for example the power of 35.23% will be sent as 3523
	aData.m_powerChanel3 = std::min(std::max(aData.m_powerChanel3,	0), 	10000);             // the power of 99.00% will be sent as 9900
	aData.m_powerChanel4 = std::min(std::max(aData.m_powerChanel4,	0), 	10000);             // the power of 100.00% will be sent as 10000
	aData.m_strobeDelay  = std::min(std::max(aData.m_strobeDelay, 	0),  	0xFFFFFFFF);        // the delay of outcoming light strobe in microseconds
	aData.m_strobeWidth  = std::min(std::max(aData.m_strobeWidth, 	0),  	0xFFFFFFFF);        // the duration of outcoming light strobe in microseconds
	aData.m_triggerEdge  = std::min(std::max(aData.m_triggerEdge, 	0),  	2);                 // the edge of incoming trigger to start counting, 0 - raising, 1 - falling, 2 - DC mode
	aData.m_triggerId    = std::min(std::max(aData.m_triggerId, 	1),  	3);                 // the ID of the trigger this output head will trigger on. 1 - Trigger 1, 2 - Trigger 2, 3 - Trigger 1 or 2
	aData.m_chanelAmplifier= std::min(std::max(aData.m_chanelAmplifier, 	1),  	5);         // the amplification value 1-5

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
	TLD130Status retVal;
  	char cmdBuf[1024] = {0};

	// "gethstatus,headId"
  	_snprintf (cmdBuf, sizeof(cmdBuf) / sizeof(cmdBuf[0]), "getstatus\n");

    // "status,TH1,TH2,TAmb",
	TCommandErrorOutput retCode = m_conMan.send(cmdBuf);
	if (!retCode.hasError()) {
	    retVal.m_temperatureH1 = atof(TSerialConMan::GetValueByName("TH1"));
	    retVal.m_temperatureH2 = atof(TSerialConMan::GetValueByName("TH2"));
	    retVal.m_temperatureAmb = atof(TSerialConMan::GetValueByName("TAmb"));
	}
	return retVal;
}

