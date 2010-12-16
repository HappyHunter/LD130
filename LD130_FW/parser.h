////////////////////////////////////////////////////////////////////
//
// 	Alexandr Filenkov 2008
//
////////////////////////////////////////////////////////////////////


#ifndef Parser_201012101442
#define Parser_201012101442

/**
 * InitializeParser() will do the Parser engine initialization.
 */
extern "C" void InitializeParser();

/**
 * Parse the incoming string and copy the found variables into
 * internal array
 *
 * @param pSentenceIn - the pointer to null terminated string
 *                    which contains the command
 *
 * @return unsigned char - the number of values+1 found in a
 *         pattern. Return value of 1 indicates that the command
 *         is found in reply, but no parameters
 */
extern "C" unsigned char ParseSentence(char * pSentenceIn);


/**
 * GetValueByNum() returns the value of the parameter by its
 * relative position in CMD, the first parameter has index 0
 *
 * @param anIndex - the relative position index in cmd, the
 *                first parameter has index 0
 *
 * @return const char* - returns pointer to a buffer
 */
extern "C" const char * GetValueByNum(unsigned char anIndex);


/**
 * GetValueByName() returns the reference to value of a variable
 * according to its name
 *
 * @param pName - pointer to the parameter name
 *
 * @return const char* - returns pointer to a buffer
 */
extern "C" const char * GetValueByName(const char * pName);


/**
 * GetCmdName() returns the last parsed command name
 *
 * @return const char* - returns pointer to a cmd name buffer
 */
extern "C" const char * GetCmdName();

#endif

