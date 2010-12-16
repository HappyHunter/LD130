//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "CommonData.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "RzCommon"
#pragma resource "*.dfm"
TDataModuleCommon *DataModuleCommon;
//---------------------------------------------------------------------------
__fastcall TDataModuleCommon::TDataModuleCommon(TComponent* Owner)
	: TDataModule(Owner)
{
}
//---------------------------------------------------------------------------
