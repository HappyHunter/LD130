////////////////////////////////////////////////////////////////////
//
// 	Alexandr Filenkov 2010
//
////////////////////////////////////////////////////////////////////


#ifndef LD130Common_201012161423
#define LD130Common_201012161423

/**
 * The controller can have up to 4 banks to hold the programming
 * settings for it's output heads
 *
 * The banks are numbererd from 1, so the valid bank IDs are
 * 1,2,3,4
*/
#define MAX_NUM_OF_BANKS 4

/**
 * The controller can have up to 2 output heads
 *
 * The heads are numbered from 1, so the valid head IDs are 1,2
*/
#define MAX_NUM_OF_HEADS 2


/**
 * The maximum number of controllers connected to 1 PC
 */
#define MAX_NUM_OF_CONTROLLERS 10


// this flags are the same as TFlashInfoFlags
typedef enum tag_ConfigDataFlags
{
	fifUnknown		= 0x0000,
	fifAutoLoad		= 0x0001,		// automaticaly load the parameters from flash on bootup
	fifUseBanks		= 0x0002,		// Use banks
} TConfigDataFlags;

//-----------------------------------------------------------------------------------------
// the trigger ID bitmask
//
// We can select which trigger will fire output head 1 or 2
//
// It is done as a bit mask so we can choose both triggers to fire the same output
//-----------------------------------------------------------------------------------------
typedef enum {
	TriggerID1	= 0x01,
	TriggerID2	= 0x02,
} TTriggerID;


//-----------------------------------------------------------------------------------------
// the trigger ID Edge enum
//-----------------------------------------------------------------------------------------
typedef enum {
	TriggerRaising	= 0,	// on raising edge of the signal
	TriggerFalling	= 1,	// on falling edge of the signal
	TriggerDC		= 2,	// no trigger, DC mode
} TTriggerEdge;


/**
 * The structure that holds all the parameters required to
 * program the output head parameters
*/
typedef struct tag_BankHeadData {
	#ifdef __cplusplus
    tag_BankHeadData()
    {
		m_bankId		= 1; // Bank ID, 1 - bank 1, 2 - Bank 2 etc.., max 4
		m_outputId		= 1; // 1 - head 1, 2 - Head 2
		m_voltage		= 0; // 0 - 100 %
		m_powerChanel1	= 0; // 0 - 100 00% with fixed decimal point at 2 digits
		m_powerChanel2	= 0; // for example the power of 35.23% will be sent as 3523
		m_powerChanel3	= 0; // the power of 99.00% will be sent as 9900
		m_powerChanel4	= 0; // the power of 100.00% will be sent as 10000
		m_strobeDelay	= 5; // the delay of outcoming light strobe in microseconds
		m_strobeWidth	= 5; // the duration of outcoming light strobe in microseconds
		m_triggerEdge	= 0; // the edge of incoming trigger to start counting, 0 - raising, 1 - falling, 2 - DC mode
		m_triggerId		= 0; // the ID of the trigger this output head will trigger on. 1 - Trigger 1, 2 - Trigger 2, 3 - Trigger 1 or 2
		m_chanelAmplifier	= 1;  // the amplification value 1-5
    }
	#endif

    unsigned short	m_bankId;		// 1, 2, 3, 4
    unsigned short	m_outputId;		// 1 - head 1, 2 - Head 2
    unsigned short	m_voltage;		// 0 - 100 %
    unsigned short	m_powerChanel1; // 0 - 100 00% with fixed decimal point at 2 digits
    unsigned short	m_powerChanel2; // for example the power of 35.23% will be sent as 3523
    unsigned short	m_powerChanel3; // the power of 99.00% will be sent as 9900
    unsigned short	m_powerChanel4; // the power of 100.00% will be sent as 10000
    unsigned long	m_strobeDelay;	// the delay of outcoming light strobe in microseconds
    unsigned long	m_strobeWidth;	// the duration of outcoming light strobe in microseconds
    unsigned short	m_triggerEdge;	// the edge of incoming trigger to start counting, 0 - raising, 1 - falling, 2 - DC mode
    unsigned short	m_triggerId;	// the ID of the trigger this output head will trigger on. 1 - Trigger 1, 2 - Trigger 2, 3 - Trigger 1 or 2
    unsigned short	m_chanelAmplifier;	 // the amplification value 1-5
	unsigned char	m_reserved[16];	 //reserver for future
} TBankHeadData;


typedef enum tag_TChanelStatus
{
	csOK			= 0,	// Chanel status OK
} TChanelStatus;

typedef struct tag_TInt64 {
    #ifdef __cplusplus
    tag_TInt64() {
		m_low = 0;
		m_hi = 0;
    }
    #endif

    unsigned long	m_low;
    unsigned long	m_hi;
} TInt64;

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

	TInt64			m_trigCountChanel1;	// life time trigger count
	TInt64			m_trigCountChanel2;	// life time trigger count
	TInt64			m_trigCountChanel3;	// life time trigger count
	TInt64			m_trigCountChanel4;	// life time trigger count

	TInt64			m_onTimeChanel1;	// ON time for the light
	TInt64			m_onTimeChanel2;	// ON time for the light
	TInt64			m_onTimeChanel3;	// ON time for the light
	TInt64			m_onTimeChanel4;	// ON time for the light

} THeadStatus;



// The controller will reply this message back with the status of each chanel
typedef struct tag_Status {
    #ifdef __cplusplus
    tag_Status() {
		m_temperatureH1 = 0;
		m_temperatureH2 = 0;
		m_temperatureAmb = 0;
    }
    #endif
    float	m_temperatureH1;
    float	m_temperatureH2;
    float	m_temperatureAmb;
} TLD130Status;





//---------------------------------------------------------------------------------
// INTERFACE section
//---------------------------------------------------------------------------------


#if defined(_MSC_VER) || defined(__BORLANDC__)

#ifdef LD130I_EXPORTS
	#define LD130I_API __declspec(dllexport)
#else
	#define LD130I_API __declspec(dllimport)
#endif


/**
 * LD130GetLastError() returns the last error code for the
 * assosiated LD130 object.
 *
 * @param aHandle is the handle returned LD130Init() function.
 *      	  The error code will be retuned for the LD130
 *      	  object specified. If aHandle is 0 then the
 *      	  last error code assosiated with init/close
 *      	  operation is returned
 *
 * @return DWORD is the error code
 *
 *
*/
#ifdef __cplusplus
extern "C"
#endif
LD130I_API DWORD LD130GetLastError(HANDLE aHandle);


// The controller will reply this message back with the status of each chanel
typedef enum tag_TLD130Errors {
	ld130OK,			// no error
	ld130ExceptionError,		// Windows Exception while acessing LD130
	ld130MaxNumOfControllersUsed,	// MAX_NUM_OF_CONTROLLERS already open, cannot open more
	ld130InvalidHandle,		// the handle specified is not valid

} TLD130Errors;


/**
 * LD130Open() opens a handle to LD130 which is same as creating
 * an instance of LD130 object.
 *
 * @param None
 *
 * @return HANDLE to the created object. When you are done using
 *  	   the object do not forget to call LD130Close()
 *
 *
*/
#ifdef __cplusplus
extern "C"
#endif
LD130I_API HANDLE LD130Open();

/**
 * LD130Close() deletes an instance of LD130 object.
 *
 * @param aHandle is the handle returned LD130Init() function
 *
 * @return bool true if the command succedded, false in case of
 *         error
 *
 *
*/
#ifdef __cplusplus
extern "C"
#endif
LD130I_API BOOL LD130Close(HANDLE aHandle);


typedef void (*TLD130OnLogEvent)(const char* pCmd, const char* pLogStr);

/**
 * LD130Init() creates an instance of LD130 object.
 *
 * @param aHandle is the handle returned LD130Init()
 * function.
 * aComPort the com port number as it is found in windows device
 * manager.
 * aSpeed is the com port speed in bauds. The default value is
 * 115200 bauds
 * onLogEvent is a callback function that will be called by
 * LD130 object to indicate any events required logging
 *
 * @return bool to indicate the success of initialization
 *
 *
*/
#ifdef __cplusplus
extern "C"
#endif
LD130I_API BOOL LD130Init(HANDLE aHandle, int aComPort, int aSpeed = 115200, TLD130OnLogEvent onLogEvent = 0);


/**
 * LD130GetVersion() returns the firmware version.
 *
 * @param aHandle is the handle returned LD130Init() function
 * const char** verMajor the pointer to a variable where to put
 * the major version string
 * const char** verMinor the pointer to a variable where to put
 * the minor version string
 * const char** verBuild the pointer to a variable where to put
 * the build version string
 *
 * @return bool true if the command succedded, false in case of
 *         error
 *
 *
*/

#ifdef __cplusplus
extern "C"
#endif
LD130I_API  BOOL LD130GetVersion(HANDLE aHandle, const char** verMajor, const char** verMinor,const char** verBuild);



/**
 * LD130SetBankHeadData() programs the light controller with the
 * parameters specified
 *
 * @param aHandle is the handle returned LD130Init() function
 * unsigned short aBankId,	// 1,2,3,4
 *
 * unsigned short aHeadId, 	// 1,2
 *
 * unsigned short aVoltage,	// 0 - 100 Volts
 *
 * unsigned short aPowerChanel1, 	// 0 - 100 00% with fixed decimal point at 2 digits
 *
 * unsigned short aPowerChanel2,	// for example the power of 35.23% will be sent as 3523
 *
 * unsigned short aPowerChanel3,	// the power of 99.00% will be sent as 9900
 *
 * unsigned short aPowerChanel4,	// the power of 100.00% will be sent as 10000
 *
 * unsigned long  aStrobeDelay,	// the delay of outcoming light strobe in microseconds
 *
 * unsigned long  aStrobeWidth,	// the duration of outcoming light strobe in microseconds
 *
 * unsigned short aTriggerEdge,	// the edge of incoming trigger to start counting, 0 - raising, 1 - falling, 2 - DC mode
 *
 * unsigned short aTriggerId,	// the ID of the trigger this output head will trigger on. 1 - Trigger 1, 2 - Trigger 2, 3 - Trigger 1 or 2
 *
 * unsigned short aChanelAmplifier	// the amplification value 1-5
 *
 * @return bool true if the command succedded, false in case of
 *         error
 *
 *
*/
#ifdef __cplusplus
extern "C"
#endif
LD130I_API BOOL LD130SetBankHeadData(HANDLE aHandle, 		// Handle to Light Controller
				unsigned short aBankId,		// 1,2,3,4
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


#endif

#endif
