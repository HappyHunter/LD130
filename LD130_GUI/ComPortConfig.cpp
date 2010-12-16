//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "ComPortConfig.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "RzCmboBx"
#pragma link "RzPanel"
#pragma link "RzButton"
#pragma resource "*.dfm"
TFormComPortConfig *FormComPortConfig;
#include "CommonData.h"
#include <stdio.h>

//---------------------------------------------------------------------------
__fastcall TFormComPortConfig::TFormComPortConfig(TComponent* Owner)
	: TForm(Owner)
	, m_comPort(0)
{

}
//---------------------------------------------------------------------------
void __fastcall TFormComPortConfig::FormShow(TObject *Sender)
{
	char str[64] = {0};
	_snprintf (str, sizeof(str), "COM%d", m_comPort);

	int speed = DataModuleCommon->RzRegIniFile->ReadInteger(str, "Speed", 9600);

	for (int iV = 0; iV < cbComPortSpeed->Values->Count; ++iV)
	{
		if (atoi(cbComPortSpeed->Values->Strings[iV].c_str()) == speed){
			cbComPortSpeed->Value = cbComPortSpeed->Values->Strings[iV];
			break;
		}
	}

}
//---------------------------------------------------------------------------
void __fastcall TFormComPortConfig::FormClose(TObject *Sender, TCloseAction &Action)
{
	m_newSpeed = 0;
	if (ModalResult == mrOk && m_comPort > 0) {
		char str[64] = {0};
		_snprintf (str, sizeof(str), "COM%d", m_comPort);

		DataModuleCommon->RzRegIniFile->WriteInteger(str, "Speed", cbComPortSpeed->Value.ToInt());

		m_newSpeed = cbComPortSpeed->Value.ToInt();
	}
}
//---------------------------------------------------------------------------
