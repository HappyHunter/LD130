////////////////////////////////////////////////////////////////////
//
// 	Alexandr Filenkov 2010
//
////////////////////////////////////////////////////////////////////

#if defined(_MSC_VER) || defined(__BORLANDC__)
#else
    #include <p30fxxxx.h>
    #include "osa.h"
#endif

#include "parser.h"
#include "common.h"
#include <string.h>


//Command Patterns
#if defined(_MSC_VER) || defined(__BORLANDC__)
const char * const CmdTemplates[PATTERNS_COUNT]  =
#else
const char * const CmdTemplates[PATTERNS_COUNT] __attribute__ ((space(auto_psv))) =
#endif
{
   "getver",
   "version,sernum,vermajor,verminor,verbuild",


//  unsigned short	m_outputId;		// 1 - head 1, 2 - Head 2
//  unsigned short	m_voltage;		// 0 - 100 %
//  unsigned short	m_powerChanel1; // 0 - 100 00% with fixed decimal point at 2 digits
//  unsigned short	m_powerChanel2; // for example the power of 35.23% will be sent as 3523
//  unsigned short	m_powerChanel3; // the power of 99.00% will be sent as 9900
//  unsigned short	m_powerChanel4; // the power of 100.00% will be sent as 10000
//  unsigned long	m_strobeDelay;	// the delay of outcoming light strobe in microseconds
//  unsigned long	m_strobeWidth;	// the duration of outcoming light strobe in microseconds
//  unsigned short	m_triggerEdge;	// the edge of incoming trigger to start counting, 0 - raising, 1 - falling, 2 - DC mode
//  unsigned short	m_triggerId;	// the ID of the trigger this output head will trigger on. 1 - Trigger 1, 2 - Trigger 2, 3 - Trigger 1 or 2
//  unsigned short	m_chanelAmplifier;	 // the amplification value 1-5
    "sethdata,outputId,voltage,powerChanel1,powerChanel2,powerChanel3,powerChanel4,strobeDelay,strobeWidth,triggerEdge,triggerId,chanelAmplifier",

    "hstatus,bankId,statusChanel1,statusChanel2,statusChanel3,statusChanel4",
    "gethstatus,outputId",

    "gethdata,outputId",
//  unsigned short	m_outputId;		// 1 - head 1, 2 - Head 2
//  unsigned short	m_voltage;		// 0 - 100 %
//  unsigned short	m_powerChanel1; // 0 - 100 00% with fixed decimal point at 2 digits
//  unsigned short	m_powerChanel2; // for example the power of 35.23% will be sent as 3523
//  unsigned short	m_powerChanel3; // the power of 99.00% will be sent as 9900
//  unsigned short	m_powerChanel4; // the power of 100.00% will be sent as 10000
//  unsigned long	m_strobeDelay;	// the delay of outcoming light strobe in microseconds
//  unsigned long	m_strobeWidth;	// the duration of outcoming light strobe in microseconds
//  unsigned short	m_triggerEdge;	// the edge of incoming trigger to start counting, 0 - raising, 1 - falling, 2 - DC mode
//  unsigned short	m_triggerId;	// the ID of the trigger this output head will trigger on. 1 - Trigger 1, 2 - Trigger 2, 3 - Trigger 1 or 2
//  unsigned short	m_chanelAmplifier;	 // the amplification value 1-5
    "hdata,outputId,voltage,powerChanel1,powerChanel2,powerChanel3,powerChanel4,strobeDelay,strobeWidth,triggerEdge,triggerId,chanelAmplifier",

//  unsigned short	m_bankId;		// 1, 2, 3, 4
//  unsigned short	m_outputId;		// 1 - head 1, 2 - Head 2
//  unsigned short	m_voltage;		// 0 - 100 %
//  unsigned short	m_powerChanel1; // 0 - 100 00% with fixed decimal point at 2 digits
//  unsigned short	m_powerChanel2; // for example the power of 35.23% will be sent as 3523
//  unsigned short	m_powerChanel3; // the power of 99.00% will be sent as 9900
//  unsigned short	m_powerChanel4; // the power of 100.00% will be sent as 10000
//  unsigned long	m_strobeDelay;	// the delay of outcoming light strobe in microseconds
//  unsigned long	m_strobeWidth;	// the duration of outcoming light strobe in microseconds
//  unsigned short	m_triggerEdge;	// the edge of incoming trigger to start counting, 0 - raising, 1 - falling, 2 - DC mode
//  unsigned short	m_triggerId;	// the ID of the trigger this output head will trigger on. 1 - Trigger 1, 2 - Trigger 2, 3 - Trigger 1 or 2
//  unsigned short	m_chanelAmplifier;	 // the amplification value 1-5
    "setbankdata,bankId,outputId,voltage,powerChanel1,powerChanel2,powerChanel3,powerChanel4,strobeDelay,strobeWidth,triggerEdge,triggerId,chanelAmplifier",

//  unsigned short	m_bankId;		// 1, 2, 3, 4
//  unsigned short	m_outputId;		// 1 - head 1, 2 - Head 2
    "getbankdata,bankId,headId",
    "bankdata,bankId,outputId,voltage,powerChanel1,powerChanel2,powerChanel3,powerChanel4,strobeDelay,strobeWidth,triggerEdge,triggerId,chanelAmplifier",

    "getcfgdata",
    "cfgdata,flags,activeBank",

    // the ID of the trigger this output head will trigger on. 1 - Trigger 1, 2 - Trigger 2, 3 - Trigger 1 or 2
    "softtrig,triggerId",

    // the bank sequence, where each element is a bank ID (0-3)
    "setseqdata,s_10,s_20,s_30,s_40,s_50,s_60,s_70,s_80,s_90,s_100,s_110,s_120,s_130,s_140,s_150,s_160,s_170,s_180,s_190,s_200,s_210,s_220,s_230,s_240,s_250,s_260,s_270,s_280,s_290,s_300,s_310,s_320,s_330,s_340,s_350,s_360,s_370,s_380,s_390,s_400",
    // the reply message will contain the length of the sequence that was stored
    "seqdatalen,seqlen",

    "getseqdata",
    // the index of the position currently programmed to the controller
    "seqdata,curIdx,s_10,s_20,s_30,s_40,s_50,s_60,s_70,s_80,s_90,s_100,s_110,s_120,s_130,s_140,s_150,s_160,s_170,s_180,s_190,s_200,s_210,s_220,s_230,s_240,s_250,s_260,s_270,s_280,s_290,s_300,s_310,s_320,s_330,s_340,s_350,s_360,s_370,s_380,s_390,s_400",

    "setportspeed,comport,baud,flags",
    "getportspeed,comport",
    "portspeed,comport,baud,flags",

    "writeeprom,flags",
    "loadeprom",
    "cleareprom",
    "eprom,flags",

//	unsigned short	m_activeBank;		// 0, 1, 2, 3
    "setbank,bankId",
    "activebank,bankId",

    "setsernum,magic,s_10,s_20,s_40",

    "getstatus",
    "status,TH1,TH2,TAmb",

    "OK",
    "ERR,errId,errPos,errTxt",
};


/**
 * the name of the last command parsed
 */
char CmdName[STRING_NAME_LENGTH+1];

/**
 * we will store the variable names here, the max length of the
 * variable is controlled by STRING_NAME_LENGTH
 */
char VarNames[VALUES_COUNT][STRING_NAME_LENGTH+1];	//49*16=784b

/**
 * after parsing the incoming string we will store actual values
 * here
 */
char VarValues[VALUES_COUNT][STRING_VALUE_LENGTH+1];	//49*10 = 490

/**
 * mapping of the variable names in pattern to the variable
 * index in our two arrays VarNames and VarValues
 *
 */
#if defined(_MSC_VER) || defined(__BORLANDC__)
unsigned char ValuesIdx[PATTERNS_COUNT][MAX_VALUES_IN_PATTERN];// 27*50=1350
#else
__eds__ unsigned char ValuesIdx[PATTERNS_COUNT][MAX_VALUES_IN_PATTERN] __attribute__ ((space(eds)));// 27*50=1350
#endif
/**
 * the separator character between values
 */
char TokenSeparator = ',';

/**
 *
 * AddNewValue() function will add the parameter name into
 * VarNames array and it will also add mapping of variable name
 * to index into ValuesIdx array
 *
 *
 * @param aName - the name of the variable, the name is case
 *              sensitive
 * @param anIdxInPattern - the index in the command, the first
 *                       parameter has index 0, the next one is
 *                       1, etc.
 * @param aCommandIdx - the index of a command in CmdTemplates
 *                    array, the first command has index 0
 *
 * @return bool - true if the parameter was added/registered
 */
unsigned char AddNewValue(const char * aName, unsigned char anIdxInPattern, unsigned char aCommandIdx)
{
    unsigned char idx;   // index of variable in our VarNames array
    unsigned char i;     // index for loop

    char*        str0;   // the destination place for storing parameters
    const char*  str1;   // pointer to the variable name

    idx = NOT_MATCH_INDEX;
    str1 = (const char *) aName;

    //The analysis of presence at the list of names just the added line
    //If such line already exists in the list - reduce the counter of names on 1
    //(ignoring dublicates)
    for(i=0; i < VALUES_COUNT; ++i)
    {
       str0 = (char *) VarNames[i];

       // no more valid names found
       if(*str0 == 0)
       {
           // The variable not found, we will add it to this index
           idx = i;
           break;
       }

       if(strcmp(str0, str1)==0)
       {
          //The variable met earlier - we are interested with an old index
          idx = i;
          break;
       }
    }

    // no place to put variables
    if (idx == NOT_MATCH_INDEX)
        return 0;

    str0 = (char *) VarNames[idx];  //to
//  str1 = (char *) aName;          //from

    // we may do extra copy of the name again, but we do not care during initialization
    // we do sizeof-1 to preserve terminating 0
    strncpy(str0, str1, sizeof(VarNames[0])-1);

    //add index of pattern, in which met variable
    ValuesIdx[aCommandIdx][anIdxInPattern] = idx;

    return 1;
}
//-----------------------------------------------------------------------------------------


/**
 * InitializeParser() will do the Parser engine initialization.
 * It will go through CmdTemplates array and initialize internal
 * structure of mapping the parameters
 *
 * This function must be called before using the parser engine
 *
 * There is no return value
 */
void InitializeParser()
{
    unsigned char i; //index variable (pattern number)
    unsigned int j; //index variable (char index in pattern)

    unsigned char tokenNum; // current token index

    unsigned int patternLength; //length of current pattern

    unsigned char chIdx; //index of the char in string
    char tmpStr[STRING_NAME_LENGTH]; //the name of the variable to be added

    // clear tmp name
    memset(tmpStr, 0, sizeof(tmpStr));

    // clear all variable names
    memset(VarNames, 0, sizeof(VarNames));

    // clear all variable values
    memset(VarValues, 0, sizeof(VarValues));

    //Array initialization:
    //Numbers of variables under numbers of patterns (need for fast search)
    for(i=0; i<PATTERNS_COUNT; ++i)
    {
        for(j=0; j<MAX_VALUES_IN_PATTERN; ++j)
        {
            ValuesIdx[i][j] = NOT_MATCH_INDEX; //void index
        }
    }

    //search all variables and initialize array of its names:

    for(i = 0; i < PATTERNS_COUNT; ++i)
    {
       tokenNum = 0;    // current token index (all chars before first token defines pattern name)
       chIdx = 0;       // clear the idx

       //scan all chars in pattern:
       patternLength = strlen(CmdTemplates[i]);

       for(j=0; j <= patternLength; ++j)
       {
           if (CmdTemplates[i][j] == TokenSeparator || CmdTemplates[i][j] == 0)
           {
               // terminate the string
               tmpStr[chIdx] = 0;

               // skip cmd and add parameters only
               if(tokenNum > 0)
               {
                   // we use tokenNum-1 since we skip the command name
                   AddNewValue(tmpStr, tokenNum-1, i);
               }


               // clear the tmp string
               memset(tmpStr, 0, sizeof(tmpStr));
               chIdx = 0;

               tokenNum++;  // next token
               continue;
           }

           if (chIdx < sizeof(tmpStr))
           {
               //add new char into internal name of variable
               tmpStr[chIdx++] = CmdTemplates[i][j];
           }
       }

   }
}
//-----------------------------------------------------------------------------------------

/**
 * Parse the incoming string and copy the found variables into
 * VarValues array
 *
 * @param pSentenceIn - the pointer to null terminated string
 *                    which contains the command
 *
 * @return unsigned char - the number of values+1 found in a
 *         pattern. Return value of 1 indicates that the command
 *         is found in reply, but no parameters
 */
unsigned char ParseSentence(char * pSentenceIn)
{
   unsigned char i; //index variable (pattern index)
   unsigned int  j; //index variable (char index)
   unsigned char valIdx;

   unsigned int patternLength; //length of current pattern

   unsigned int  sentenceLength; //length of current pattern
   unsigned int  indexToken;
   unsigned char valueNumNum;   //index of variable index in array of indexes ValuesIdx[]
   unsigned int  varLength;     // the length of variable

   char * pValue; //pointer to variable value

    // clear last CMD
    memset(CmdName, 0, sizeof(CmdName));
    // clear the buffer
    memset(VarValues, 0, sizeof(VarValues));

    for(i = 0; i < PATTERNS_COUNT; ++i)
    {
       //check of pattern name:
       //scanning of all pattern chars to first token

       indexToken = 0;

       patternLength = strlen(CmdTemplates[i]);
       for(j=0; j <= patternLength; j++)
       {
           // if we just have CMD defined
           if(CmdTemplates[i][j] == TokenSeparator  || (CmdTemplates[i][j] == 0 && CmdTemplates[i][j] == pSentenceIn[j]) )
           {
               //position of a separator token
               indexToken = j;

               varLength = j;
               if (varLength > sizeof(CmdName)-1)
                   varLength = sizeof(CmdName)-1;

               // copy the last command name
               strncpy(CmdName, pSentenceIn, varLength);

               break;
           }

           // this will break if we reach end of sentence
           if(CmdTemplates[i][j] != pSentenceIn[j])
           {
               //equal=false;
               break;
           }
       }

       // no separator found, it means there is not CMD found
       // go to next CMD
       if (indexToken == 0)
           continue;

       //analize searched pattern:
       sentenceLength = strlen(pSentenceIn);

       valueNumNum = 0; //index of the variable in array of indexes ValuesIdx[]

       // get the pointer to a value buffer we will use ValuesIdx[] to find out the mapping
       // i - is the index for CMD
       // valueNumNum - is index
       valIdx = ValuesIdx[i][valueNumNum];

       if (valIdx != NOT_MATCH_INDEX)
           pValue = (char *) VarValues[ valIdx ];
       else
           pValue = 0;


       // the current length of variable, used for overflow checking
       varLength = 0;

       // now go through the parameters
       for(j=indexToken+1; j < sentenceLength; j++)
       {
           if(pSentenceIn[j] == TokenSeparator || pSentenceIn[j] == 0)
           {
               valueNumNum++;

               // no more place for values
               if(valueNumNum >= MAX_VALUES_IN_PATTERN)
               {
                   break;
               }

               // get the pointer to a value buffer we will use ValuesIdx[] to find out the mapping
               valIdx = ValuesIdx[i][valueNumNum];

               if (valIdx != NOT_MATCH_INDEX)
                   pValue = (char *) VarValues[ valIdx ];
               else
                   pValue = 0;

               varLength = 0;
           }
           else if(varLength++ < STRING_VALUE_LENGTH && pValue)
           {
               *pValue = pSentenceIn[j];
               pValue++;
           }
       }
       return valueNumNum+1; //the number of values found +1 to indicate the cmd itself
   }
   return 0; //no one variable has not been changed
}
//-----------------------------------------------------------------------------------------

/**
 * GetValueByNum() returns the value of the parameter by its
 * relative position in CMD, the first parameter has index 0
 *
 * @param anIndex - the relative position index in cmd, the
 *                first parameter has index 0
 *
 * @return const char* - returns pointer to a buffer
 */
const char * GetValueByNum(unsigned char anIndex)
{
    if(anIndex >= VALUES_COUNT)
        return 0;

    return (const char *) VarValues[anIndex];
}
//-----------------------------------------------------------------------------------------

/**
 * GetValueByName() returns the reference to value of a variable
 * according to its name
 *
 * @param pName - pointer to the parameter name
 *
 * @return const char* - returns pointer to a buffer
 */
const char * GetValueByName(const char * pName)
{
    unsigned char i;
    const char * name;

    for(i = 0; i < VALUES_COUNT; ++i)
    {
        name = (const char *) VarNames[i];
        if(strcmp(pName, name)==0)
        {
#if defined(_MSC_VER) || defined(__BORLANDC__)
#else
			DbgOut("{");
			DbgOut(pName);
			DbgOut("=");
			DbgOut(VarValues[i]);
			DbgOut("} ");
#endif
            return (const char *) VarValues[i];
        }
    }
    return ""; //variable not found

}

//-----------------------------------------------------------------------------------------
/**
 * GetCmdName() returns the last parsed command name
 *
 * @return const char* - returns pointer to a cmd name buffer
 */
const char * GetCmdName()
{
    return CmdName;
}


//-----------------------------------------------------------------------------------------
/**
 * IsValidInteger() returns 1 if the value is valid integer number including +- sign
 *
 * @return unsigned char
 *  0 - varibale is invalid integer
 *  1 - variable is valid integer,
 *  2 - variable is empty
 */
unsigned char IsValidInteger(const char * pName)
{

    // skip empty spaces before the digits
    while(pName && *pName && *pName == ' ')
    {
        // go to next char
        ++pName;
    }

    if (!pName || *pName == 0)
        return 2;

    while(pName && *pName && *pName != ' ')
    {
        if((*pName<'0' || *pName > '9') && *pName != '+' && *pName !='-')
        {
            return 0;   // not valid
        }
        // go to next char
        ++pName;
    }

    // skip empty spaces after the digits
    while(pName && *pName)
    {
        // if we see any other character than space so it is not a valid number then
        if(*pName != ' ')
        {
            return 0;   // not valid
        }
        // go to next char
        ++pName;
    }

    return 1;   // number is OK
}
//-----------------------------------------------------------------------------------------
#if 0
void test()
{
//    char buffer[] = "$GPGGA,092204.999,4250.5589,S,14718.5084,E,1,04,24.4,19.7,M,,,,0000*1F";
	char buffer[] = "$GPRMC,111,222,333,444,555,666,777,888,999,AAA,BBB,CCC";
    const char * value = "N/A";
    InitializeParser();
    ParseSentence(buffer);
    value = GetValueByNum(0);
    value = GetValueByName("lon");
}
#endif
//-----------------------------------------------------------------------------------------

