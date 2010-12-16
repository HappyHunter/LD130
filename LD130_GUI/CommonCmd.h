////////////////////////////////////////////////////////////////////
//
// 	Alexandr Filenkov 2010
//
////////////////////////////////////////////////////////////////////


#ifndef CommonCmd_201012151057
#define CommonCmd_201012151057

#define GLOBAL_BANK_COUNT 4

struct TCommandErrorOutput {
	TCommandErrorOutput() {
		m_errorId[0] = 0;
		m_invalidParameterIndex = 0;
		m_errorDescription[0] = 0;
		m_command[0] = 0;
	}

	bool hasError() const { return m_errorId[0] != 0 || m_errorDescription[0] != 0; }

	char	m_errorId[32];						//
	unsigned short	m_invalidParameterIndex; 	// the index of the invalid parameter. The first parameter after CommandId has index=1
	char	m_command[32];			 			// the command itself
	char	m_errorDescription[256];			// the error description
} ;

// this flags are the same as TFlashInfoFlags
typedef enum tag_ConfigDataFlags
{
	fifUnknown		= 0x0000,
	fifAutoLoad		= 0x0001,		// automaticaly load the parameters from flash on bootup
	fifUseBanks		= 0x0002,		// Use banks
} TConfigDataFlags;

#endif
