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

	if (!retCode.hasError()) {
		m_verMajor = TSerialConMan::GetValueByName("vermajor");
		m_verMinor = TSerialConMan::GetValueByName("verminor");
		m_verBuild = TSerialConMan::GetValueByName("verbuild");
		readFromController();
	}
    return retCode;
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
  	char cmdBuf[1024] = {0};

	// "getcfgdata"
  	_snprintf (cmdBuf, sizeof(cmdBuf) / sizeof(cmdBuf[0]), "getcfgdata\n");

	// "cfgdata,flags,activeBank",
	TCommandErrorOutput retCode = m_conMan.send(cmdBuf);
	if (!retCode.hasError()) {
		m_activeBank = atoi(TSerialConMan::GetValueByName("activeBank"));
		m_cfgFlags = atoi(TSerialConMan::GetValueByName("flags"));
	}

	for (int i = 0; i < sizeof(m_bankData)/sizeof(m_bankData[0]); ++i) {
		getBankDataImpl(i);
	}

	getBankSequenceImpl();
//
//  updateControlsForSelectedBank();
//
//	TCmdGetHeadData cmd;
//	cmd.m_outputId		= 1;//		// 1 - head 1, 2 - Head 2
//
//	THeadData cmdReply;
//	TCommandErrorOutput errCode = sendCommand(&cmd, sizeof(cmd), &cmdReply, sizeof(cmdReply));
//
//	if (cmdReply.m_outputId != cmd.m_outputId)
//		return ;
}

//---------------------------------------------------------------------------
void TLD130Impl::getBankDataImpl(unsigned char aBankId)
{
  	char cmdBuf[1024] = {0};

	//-----------------------------------------
	// Head 1

	// "getbankdata,bankId,headId"
  	_snprintf (cmdBuf, sizeof(cmdBuf) / sizeof(cmdBuf[0]), "getbankdata,%d,%d\n", aBankId, 1);

	// "bankdata,bankId,outputId,voltage,powerChanel1,powerChanel2,powerChanel3,powerChanel4,strobeDelay,strobeWidth,triggerEdge,triggerId,chanelAmplifier",
	TCommandErrorOutput retCode = m_conMan.send(cmdBuf);
	if (!retCode.hasError()) {
		m_bankData[aBankId].m_headData[0].m_bankId		= atoi(TSerialConMan::GetValueByName("bankId"));		// 0, 1, 2, 3, 4
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
		m_bankData[aBankId].m_headData[1].m_bankId		= atoi(TSerialConMan::GetValueByName("bankId"));		// 0, 1, 2, 3, 4
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
			         string(TSerialConMan::GetValueByName("s_250")) ;
	}
}

