////////////////////////////////////////////////////////////////////
//
// 	Alexandr Filenkov 2010
//
////////////////////////////////////////////////////////////////////

#ifndef LCDMAN_1057256
#define LCDMAN_1057256

/**
* Starts the LCD low priority task to update display
*/
void Task_LCDMan (void);

/**
* set the text on the LCD for a given line
*
* @parameter aScreenId is the id of the screen (0 or 1) aLine is the line number (0,1,2,3)
* pText is the text to be displayed
*/
void setLcdText(unsigned char aScreenId, unsigned char aLine, unsigned char aCol, const char* pText);
void setLcdChar(unsigned char aScreenId, unsigned char aLine, unsigned char aCol, char aChar);
char getLcdChar(unsigned char aScreenId, unsigned char aLine, unsigned char aCol);
#endif
