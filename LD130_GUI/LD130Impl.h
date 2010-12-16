////////////////////////////////////////////////////////////////////
//
// 	Alexandr Filenkov 2010
//
////////////////////////////////////////////////////////////////////


#ifndef LD130Impl_201012151040
#define LD130Impl_201012151040

#include <string>
using std::string;

#include "CommonCmd.h"
#include "SerialConnection.h"

typedef void (*TLD130OnLogEvent)(const char* pCmd, const char* pLogStr);


typedef struct tag_BankHeadData {
    #ifdef __cplusplus
    tag_BankHeadData()
    {
		m_bankId			= 0;
		m_outputId			= 0; // 1 - head 1, 2 - Head 2
		m_voltage			= 0; // 0 - 100 Volts
		m_powerChanel1		= 0; // 0 - 100 00% with fixed decimal point at 2 digits
		m_powerChanel2		= 0; // for example the power of 35.23% will be sent as 3523
		m_powerChanel3		= 0; // the power of 99.00% will be sent as 9900
		m_powerChanel4		= 0; // the power of 100.00% will be sent as 10000
		m_strobeDelay		= 5; // the delay of outcoming light strobe in microseconds
		m_strobeWidth		= 5; // the duration of outcoming light strobe in microseconds
		m_triggerEdge		= 0; // the edge of incoming trigger to start counting, 0 - raising, 1 - falling, 2 - DC mode
		m_triggerId			= 0; // the ID of the trigger this output head will trigger on. 1 - Trigger 1, 2 - Trigger 2, 3 - Trigger 1 or 2
		m_chanelAmplifier	= 1;  // the amplification value 1-5
    }
    #endif

    unsigned short	m_bankId;		// 1, 2, 3, 4
    unsigned short	m_outputId;		// 1 - head 1, 2 - Head 2
    unsigned short	m_voltage;		// 0 - 100 Volts
    unsigned short	m_powerChanel1; // 0 - 100 00% with fixed decimal point at 2 digits
    unsigned short	m_powerChanel2; // for example the power of 35.23% will be sent as 3523
    unsigned short	m_powerChanel3; // the power of 99.00% will be sent as 9900
    unsigned short	m_powerChanel4; // the power of 100.00% will be sent as 10000
    unsigned long	m_strobeDelay;	// the delay of outcoming light strobe in microseconds
    unsigned long	m_strobeWidth;	// the duration of outcoming light strobe in microseconds
    unsigned short	m_triggerEdge;	// the edge of incoming trigger to start counting, 0 - raising, 1 - falling, 2 - DC mode
    unsigned short	m_triggerId;	// the ID of the trigger this output head will trigger on. 1 - Trigger 1, 2 - Trigger 2, 3 - Trigger 1 or 2
    unsigned short	m_chanelAmplifier;	 // the amplification value 1-5
} TBankHeadData;

// The controller will reply this message back with the status of each chanel
typedef struct tag_HeadStatus {
    #ifdef __cplusplus
    tag_HeadStatus() {
		m_statusChanel1 = 0;
		m_statusChanel2 = 0;
		m_statusChanel3 = 0;
		m_statusChanel4 = 0;
    }
    #endif
    unsigned short	m_statusChanel1;
    unsigned short	m_statusChanel2;
    unsigned short	m_statusChanel3;
    unsigned short	m_statusChanel4;
} THeadStatus;


class TLD130Impl {

public:
    TLD130Impl();
    ~TLD130Impl();

    TCommandErrorOutput init(int aComPort, int aSpeed = 9600, TLD130OnLogEvent onLogEvent = 0);

    void getVersion(string& verMajor, string& verMinor,string& verBuild) const
    {
		verMajor = m_verMajor;
		verMinor = m_verMinor;
		verBuild = m_verBuild;
    }

    TCommandErrorOutput softTrig(int aTrigId = 1);


    unsigned short 		getActiveBank() const   { return m_activeBank; }
    TCommandErrorOutput	setActiveBank(unsigned short aBankId);

    bool 	areBanksEnabled() const { return (m_cfgFlags & fifUseBanks) != 0; }

	TCommandErrorOutput setBankHeadData(unsigned short aBankId,			//1,2,3,4
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
	) ;
	TCommandErrorOutput setBankHeadData(const TBankHeadData& aData);

	TBankHeadData  getBankHeadData(unsigned short aBankId/*1,2,3,4*/, unsigned short aHeadId /*1,2*/) const;
	THeadStatus    getHeadStatus(unsigned short aHeadId /*1,2*/) ;
private:

    void readFromController();

    void getBankDataImpl(unsigned short aBankId);
    void getBankSequenceImpl();

    string m_verMajor;
    string m_verMinor;
    string m_verBuild;

    enum{

    };

    struct TBankData {
		TBankHeadData	m_headData[2];
    };

    TBankData	m_bankData[GLOBAL_BANK_COUNT];
    THeadStatus	m_headStatus[2];

    int		m_sequenceIndex;
    string	m_sequence;

    unsigned short 	m_activeBank;
    int 	m_cfgFlags;

    TSerialConMan 	m_conMan;

    TLD130OnLogEvent 	m_onLogEvent;

};

#endif
