////////////////////////////////////////////////////////////////////
//
// 	Alexandr Filenkov 2011
//
////////////////////////////////////////////////////////////////////

#ifndef Monitoring_201101081722
#define Monitoring_201101081722

/**
* the task for monitoring the temperature and other conditions
*/
void Task_Monitoring();


/**
 * Returns the temperature in degrees in float
 *
 * @return float the temperature +-125C
 */
float getTemperatureH1();
float getTemperatureH2();
float getTemperatureAmb();

/**
 * Returns the status string for any errors happening
*/
const char* getErrorStatus();
#endif
