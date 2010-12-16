//---------------------------------------------------------------------------

#ifndef ComPortConfigH
#define ComPortConfigH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include "RzCmboBx.hpp"
#include "RzPanel.hpp"
#include <ExtCtrls.hpp>
#include "RzButton.hpp"
//---------------------------------------------------------------------------
class TFormComPortConfig : public TForm
{
__published:	// IDE-managed Components
	TRzGroupBox *RzGroupBox1;
	TRzComboBox *cbComPortSpeed;
	TLabel *Label1;
	TRzBitBtn *btnOk;
	TRzBitBtn *btnCancel;
	void __fastcall FormShow(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
private:	// User declarations
public:		// User declarations
	__fastcall TFormComPortConfig(TComponent* Owner);
	int	m_comPort;
	int m_newSpeed;
};
//---------------------------------------------------------------------------
extern PACKAGE TFormComPortConfig *FormComPortConfig;
//---------------------------------------------------------------------------
#endif
