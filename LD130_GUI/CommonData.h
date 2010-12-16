//---------------------------------------------------------------------------

#ifndef CommonDataH
#define CommonDataH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include "RzCommon.hpp"
//---------------------------------------------------------------------------
class TDataModuleCommon : public TDataModule
{
__published:	// IDE-managed Components
	TRzRegIniFile *RzRegIniFile;
private:	// User declarations
public:		// User declarations
	__fastcall TDataModuleCommon(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TDataModuleCommon *DataModuleCommon;
//---------------------------------------------------------------------------
#endif
