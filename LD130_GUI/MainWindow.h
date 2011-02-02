////////////////////////////////////////////////////////////////////
//
// 	Alexandr Filenkov 2010
//
////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------

#ifndef MainWindowH
#define MainWindowH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include "RzPanel.hpp"
#include <ExtCtrls.hpp>
#include "RzSplit.hpp"
#include "RzTrkBar.hpp"
#include "RzEdit.hpp"
#include <Mask.hpp>
#include "RzButton.hpp"
#include "RzLine.hpp"
#include "RzRadChk.hpp"
#include "RzLabel.hpp"
#include "RzCmboBx.hpp"
#include "RzLstBox.hpp"
#include <ActnList.hpp>
#include "RzTabs.hpp"
#include "RzRadGrp.hpp"
#include "RzStatus.hpp"
//---------------------------------------------------------------------------


#include "cxContainer.hpp"
#include "cxControls.hpp"
#include "cxEdit.hpp"
#include "cxMaskEdit.hpp"
#include "cxTextEdit.hpp"
#include "RzCommon.hpp"
#include "RzSpnEdt.hpp"
#include <Buttons.hpp>

#include <map>
#include <list>
#include "LD130Impl.h"


//---------------------------------------------------------------------------
struct THeadControlGroup
{
	THeadControlGroup (int headIndex, TRzNumericEdit* pEdit, TRzTrackBar* pTrackBar)
		: m_headIndex (headIndex)
		, m_pEdit (pEdit)
		, m_pTrackBar (pTrackBar)
	{}

	int m_headIndex; 		//0 - head 1; 1 - head 2
	TRzNumericEdit* m_pEdit;
	TRzTrackBar* m_pTrackBar;
};
//---------------------------------------------------------------------------
struct THeadControlInfo
{
	void addControlGroup (int headIndex, TRzNumericEdit* pEdit, TRzTrackBar* pTrackBar)
	{
		m_powerChangingHead[headIndex] = false;
		m_updateHead[headIndex] = false;

		if (!pEdit)
			assert (!"Invalid numeric edit box pointer for the head control group");

		m_headControlGroupList.push_back (THeadControlGroup(headIndex, pEdit, pTrackBar));
	}
	THeadControlGroup* findHeadControlGroupByEdit (TRzNumericEdit* pEdit)
	{
		if (!pEdit)
			return 0;
		std::list <THeadControlGroup>::iterator it;
		for (it = m_headControlGroupList.begin(); it != m_headControlGroupList.end(); it++) {
			if (it->m_pEdit == pEdit)
				return &(*it);
		}
		return 0;
	}
	THeadControlGroup* findHeadControlGroupByTrackBar (TRzTrackBar* pTrackBar)
	{
		if (!pTrackBar)
			return 0;
		std::list <THeadControlGroup>::iterator it;
		for (it = m_headControlGroupList.begin(); it != m_headControlGroupList.end(); it++) {
			if (it->m_pTrackBar == pTrackBar)
				return &(*it);
		}
		return 0;
	}

	std::map<int, bool> m_powerChangingHead;
	std::map<int, bool> m_updateHead;
	std::list <THeadControlGroup> m_headControlGroupList;
};

//---------------------------------------------------------------------------
class TFormMainWindow : public TForm
{
__published:	// IDE-managed Components
	TRzPanel *RzPanel1;
	TRzStatusBar *StatusBar;
	TRzPanel *RzPanel3;
	TRzComboBox *cbComPort;
	TRzLabel *RzLabel3;
	TRzPanel *RzPanelVersion;
	TTimer *TimerUpdate;
	TActionList *ActionList;
	TAction *actTriggerHead1;
	TRzPageControl *RzPageControlParameters;
	TRzTabSheet *TabSheet1;
	TRzTabControl *RzTabBanks;
	TRzSizePanel *RzSizePanel1;
	TRzGroupBox *RzGroupBox1;
	TRzTrackBar *tbVoltageHead1;
	TRzNumericEdit *edVoltageHead1;
	TRzGroupBox *RzGroupBox2;
	TRzTrackBar *tbChanel1Head1;
	TRzTrackBar *tbChanel2Head1;
	TRzTrackBar *tbChanel3Head1;
	TRzTrackBar *tbChanel4Head1;
	TRzNumericEdit *edChanel1Head1;
	TRzNumericEdit *edChanel2Head1;
	TRzNumericEdit *edChanel3Head1;
	TRzNumericEdit *edChanel4Head1;
	TRzPanel *pnlOverCurrentHead1Chanel1;
	TRzPanel *pnlOverCurrentHead1Chanel2;
	TRzPanel *pnlOverCurrentHead1Chanel3;
	TRzPanel *pnlOverCurrentHead1Chanel4;
	TRzRadioGroup *grHead1Amplifier;
	TPanel *Panel1;
	TRzPanel *RzPanel8;
	TRzGroupBox *RzGroupBox6;
	TRzLabel *RzLabel1;
	TRzLabel *RzLabel2;
	TRzNumericEdit *edDelayHead1;
	TRzNumericEdit *edWidthHead1;
	TRzGroupBox *RzGroupBox3;
	TBevel *Bevel1;
	TBevel *Bevel2;
	TRzPanel *RzPanel7;
	TBevel *Bevel3;
	TBevel *Bevel4;
	TBevel *Bevel5;
	TBevel *Bevel6;
	TRzGroupBox *RzGroupBox5;
	TRzLine *RzLine1;
	TRzLine *RzLine3;
	TRzLine *RzLine4;
	TRzLine *RzLine2;
	TRzLine *RzLine5;
	TRzLine *RzLine6;
	TRzLine *RzLine7;
	TRzLine *RzLine8;
	TRzLine *RzLine9;
	TRzLine *RzLine10;
	TRzRadioButton *rbRaisingTriggerHead1;
	TRzRadioButton *rbFallingTriggerHead1;
	TRzRadioButton *rzDCModeHead1;
	TRzPanel *RzPanel6;
	TRzRadioGroup *rgTriggerSourceHead1;
	TRzSizePanel *RzSizePanel2;
	TPanel *Panel7;
	TRzTabSheet *TabSheet2;
	TRzGroupBox *RzGroupBox7;
	TRzTrackBar *tbVoltageHead2;
	TRzNumericEdit *edVoltageHead2;
	TRzGroupBox *RzGroupBox8;
	TRzTrackBar *tbChanel1Head2;
	TRzTrackBar *tbChanel2Head2;
	TRzTrackBar *tbChanel3Head2;
	TRzTrackBar *tbChanel4Head2;
	TRzNumericEdit *edChanel1Head2;
	TRzNumericEdit *edChanel2Head2;
	TRzNumericEdit *edChanel3Head2;
	TRzNumericEdit *edChanel4Head2;
	TRzPanel *pnlOverCurrentHead2Chanel1;
	TRzPanel *pnlOverCurrentHead2Chanel2;
	TRzPanel *pnlOverCurrentHead2Chanel3;
	TRzPanel *pnlOverCurrentHead2Chanel4;
	TRzRadioGroup *grHead2Amplifier;
	TRzPanel *RzPanel11;
	TRzGroupBox *RzGroupBox9;
	TRzLabel *RzLabel5;
	TRzLabel *RzLabel6;
	TRzNumericEdit *edDelayHead2;
	TRzNumericEdit *edWidthHead2;
	TRzGroupBox *RzGroupBox10;
	TBevel *Bevel7;
	TBevel *Bevel8;
	TRzPanel *RzPanel12;
	TBevel *Bevel9;
	TBevel *Bevel10;
	TBevel *Bevel11;
	TBevel *Bevel12;
	TRzGroupBox *RzGroupBox11;
	TRzLine *RzLine11;
	TRzLine *RzLine12;
	TRzLine *RzLine13;
	TRzLine *RzLine14;
	TRzLine *RzLine15;
	TRzLine *RzLine16;
	TRzLine *RzLine17;
	TRzLine *RzLine18;
	TRzLine *RzLine19;
	TRzLine *RzLine20;
	TRzRadioButton *rbRaisingTriggerHead2;
	TRzRadioButton *rbFallingTriggerHead2;
	TRzRadioButton *rzDCModeHead2;
	TRzPanel *RzPanel13;
	TRzRadioGroup *rgTriggerSourceHead2;
	TRzGroupBox *RzGroupBox4;
	TRzLabel *RzLabel4;
	TRzBitBtn *RzBtnSoftTrigger;
	TRzNumericEdit *edAutoTriggerPeriod;
	TRzCheckBox *cbAutoTrigger;
	TBevel *Bevel13;
	TRzListBox *lbLog;
	TRzGroupBox *RzGroupBox12;
	TRzBitBtn *RzBitBtn2;
	TRzBitBtn *RzBitBtn3;
	TRzBitBtn *RzBitBtn4;
	TRzGroupBox *RzGroupBox13;
	TRzLabel *RzLabel7;
	TRzRadioButton *RzbrUseTrigger1;
	TRzRadioButton *RzrbUseNextSequenceInput;
	TRzClockStatus *RzStatusClock;
	TRzFieldStatus *RzFieldFirmwareVersion;
	TRzCheckBox *rzchkEnableBanks;
	TRzVersionInfo *RzVersionInfo;
	TTimer *TimerAutoTrigger;
	TcxMaskEdit *RzBankSequence;
	TRzBitBtn *rzbtnSendSequence;
	TRzRegIniFile *RzRegIniFile;
	TRzBitBtn *btnComConfig;
	TTimer *TimerUpdateGUI;
	TAction *actStoreToEPROM;
	TAction *actLoadFromEPROM;
	TRzStatusPane *RzStatusOperation;
	TRzFieldStatus *RzFieldActiveBank;
	TAction *actTriggerHead2;
	TRzRapidFireButton *btnActivateBank;
	TAction *actActivateBank;
	TRzCheckBox *rzchkAutoActivateBank;
    TRzCheckGroup *grHead1Lock;
    TRzCheckGroup *grHead2Lock;
    TRzTabSheet *tabLog;
    TRzListBox *lbFullLog;
    TRzBitBtn *rzRefresh;
	void __fastcall TimerUpdateTimer(TObject *Sender);
	void __fastcall EditChange(TObject *Sender);
	void __fastcall TrackBarChange(TObject *Sender);
	void __fastcall TimerAutoTriggerHead1Timer(TObject *Sender);
	void __fastcall cbAutoTriggerClick(TObject *Sender);
	void __fastcall cbComPortCloseUp(TObject *Sender);
	void __fastcall rbRaisingTriggerHead1Click(TObject *Sender);
	void __fastcall rbFallingTriggerHead1Click(TObject *Sender);
	void __fastcall rzDCModeHead1Click(TObject *Sender);
	void __fastcall TabSheet2Show(TObject *Sender);
	void __fastcall rzchkEnableBanksClick(TObject *Sender);
	void __fastcall TabSheet1Show(TObject *Sender);
	void __fastcall rbRaisingTriggerHead2Click(TObject *Sender);
	void __fastcall rbFallingTriggerHead2Click(TObject *Sender);
	void __fastcall grHead1AmplifierChanging(TObject *Sender, int NewIndex, bool &AllowChange);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall TimerAutoTriggerTimer(TObject *Sender);
	void __fastcall RzBtnSoftTriggerClick(TObject *Sender);
	void __fastcall RzTabBanksChange(TObject *Sender);
	void __fastcall rzbtnSendSequenceClick(TObject *Sender);
	void __fastcall btnComConfigClick(TObject *Sender);
	void __fastcall TimerUpdateGUITimer(TObject *Sender);
	void __fastcall actStoreToEPROMExecute(TObject *Sender);
	void __fastcall actStoreToEPROMUpdate(TObject *Sender);
	void __fastcall actLoadFromEPROMExecute(TObject *Sender);
	void __fastcall actLoadFromEPROMUpdate(TObject *Sender);
	void __fastcall cbComPortDropDown(TObject *Sender);
	void __fastcall actTriggerHead2Execute(TObject *Sender);
	void __fastcall actTriggerHead2Update(TObject *Sender);
	void __fastcall actTriggerHead1Execute(TObject *Sender);
	void __fastcall actTriggerHead1Update(TObject *Sender);
	void __fastcall actActivateBankExecute(TObject *Sender);
	void __fastcall actActivateBankUpdate(TObject *Sender);
    void __fastcall rgTriggerSourceHead1Click(TObject *Sender);
    void __fastcall rgTriggerSourceHead1Changing(TObject *Sender,
          int NewIndex, bool &AllowChange);
    void __fastcall rgTriggerSourceHead2Changing(TObject *Sender,
          int NewIndex, bool &AllowChange);
    void __fastcall grHead2AmplifierChanging(TObject *Sender, int NewIndex,
          bool &AllowChange);
    void __fastcall rzDCModeHead2Click(TObject *Sender);
    void __fastcall tbChanel1Head1Changing(TObject *Sender, int NewPos,
          bool &AllowChange);
    void __fastcall tbChanel2Head1Changing(TObject *Sender, int NewPos,
          bool &AllowChange);
    void __fastcall tbChanel3Head1Changing(TObject *Sender, int NewPos,
          bool &AllowChange);
    void __fastcall tbChanel4Head1Changing(TObject *Sender, int NewPos,
          bool &AllowChange);
    void __fastcall tbChanel1Head2Changing(TObject *Sender, int NewPos,
          bool &AllowChange);
    void __fastcall tbChanel2Head2Changing(TObject *Sender, int NewPos,
          bool &AllowChange);
    void __fastcall tbChanel3Head2Changing(TObject *Sender, int NewPos,
          bool &AllowChange);
    void __fastcall tbChanel4Head2Changing(TObject *Sender, int NewPos,
          bool &AllowChange);
    void __fastcall rzRefreshClick(TObject *Sender);


private:	// User declarations

	THeadControlInfo m_headControlInfo;
	bool m_updateActiveBank;


	void initControlsFromController();

	void setTrackBarSafely(TRzTrackBar * pTrackBar, int newValue);
	void setNumericEditSafely(TRzNumericEdit* pEdit, int newValue);

	void setPowerSafely(TRzTrackBar * pTrackBar, TRzNumericEdit* pEdit, float newValue);

	void updateControlsForSelectedBank();

	TLD130Impl	m_LD130;

	static void OnLogEvent(const char* pCmd, const char* pLogStr);

    bool m_head1PowerChanging;
    bool m_head2PowerChanging;

public:		// User declarations
		__fastcall TFormMainWindow(TComponent* Owner);
};

//---------------------------------------------------------------------------
extern PACKAGE TFormMainWindow *FormMainWindow;
//---------------------------------------------------------------------------
#endif
