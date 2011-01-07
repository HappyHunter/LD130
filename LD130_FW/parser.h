////////////////////////////////////////////////////////////////////
//
// 	Alexandr Filenkov 2008
//
////////////////////////////////////////////////////////////////////


#ifndef Parser_201012101442
#define Parser_201012101442

//void index
#define NOT_MATCH_INDEX 255

//count of used patterns
#define PATTERNS_COUNT 27

//maximum count of named variables
#define VALUES_COUNT 49

//maximum count of variables in pattern
#define MAX_VALUES_IN_PATTERN 50

//maximum length of variable name
#define STRING_NAME_LENGTH 20

//maximum length of value in characters
#define STRING_VALUE_LENGTH 10

/**
 * InitializeParser() will do the Parser engine initialization.
 */
#ifdef __cplusplus
extern "C"
#endif
void InitializeParser();

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
#ifdef __cplusplus
extern "C"
#endif
unsigned char ParseSentence(char * pSentenceIn);


/**
 * GetValueByNum() returns the value of the parameter by its
 * relative position in CMD, the first parameter has index 0
 *
 * @param anIndex - the relative position index in cmd, the
 *                first parameter has index 0
 *
 * @return const char* - returns pointer to a buffer
 */
#ifdef __cplusplus
extern "C"
#endif
const char * GetValueByNum(unsigned char anIndex);


/**
 * GetValueByName() returns the reference to value of a variable
 * according to its name
 *
 * @param pName - pointer to the parameter name
 *
 * @return const char* - returns pointer to a buffer
 */
#ifdef __cplusplus
extern "C"
#endif
const char * GetValueByName(const char * pName);


/**
 * GetCmdName() returns the last parsed command name
 *
 * @return const char* - returns pointer to a cmd name buffer
 */
#ifdef __cplusplus
extern "C"
#endif
const char * GetCmdName();


/**
 * IsValidInteger() returns the last parsed command name
 *
 * @return unsigned char
 *  0 - varibale is invalid integer
 *  1 - variable is valid integer,
 *  2 - variable is empty
 */
#ifdef __cplusplus
extern "C"
#endif
unsigned char IsValidInteger(const char * pName);

#endif

