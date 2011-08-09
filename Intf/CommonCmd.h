////////////////////////////////////////////////////////////////////
//
// 	Alexandr Filenkov 2010
//
////////////////////////////////////////////////////////////////////


#ifndef CommonCmd_201012151057
#define CommonCmd_201012151057


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


#endif
