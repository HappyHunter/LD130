////////////////////////////////////////////////////////////////////
//
// 	Alexandr Filenkov 2010
//
////////////////////////////////////////////////////////////////////


#ifndef LD130Impl_201012151040
#define LD130Impl_201012151040

#include <string>
using std::string;

#include "../Common/LD130Common.h"
#include "SerialConnection.h"

typedef void (*TLD130OnLogEvent)(const char* pCmd, const char* pLogStr);


/**
 * The interface class to control LD130 light controller
 *
 */
class TLD130Impl {

public:
    TLD130Impl();
    ~TLD130Impl();

    TCommandErrorOutput init(int aComPort, int aSpeed = 9600, TLD130OnLogEvent onLogEvent = 0);

    void close();

    bool isConnected() const;

    void getVersion(string& verMajor, string& verMinor,string& verBuild) const
    {
	verMajor = m_verMajor;
	verMinor = m_verMinor;
	verBuild = m_verBuild;
    }

    TCommandErrorOutput softTrig(int aTrigId = 1);


    // Bank ID = 1,2,3,4 when BankId=0, then banks are not used
    unsigned short getActiveBank()  { getConfigDataImpl(); return areBanksEnabled() ? m_activeBank : 0; }

    TCommandErrorOutput	setActiveBank(unsigned short aBankId);

    bool areBanksEnabled() const { return (m_cfgFlags & fifUseBanks) != 0; }

    TCommandErrorOutput setBankHeadData(unsigned short aBankId,		// 1,2,3,4
					unsigned short aHeadId, 	// 1,2
				        unsigned short aVoltage,	// 0 - 100 Volts
				        unsigned short aPowerChanel1, 	// 0 - 100 00% with fixed decimal point at 2 digits
				        unsigned short aPowerChanel2,	// for example the power of 35.23% will be sent as 3523
				        unsigned short aPowerChanel3,	// the power of 99.00% will be sent as 9900
				        unsigned short aPowerChanel4,	// the power of 100.00% will be sent as 10000
				        unsigned long  aStrobeDelay,	// the delay of outcoming light strobe in microseconds
				        unsigned long  aStrobeWidth,	// the duration of outcoming light strobe in microseconds
				        unsigned short aTriggerEdge,	// the edge of incoming trigger to start counting, 0 - raising, 1 - falling, 2 - DC mode
				        unsigned short aTriggerId,	// the ID of the trigger this output head will trigger on. 1 - Trigger 1, 2 - Trigger 2, 3 - Trigger 1 or 2
				        unsigned short aChanelAmplifier	// the amplification value 1-5
					) ;


    /**
    * Sets bank data to light controller
    *
    * The function will do the parameter checking, that is why the
    * parameter is passe by value
    */
    TCommandErrorOutput setBankHeadData(TBankHeadData aData);

    TBankHeadData  getBankHeadData(unsigned short aBankId/*1,2,3,4*/, unsigned short aHeadId /*1,2*/) const;
    THeadStatus    getHeadStatus(unsigned short aHeadId /*1,2*/) ;


    string getSequence() { getBankSequenceImpl(); return m_sequence; }
    TCommandErrorOutput TLD130Impl::setSequence(const string& aValue);

    TLD130Status getStatus();
private:

    void readFromController();

    void getBankDataImpl(unsigned short aBankId);
    void getBankSequenceImpl();
    void getConfigDataImpl();


    /**
     * The version of firmware returned by microcontroller
     */
    string m_verMajor;
    string m_verMinor;
    string m_verBuild;


    /**
     * The description of the parameters for light output control
     */
    struct TBankData {
     	TBankHeadData	m_headData[2];
    };

    /**
     * There are several banks that can pre programmed
     */
    TBankData	m_bankData[MAX_NUM_OF_BANKS];

    /**
     * The current output status after programming to see under/over
     * voltage/temperature etc
     */
    THeadStatus	m_headStatus[2];

    /**
     * The current position in sequence
     */
    int		m_sequenceIndex;

    /**
     *  The bank sequence it will hold the ascii bank ID. First bank
     *  has id = '1', second bank has ID='2' etc
     */
    string	m_sequence;

    /**
     * The id of currently active bank. First bank
     *  has id = 1, second bank has ID=2 etc
    */
    unsigned short 	m_activeBank;

    /**
     * Configuration flags defined in TConfigDataFlags
    */
    int 	m_cfgFlags;


    /**
     * The connection manager that we use to send/recieve data
     */
    TSerialConMan 	m_conMan;


    /**
     * The custom callback function, which will be called after
     * sending data to the controller. This callback function can be
     * used for logging  purpose
     */
    TLD130OnLogEvent 	m_onLogEvent;

};

#endif
