////////////////////////////////////////////////////////////////////
//
// 	Alexandr Filenkov 2010
//
////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include <cstdio>
#include <ctime>
#include <cassert>

#include "..\LD130_FW\parser.h"

#include "MainWindow.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "RzPanel"
#pragma link "RzSplit"
#pragma link "RzTrkBar"
#pragma link "RzEdit"
#pragma link "RzButton"
#pragma link "RzLine"
#pragma link "RzRadChk"
#pragma link "RzLabel"
#pragma link "RzCmboBx"
#pragma link "RzLstBox"
#pragma link "RzTabs"
#pragma link "RzRadGrp"
#pragma link "RzStatus"
#pragma link "cxContainer"
#pragma link "cxControls"
#pragma link "cxEdit"
#pragma link "cxMaskEdit"
#pragma link "cxTextEdit"
#pragma link "RzCommon"
#pragma link "RzSpnEdt"
#pragma resource "*.dfm"
TFormMainWindow *FormMainWindow;
#include "CommonData.h"
#include "ComPortConfig.h"

using namespace std;



//---------------------------------------------------------------------------
__fastcall TFormMainWindow::TFormMainWindow(TComponent* Owner)
: TForm(Owner)
, m_updateActiveBank (false)
{
	m_headControlInfo.addControlGroup (0, edChanel1Head1, tbChanel1Head1);
	m_headControlInfo.addControlGroup (0, edChanel2Head1, tbChanel2Head1);
	m_headControlInfo.addControlGroup (0, edChanel3Head1, tbChanel3Head1);
	m_headControlInfo.addControlGroup (0, edChanel4Head1, tbChanel4Head1);
	m_headControlInfo.addControlGroup (0, edVoltageHead1, tbVoltageHead1);
	m_headControlInfo.addControlGroup (0, edWidthHead1, 0);
	m_headControlInfo.addControlGroup (0, edDelayHead1, 0);
	m_headControlInfo.addControlGroup (1, edChanel1Head2, tbChanel1Head2);
	m_headControlInfo.addControlGroup (1, edChanel2Head2, tbChanel2Head2);
	m_headControlInfo.addControlGroup (1, edChanel3Head2, tbChanel3Head2);
	m_headControlInfo.addControlGroup (1, edChanel4Head2, tbChanel4Head2);
	m_headControlInfo.addControlGroup (1, edVoltageHead2, tbVoltageHead2);
	m_headControlInfo.addControlGroup (1, edWidthHead2, 0);
	m_headControlInfo.addControlGroup (1, edDelayHead2, 0);

}
//---------------------------------------------------------------------------

void __fastcall TFormMainWindow::TrackBarChange(TObject* Sender)
{
	TRzTrackBar* pTrackBar = dynamic_cast<TRzTrackBar*>(Sender);

	if (THeadControlGroup* pGroup = m_headControlInfo.findHeadControlGroupByTrackBar (pTrackBar)) {

		if (!m_headControlInfo.m_powerChangingHead[pGroup->m_headIndex]) {
			m_headControlInfo.m_powerChangingHead[pGroup->m_headIndex] = true;

			if (pGroup->m_pEdit)
				pGroup->m_pEdit->Value = pTrackBar->Position;	//update the associated trackbar, if specified


			m_headControlInfo.m_updateHead[pGroup->m_headIndex] = true;
			m_headControlInfo.m_powerChangingHead[pGroup->m_headIndex] = false;
		}
	}
}
//---------------------------------------------------------------------------

void __fastcall TFormMainWindow::EditChange(TObject* Sender)
{
	TRzNumericEdit* pEdit = dynamic_cast<TRzNumericEdit*>(Sender);
	if (THeadControlGroup* pGroup = m_headControlInfo.findHeadControlGroupByEdit (pEdit)) {

		if (!m_headControlInfo.m_powerChangingHead[pGroup->m_headIndex]) {
			m_headControlInfo.m_powerChangingHead[pGroup->m_headIndex] = true;

			if (pGroup->m_pTrackBar)
				pGroup->m_pTrackBar->Position = pEdit->Value;	//update the associated edit box, if specified


			m_headControlInfo.m_updateHead[pGroup->m_headIndex] = true;
			m_headControlInfo.m_powerChangingHead[pGroup->m_headIndex] = false;
		}
	}
}
//---------------------------------------------------------------------------

void __fastcall TFormMainWindow::TimerUpdateTimer(TObject *Sender)
{
	const int bankIndex = rzchkEnableBanks->Checked ? RzTabBanks->TabIndex+1 : 1;
	bool bUpdated1 = false;
	if (m_headControlInfo.m_updateHead[0]) {
		m_headControlInfo.m_updateHead[0] = false;

		int triggerMode = TriggerRaising;
		if (rbRaisingTriggerHead1->Checked) {
			triggerMode = TriggerRaising;
		} else if (rbFallingTriggerHead1->Checked) {
			triggerMode = TriggerFalling;
		} else if (rzDCModeHead1->Checked) {
			triggerMode = TriggerDC;
		}


		// limit the amplifier to 1 in DC mode
		int theAmplifier = triggerMode != TriggerDC ? grHead1Amplifier->ItemIndex + 1 : 1;

		TCommandErrorOutput retCode = m_LD130.setBankHeadData(
															 bankIndex,
															 1,								   // 1 - head 1, 2 - Head 2
															 int(edVoltageHead1->IntValue),	   // 0 - 100 Volts
															 int(edChanel1Head1->Value*100.f), // 0 - 100 00% with fixed decimal point at 2 digits
															 int(edChanel2Head1->Value*100.f), // for example the power of 35.23% will be sent as 3523
															 int(edChanel3Head1->Value*100.f), // the power of 99.00% will be sent as 9900
															 int(edChanel4Head1->Value*100.f), // the power of 100.00% will be sent as 10000
															 int(edDelayHead1->IntValue),	   // the delay of outcoming light strobe in microseconds
															 int(edWidthHead1->IntValue),	   // the duration of outcoming light strobe in microseconds
															 triggerMode, // the edge of incoming trigger to start counting, 0 - raising, 1 - falling
															 3,	 								// the ID of the trigger this output head will trigger on. 1 - Trigger 1, 2 - Trigger 2, 3 - Trigger 1 or 2
															 theAmplifier 						// the amplification value 1-5
															 );

		if (retCode.hasError()) {
			lbLog->Items->Add(AnsiString((char*)retCode.m_errorDescription));
            lbFullLog->Items->Add(AnsiString((char*)retCode.m_errorDescription));
		}
		bUpdated1 = true;
	}

	bool bUpdated2 = false;
	if (m_headControlInfo.m_updateHead[1]) {
		m_headControlInfo.m_updateHead[1] = false;

		int triggerMode = TriggerRaising;
		if (rbRaisingTriggerHead2->Checked) {
			triggerMode = TriggerRaising;
		} else if (rbFallingTriggerHead2->Checked) {
			triggerMode = TriggerFalling;
		} else if (rzDCModeHead2->Checked) {
			triggerMode = TriggerDC;
		}


		// limit the amplifier to 1 in DC mode
		int theAmplifier = triggerMode != TriggerDC ? grHead2Amplifier->ItemIndex + 1 : 1;

		TCommandErrorOutput retCode = m_LD130.setBankHeadData(
															 bankIndex,
															 2,								    // 1 - head 1, 2 - Head 2
															 int(edVoltageHead2->IntValue),	    // 0 - 100 Volts
															 int(edChanel1Head2->Value*100.f),  // 0 - 100 00% with fixed decimal point at 2 digits
															 int(edChanel2Head2->Value*100.f),  // for example the power of 35.23% will be sent as 3523
															 int(edChanel3Head2->Value*100.f),  // the power of 99.00% will be sent as 9900
															 int(edChanel4Head2->Value*100.f),  // the power of 100.00% will be sent as 10000
															 int(edDelayHead2->IntValue),	    // the delay of outcoming light strobe in microseconds
															 int(edWidthHead2->IntValue),	    // the duration of outcoming light strobe in microseconds
															 triggerMode, 						// the edge of incoming trigger to start counting, 0 - raising, 1 - falling
															 3,	 								// the ID of the trigger this output head will trigger on. 1 - Trigger 1, 2 - Trigger 2, 3 - Trigger 1 or 2
															 theAmplifier					 	// the amplification value 1-5
															 );

		if (retCode.hasError()) {
			lbLog->Items->Add(AnsiString((char*)retCode.m_errorDescription));
            lbFullLog->Items->Add(AnsiString((char*)retCode.m_errorDescription));
		}
		bUpdated2 = true;
	}

	if (m_updateActiveBank) {
		m_updateActiveBank = false;
		m_LD130.setActiveBank(RzTabBanks->TabIndex+1);
		bUpdated1 = true;
		bUpdated2 = true;
	}

	if (bUpdated1) {
		m_LD130.getHeadStatus(1);
	}
	if (bUpdated2) {
		m_LD130.getHeadStatus(2);
	}
//	m_LD130.getStatus();
}
//---------------------------------------------------------------------------

void __fastcall TFormMainWindow::actTriggerHead1Execute(TObject *Sender)
{
	m_headControlInfo.m_updateHead[0] = true;
}
//---------------------------------------------------------------------------

void __fastcall TFormMainWindow::actTriggerHead1Update(TObject *Sender)
{
	actTriggerHead1->Enabled = !cbAutoTrigger->Checked;
}
//---------------------------------------------------------------------------

void __fastcall TFormMainWindow::actTriggerHead2Execute(TObject *Sender)
{
	m_headControlInfo.m_updateHead[1] = true;
}
//---------------------------------------------------------------------------

void __fastcall TFormMainWindow::actTriggerHead2Update(TObject *Sender)
{
	actTriggerHead2->Enabled = !cbAutoTrigger->Checked;
}

//---------------------------------------------------------------------------

void __fastcall TFormMainWindow::TimerAutoTriggerHead1Timer(TObject *Sender)
{
	actTriggerHead1Execute(Sender);
}
//---------------------------------------------------------------------------

void __fastcall TFormMainWindow::cbAutoTriggerClick(TObject *Sender)
{
	if (cbAutoTrigger->Checked) {
		RzBtnSoftTrigger->GroupIndex = 1;
	} else {
		RzBtnSoftTrigger->GroupIndex = 0;
		RzBtnSoftTrigger->Caption = "Trigger";
	}

}
//---------------------------------------------------------------------------

void __fastcall TFormMainWindow::cbComPortCloseUp(TObject *Sender)
{
	RzStatusOperation->Caption="Opening COM Port...";
	RzFieldFirmwareVersion->Caption = "";
	RzFieldActiveBank->Caption = "";

	char str2[64] = {0};
	_snprintf (str2, sizeof(str2), "COM%d", cbComPort->ItemIndex);
	int speed = RzRegIniFile->ReadInteger(str2, "Speed", 9600);

	TCommandErrorOutput retCode = m_LD130.init(cbComPort->ItemIndex, speed, &OnLogEvent);

	if (retCode.hasError()) {
		RzFieldFirmwareVersion->Caption = "ERROR";
	} else {
		string verMajor, verMinor,verBuild;
		m_LD130.getVersion(verMajor, verMinor,verBuild);

		RzFieldFirmwareVersion->Caption = AnsiString(verMajor.c_str()) +"." + AnsiString(verMinor.c_str()) + "." + AnsiString(verBuild.c_str());
		initControlsFromController();
	}
	RzStatusOperation->Caption="";
}
//---------------------------------------------------------------------------

void __fastcall TFormMainWindow::rbRaisingTriggerHead1Click(TObject *Sender)
{
	m_headControlInfo.m_updateHead[0] = true;
    grHead1Amplifier->Enabled = true;
}
//---------------------------------------------------------------------------

void __fastcall TFormMainWindow::rbFallingTriggerHead1Click(TObject *Sender)
{
	m_headControlInfo.m_updateHead[0] = true;
    grHead1Amplifier->Enabled = true;
}
//---------------------------------------------------------------------------

void __fastcall TFormMainWindow::rzDCModeHead1Click(TObject *Sender)
{
    m_headControlInfo.m_updateHead[0] = true;
    grHead1Amplifier->ItemIndex = 0;
    grHead1Amplifier->Enabled = false;
}
//---------------------------------------------------------------------------

void __fastcall TFormMainWindow::TabSheet2Show(TObject *Sender)
{
	RzBankSequence->Enabled = rzchkEnableBanks->Checked;
	RzbrUseTrigger1->Enabled = rzchkEnableBanks->Checked;
	RzrbUseNextSequenceInput->Enabled = rzchkEnableBanks->Checked;
}
//---------------------------------------------------------------------------

void __fastcall TFormMainWindow::rzchkEnableBanksClick(TObject *Sender)
{
	RzBankSequence->Enabled = rzchkEnableBanks->Checked;
	RzbrUseTrigger1->Enabled = rzchkEnableBanks->Checked;
	RzrbUseNextSequenceInput->Enabled = rzchkEnableBanks->Checked;

	RzTabBanks->Tabs->Items[1]->Visible = rzchkEnableBanks->Checked;
	RzTabBanks->Tabs->Items[2]->Visible = rzchkEnableBanks->Checked;
	RzTabBanks->Tabs->Items[3]->Visible = rzchkEnableBanks->Checked;
}
//---------------------------------------------------------------------------

void __fastcall TFormMainWindow::TabSheet1Show(TObject *Sender)
{
	RzTabBanks->Tabs->Items[1]->Visible = rzchkEnableBanks->Checked;
	RzTabBanks->Tabs->Items[2]->Visible = rzchkEnableBanks->Checked;
	RzTabBanks->Tabs->Items[3]->Visible = rzchkEnableBanks->Checked;

	rbRaisingTriggerHead2->Enabled = !rzchkEnableBanks->Checked;
	rbFallingTriggerHead2->Enabled = !rzchkEnableBanks->Checked;
	rzDCModeHead2->Enabled = !rzchkEnableBanks->Checked;
}
//---------------------------------------------------------------------------

void __fastcall TFormMainWindow::rbRaisingTriggerHead2Click(TObject *Sender)
{
	m_headControlInfo.m_updateHead[1] = true;
    grHead2Amplifier->Enabled = true;
}
//---------------------------------------------------------------------------

void __fastcall TFormMainWindow::rbFallingTriggerHead2Click(TObject *Sender)
{
	m_headControlInfo.m_updateHead[1] = true;
    grHead2Amplifier->Enabled = true;
}
//---------------------------------------------------------------------------

void TFormMainWindow::initControlsFromController()
{
	updateControlsForSelectedBank();
}

//---------------------------------------------------------------------------

void TFormMainWindow::updateControlsForSelectedBank()
{
	//------------------------------------------------
	// Head 1
	//------------------------------------------------

	TBankHeadData theHeadData = m_LD130.getBankHeadData(RzTabBanks->TabIndex+1, 1);

	setTrackBarSafely(tbVoltageHead1, theHeadData.m_voltage);

	setPowerSafely(tbChanel1Head1, edChanel1Head1, theHeadData.m_powerChanel1/100.f);	// 0 - 100 00% with fixed decimal point at 2 digits
	setPowerSafely(tbChanel2Head1, edChanel2Head1, theHeadData.m_powerChanel2/100.f);	// for example the power of 35.23% will be sent as 3523
	setPowerSafely(tbChanel3Head1, edChanel3Head1, theHeadData.m_powerChanel3/100.f);	// the power of 99.00% will be sent as 9900
	setPowerSafely(tbChanel4Head1, edChanel4Head1, theHeadData.m_powerChanel4/100.f);	// the power of 100.00% will be sent as 10000

	setNumericEditSafely(edDelayHead1, theHeadData.m_strobeDelay);
	setNumericEditSafely(edWidthHead1, theHeadData.m_strobeWidth);
	switch (theHeadData.m_triggerEdge) {
	case TriggerRaising:
		rbRaisingTriggerHead1->Checked = true;
		rbFallingTriggerHead1->Checked = false;
		rzDCModeHead1->Checked = false;
		break;

	case TriggerFalling:
		rbRaisingTriggerHead1->Checked = false;
		rbFallingTriggerHead1->Checked = true;
		rzDCModeHead1->Checked = false;
		break;

	case TriggerDC:
		rbFallingTriggerHead1->Checked = false;
		rbRaisingTriggerHead1->Checked = false;
		rzDCModeHead1->Checked = true;
		break;

	}

	switch (theHeadData.m_triggerId) {
	case 1:
		rgTriggerSourceHead1->ItemIndex = 1;
		break;

	case 2:
		rgTriggerSourceHead1->ItemIndex = 2;
		break;

	case 3:
		rgTriggerSourceHead1->ItemIndex = 3;
		break;
	}

	if (theHeadData.m_chanelAmplifier > 0 && theHeadData.m_chanelAmplifier <= 5)
		grHead1Amplifier->ItemIndex  = theHeadData.m_chanelAmplifier - 1;

	THeadStatus theHeadStatus = m_LD130.getHeadStatus(1);

	pnlOverCurrentHead1Chanel1->Color = ((m_LD130.getActiveBank() -1) == RzTabBanks->TabIndex) ? (theHeadStatus.m_statusChanel1 ? clRed : clGreen) : clBackground;
	pnlOverCurrentHead1Chanel2->Color = ((m_LD130.getActiveBank() -1) == RzTabBanks->TabIndex) ? (theHeadStatus.m_statusChanel2 ? clRed : clGreen) : clBackground;
	pnlOverCurrentHead1Chanel3->Color = ((m_LD130.getActiveBank() -1) == RzTabBanks->TabIndex) ? (theHeadStatus.m_statusChanel3 ? clRed : clGreen) : clBackground;
	pnlOverCurrentHead1Chanel4->Color = ((m_LD130.getActiveBank() -1) == RzTabBanks->TabIndex) ? (theHeadStatus.m_statusChanel4 ? clRed : clGreen) : clBackground;

	//------------------------------------------------
	// Head 2
	//------------------------------------------------

	theHeadData = m_LD130.getBankHeadData(RzTabBanks->TabIndex+1, 2);
	setTrackBarSafely(tbVoltageHead2, theHeadData.m_voltage);

	setPowerSafely(tbChanel1Head2, edChanel1Head2, theHeadData.m_powerChanel1/100.f);	// 0 - 100 00% with fixed decimal point at 2 digits
	setPowerSafely(tbChanel2Head2, edChanel2Head2, theHeadData.m_powerChanel2/100.f);	// for example the power of 35.23% will be sent as 3523
	setPowerSafely(tbChanel3Head2, edChanel3Head2, theHeadData.m_powerChanel3/100.f);	// the power of 99.00% will be sent as 9900
	setPowerSafely(tbChanel4Head2, edChanel4Head2, theHeadData.m_powerChanel4/100.f);	// the power of 100.00% will be sent as 10000

	setNumericEditSafely(edDelayHead2, theHeadData.m_strobeDelay);
	setNumericEditSafely(edWidthHead2, theHeadData.m_strobeWidth);
	switch (theHeadData.m_triggerEdge) {
	case TriggerRaising:
		rbRaisingTriggerHead2->Checked = true;
		rbFallingTriggerHead2->Checked = false;
		rzDCModeHead2->Checked = false;
		break;

	case TriggerFalling:
		rbRaisingTriggerHead2->Checked = false;
		rbFallingTriggerHead2->Checked = true;
		rzDCModeHead2->Checked = false;
		break;

	case TriggerDC:
		rbFallingTriggerHead2->Checked = false;
		rbRaisingTriggerHead2->Checked = false;
		rzDCModeHead2->Checked = true;
		break;


	}

	switch (theHeadData.m_triggerId) {
	case 1:
		rgTriggerSourceHead2->ItemIndex = 1;
		break;

	case 2:
		rgTriggerSourceHead2->ItemIndex = 2;
		break;

	case 3:
		rgTriggerSourceHead2->ItemIndex = 3;
		break;
	}

	if (theHeadData.m_chanelAmplifier > 0 && theHeadData.m_chanelAmplifier <= 5)
		grHead2Amplifier->ItemIndex  = theHeadData.m_chanelAmplifier - 1;

	theHeadStatus = m_LD130.getHeadStatus(2);

	pnlOverCurrentHead2Chanel1->Color = ((m_LD130.getActiveBank() -1) == RzTabBanks->TabIndex) ? (theHeadStatus.m_statusChanel1 ? clRed : clGreen) : clBackground;
	pnlOverCurrentHead2Chanel2->Color = ((m_LD130.getActiveBank() -1) == RzTabBanks->TabIndex) ? (theHeadStatus.m_statusChanel2 ? clRed : clGreen) : clBackground;
	pnlOverCurrentHead2Chanel3->Color = ((m_LD130.getActiveBank() -1) == RzTabBanks->TabIndex) ? (theHeadStatus.m_statusChanel3 ? clRed : clGreen) : clBackground;
	pnlOverCurrentHead2Chanel4->Color = ((m_LD130.getActiveBank() -1) == RzTabBanks->TabIndex) ? (theHeadStatus.m_statusChanel4 ? clRed : clGreen) : clBackground;

	m_headControlInfo.m_updateHead[0] = false;
	m_headControlInfo.m_updateHead[1] = false;

}

//---------------------------------------------------------------------------
void TFormMainWindow::setTrackBarSafely(TRzTrackBar * pTrackBar, int newValue)
{
	if (!pTrackBar)
		return ;

	if (newValue < pTrackBar->Min)
		newValue = pTrackBar->Min;

	if (newValue > pTrackBar->Max)
		newValue = pTrackBar->Max;
	pTrackBar->Position = newValue;
}

//---------------------------------------------------------------------------
void TFormMainWindow::setNumericEditSafely(TRzNumericEdit* pEdit, int newValue)
{
	if (!pEdit)
		return ;

	if (newValue < pEdit->Min)
		newValue = pEdit->Min;

	if (newValue > pEdit->Max)
		newValue = pEdit->Max;

	pEdit->Value = newValue;
}

//---------------------------------------------------------------------------
void TFormMainWindow::setPowerSafely(TRzTrackBar * pTrackBar, TRzNumericEdit* pEdit, float newValue)
{
	if (!pTrackBar || !pEdit)
		return ;

	if (newValue < pTrackBar->Min)
		newValue = pTrackBar->Min;

	if (newValue > pTrackBar->Max)
		newValue = pTrackBar->Max;
	//	pTrackBar->Position = newValue;
	pEdit->Value = newValue;
}

void __fastcall TFormMainWindow::grHead1AmplifierChanging(TObject *Sender, int NewIndex, bool &AllowChange)
{
	m_headControlInfo.m_updateHead[0] = true;
}
//---------------------------------------------------------------------------

void __fastcall TFormMainWindow::FormShow(TObject *Sender)
{
	RzPanelVersion->Caption = RzPanelVersion->Caption + RzVersionInfo->FileVersion;
	RzPageControlParameters->ActivePage = TabSheet1;
}
//---------------------------------------------------------------------------

void __fastcall TFormMainWindow::TimerAutoTriggerTimer(TObject *Sender)
{
	m_LD130.softTrig();
}
//---------------------------------------------------------------------------

void __fastcall TFormMainWindow::RzBtnSoftTriggerClick(TObject *Sender)
{
	if (RzBtnSoftTrigger->GroupIndex == 0) {
		m_LD130.softTrig();
	} else {
		if (RzBtnSoftTrigger->Down) {
			TimerAutoTrigger->Interval = edAutoTriggerPeriod->IntValue;
			TimerAutoTrigger->Enabled = true;
			RzBtnSoftTrigger->Caption = "Trigger ON";
		} else {
			TimerAutoTrigger->Enabled = false;
			RzBtnSoftTrigger->Caption = "Trigger OFF";
		}
	}
}
//---------------------------------------------------------------------------

void __fastcall TFormMainWindow::RzTabBanksChange(TObject *Sender)
{
	updateControlsForSelectedBank();
	actActivateBank->Update();
}
//---------------------------------------------------------------------------

void __fastcall TFormMainWindow::rzbtnSendSequenceClick(TObject *Sender)
{
	string theValue;
	for (int iStr = 1; iStr <= RzBankSequence->Text.Length(); ++iStr) {
		if (isdigit(RzBankSequence->Text[iStr])) {
			theValue += RzBankSequence->Text[iStr];
		}
	}

	m_LD130.setSequence(theValue);
}
//---------------------------------------------------------------------------

void __fastcall TFormMainWindow::btnComConfigClick(TObject *Sender)
{
//  FormComPortConfig->m_comPort = cbComPort->ItemIndex;
//  if (FormComPortConfig->ShowModal() == mrOk && FormComPortConfig->m_newSpeed) {
//
//  	TCmdSetPortSpeed cmd;
//  	cmd.m_UartID = m_UartID;
//  	cmd.m_baudRate = FormComPortConfig->m_newSpeed;
//  	TCmdPortSpeed cmdReply;
//  	TCommandErrorOutput errCode = sendCommand(&cmd, sizeof(cmd), &cmdReply, sizeof(cmdReply));
//  	if (errCode.m_command == 0) {
//  		DCB dcb;
//  		if (GetCommState(m_hComPort, &dcb)) {
//  			dcb.BaudRate = FormComPortConfig->m_newSpeed;     	// set the baud rate
//  			if (!SetCommState(m_hComPort, &dcb)) {
//  				; // log the error
//  			}
//  		}
//
//  	}
//  }
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void __fastcall TFormMainWindow::TimerUpdateGUITimer(TObject *Sender)
{
	if (m_LD130.isConnected()) {
		unsigned short activeBank = m_LD130.getActiveBank();
		RzFieldActiveBank->Caption = "Active Bank:" + AnsiString(activeBank);
	}
}
//---------------------------------------------------------------------------

void __fastcall TFormMainWindow::actStoreToEPROMExecute(TObject *Sender)
{
//  TCmdWriteParametersToEPROM cmd;
//  TCmdEPROMOperationDone cmdReply;
//
//  cmd.m_flags = ((rzchkEnableBanks->Checked) ? fifUseBanks : 0);
//  TCommandErrorOutput errCode = sendCommand(&cmd, sizeof(cmd), &cmdReply, sizeof(cmdReply));
}
//---------------------------------------------------------------------------

void __fastcall TFormMainWindow::actStoreToEPROMUpdate(TObject *Sender)
{
//q
}
//---------------------------------------------------------------------------

void __fastcall TFormMainWindow::actLoadFromEPROMExecute(TObject *Sender)
{
//  TCmdLoadParametersFromEPROM cmd;
//  TCmdEPROMOperationDone cmdReply;
//
//  TCommandErrorOutput errCode = sendCommand(&cmd, sizeof(cmd), &cmdReply, sizeof(cmdReply));
//  if (errCode.m_command == 0) {
//  	initControlsFromController();
//  }
}
//---------------------------------------------------------------------------

void __fastcall TFormMainWindow::actLoadFromEPROMUpdate(TObject *Sender)
{
//q
}
//---------------------------------------------------------------------------

void __fastcall TFormMainWindow::cbComPortDropDown(TObject *Sender)
{
	RzStatusOperation->Caption = "";
	RzFieldFirmwareVersion->Caption = "";
	RzFieldActiveBank->Caption = "";
}
//---------------------------------------------------------------------------


void __fastcall TFormMainWindow::actActivateBankExecute(TObject *Sender)
{
//	m_activeBank = RzTabBanks->TabIndex;
	m_updateActiveBank = true;
	//updateControlsForSelectedBank();
}
//---------------------------------------------------------------------------

void __fastcall TFormMainWindow::actActivateBankUpdate(TObject *Sender)
{
//	actActivateBank->Visible = (RzTabBanks->TabIndex != m_activeBank);
}
//---------------------------------------------------------------------------

void TFormMainWindow::OnLogEvent(const char* pCmd, const char* pLogStr)
{
	while (FormMainWindow->lbLog->Items->Count > 500) {
		FormMainWindow->lbLog->Items->Delete(0);
	}

	int idx = FormMainWindow->lbLog->Items->Add(AnsiString(pLogStr));
	if (idx > 3)
		FormMainWindow->lbLog->TopIndex = idx - 1;




	while (FormMainWindow->lbFullLog->Items->Count > 1000) {
		FormMainWindow->lbFullLog->Items->Delete(0);
	}

	idx = FormMainWindow->lbFullLog->Items->Add(AnsiString(pLogStr));
//	if (idx > 3)
//		FormMainWindow->lbFullLog->TopIndex = idx - 1;
}
//---------------------------------------------------------------------------

void __fastcall TFormMainWindow::rgTriggerSourceHead1Click(TObject *Sender)
{
    m_headControlInfo.m_updateHead[0] = true;
}
//---------------------------------------------------------------------------

void __fastcall TFormMainWindow::rgTriggerSourceHead1Changing(TObject *Sender, int NewIndex, bool &AllowChange)
{
    m_headControlInfo.m_updateHead[0] = true;
}
//---------------------------------------------------------------------------

void __fastcall TFormMainWindow::rgTriggerSourceHead2Changing(
      TObject *Sender, int NewIndex, bool &AllowChange)
{
    m_headControlInfo.m_updateHead[1] = true;
}
//---------------------------------------------------------------------------

void __fastcall TFormMainWindow::grHead2AmplifierChanging(TObject *Sender,
      int NewIndex, bool &AllowChange)
{
    m_headControlInfo.m_updateHead[1] = true;
}
//---------------------------------------------------------------------------

void __fastcall TFormMainWindow::rzDCModeHead2Click(TObject *Sender)
{
    m_headControlInfo.m_updateHead[1] = true;
    grHead2Amplifier->ItemIndex = 0;
    grHead2Amplifier->Enabled = false;
}
//---------------------------------------------------------------------------

void __fastcall TFormMainWindow::tbChanel1Head1Changing(TObject *Sender, int NewPos, bool &AllowChange)
{
    if (m_head1PowerChanging)
        return;

    m_head1PowerChanging = true;

    int diff = NewPos - tbChanel1Head1->Position;
    if (grHead1Lock->ItemChecked[0] && grHead1Lock->ItemChecked[1]) {
        tbChanel2Head1->Position = tbChanel2Head1->Position + diff;
    }
    if (grHead1Lock->ItemChecked[0] && grHead1Lock->ItemChecked[2]) {
        tbChanel3Head1->Position = tbChanel3Head1->Position + diff;
    }
    if (grHead1Lock->ItemChecked[0] && grHead1Lock->ItemChecked[3]) {
        tbChanel4Head1->Position = tbChanel4Head1->Position + diff;
    }

    m_head1PowerChanging = false;
}
//---------------------------------------------------------------------------

void __fastcall TFormMainWindow::tbChanel2Head1Changing(TObject *Sender,
      int NewPos, bool &AllowChange)
{
    if (m_head1PowerChanging)
        return;

    m_head1PowerChanging = true;

    int diff = NewPos - tbChanel2Head1->Position;
    if (grHead1Lock->ItemChecked[1] && grHead1Lock->ItemChecked[0]) {
        tbChanel1Head1->Position = tbChanel1Head1->Position + diff;
    }
    if (grHead1Lock->ItemChecked[1] && grHead1Lock->ItemChecked[2]) {
        tbChanel3Head1->Position = tbChanel3Head1->Position + diff;
    }
    if (grHead1Lock->ItemChecked[1] && grHead1Lock->ItemChecked[3]) {
        tbChanel4Head1->Position = tbChanel4Head1->Position + diff;
    }

    m_head1PowerChanging = false;
}
//---------------------------------------------------------------------------

void __fastcall TFormMainWindow::tbChanel3Head1Changing(TObject *Sender,
      int NewPos, bool &AllowChange)
{
    if (m_head1PowerChanging)
        return;

    m_head1PowerChanging = true;

    int diff = NewPos - tbChanel3Head1->Position;
    if (grHead1Lock->ItemChecked[2] && grHead1Lock->ItemChecked[1]) {
        tbChanel2Head1->Position = tbChanel2Head1->Position + diff;
    }
    if (grHead1Lock->ItemChecked[2] && grHead1Lock->ItemChecked[0]) {
        tbChanel1Head1->Position = tbChanel1Head1->Position + diff;
    }
    if (grHead1Lock->ItemChecked[2] && grHead1Lock->ItemChecked[3]) {
        tbChanel4Head1->Position = tbChanel4Head1->Position + diff;
    }

    m_head1PowerChanging = false;
}
//---------------------------------------------------------------------------

void __fastcall TFormMainWindow::tbChanel4Head1Changing(TObject *Sender,
      int NewPos, bool &AllowChange)
{
    if (m_head1PowerChanging)
        return;

    m_head1PowerChanging = true;

    int diff = NewPos - tbChanel4Head1->Position;
    if (grHead1Lock->ItemChecked[3] && grHead1Lock->ItemChecked[1]) {
        tbChanel2Head1->Position = tbChanel2Head1->Position + diff;
    }
    if (grHead1Lock->ItemChecked[3] && grHead1Lock->ItemChecked[2]) {
        tbChanel3Head1->Position = tbChanel3Head1->Position + diff;
    }
    if (grHead1Lock->ItemChecked[3] && grHead1Lock->ItemChecked[0]) {
        tbChanel1Head1->Position = tbChanel1Head1->Position + diff;
    }

    m_head1PowerChanging = false;

}
//---------------------------------------------------------------------------

void __fastcall TFormMainWindow::tbChanel1Head2Changing(TObject *Sender,
      int NewPos, bool &AllowChange)
{
    if (m_head2PowerChanging)
        return;

    m_head2PowerChanging = true;

    int diff = NewPos - tbChanel1Head2->Position;
    if (grHead2Lock->ItemChecked[0] && grHead2Lock->ItemChecked[1]) {
        tbChanel2Head2->Position = tbChanel2Head2->Position + diff;
    }
    if (grHead2Lock->ItemChecked[0] && grHead2Lock->ItemChecked[2]) {
        tbChanel3Head2->Position = tbChanel3Head2->Position + diff;
    }
    if (grHead2Lock->ItemChecked[0] && grHead2Lock->ItemChecked[3]) {
        tbChanel4Head2->Position = tbChanel4Head2->Position + diff;
    }

    m_head2PowerChanging = false;

}
//---------------------------------------------------------------------------

void __fastcall TFormMainWindow::tbChanel2Head2Changing(TObject *Sender,
      int NewPos, bool &AllowChange)
{
    if (m_head2PowerChanging)
        return;

    m_head2PowerChanging = true;

    int diff = NewPos - tbChanel2Head2->Position;
    if (grHead2Lock->ItemChecked[0] && grHead2Lock->ItemChecked[0]) {
        tbChanel1Head2->Position = tbChanel1Head2->Position + diff;
    }
    if (grHead2Lock->ItemChecked[0] && grHead2Lock->ItemChecked[2]) {
        tbChanel3Head2->Position = tbChanel3Head2->Position + diff;
    }
    if (grHead2Lock->ItemChecked[0] && grHead2Lock->ItemChecked[3]) {
        tbChanel4Head2->Position = tbChanel4Head2->Position + diff;
    }

    m_head2PowerChanging = false;

}
//---------------------------------------------------------------------------

void __fastcall TFormMainWindow::tbChanel3Head2Changing(TObject *Sender,
      int NewPos, bool &AllowChange)
{
    if (m_head2PowerChanging)
        return;

    m_head2PowerChanging = true;

    int diff = NewPos - tbChanel3Head2->Position;
    if (grHead2Lock->ItemChecked[0] && grHead2Lock->ItemChecked[1]) {
        tbChanel2Head2->Position = tbChanel2Head2->Position + diff;
    }
    if (grHead2Lock->ItemChecked[0] && grHead2Lock->ItemChecked[0]) {
        tbChanel1Head2->Position = tbChanel1Head2->Position + diff;
    }
    if (grHead2Lock->ItemChecked[0] && grHead2Lock->ItemChecked[3]) {
        tbChanel4Head2->Position = tbChanel4Head2->Position + diff;
    }

    m_head2PowerChanging = false;

}
//---------------------------------------------------------------------------

void __fastcall TFormMainWindow::tbChanel4Head2Changing(TObject *Sender,
      int NewPos, bool &AllowChange)
{
    if (m_head2PowerChanging)
        return;

    m_head2PowerChanging = true;

    int diff = NewPos - tbChanel4Head2->Position;
    if (grHead2Lock->ItemChecked[0] && grHead2Lock->ItemChecked[1]) {
        tbChanel2Head2->Position = tbChanel2Head2->Position + diff;
    }
    if (grHead2Lock->ItemChecked[0] && grHead2Lock->ItemChecked[2]) {
        tbChanel3Head2->Position = tbChanel3Head2->Position + diff;
    }
    if (grHead2Lock->ItemChecked[0] && grHead2Lock->ItemChecked[0]) {
        tbChanel1Head2->Position = tbChanel1Head2->Position + diff;
    }

    m_head2PowerChanging = false;

}
//---------------------------------------------------------------------------

