#include <iomanip>
#include <string>
#include <ctype.h>

#include "../general_headers.h"
#include "../file_io.h"
#include "vars.h"
#include "DayTimeGlobalVar.h"

DayTimeGlobalVar::DayTimeGlobalVar( char *theName,int theInitType, int initHour,int initMin)  : BasicVar(theName,theInitType)
{
   initMinVal = initMin;
   initHourVal = initHour;

   if(initMinVal  < 0 || initMinVal  > 59 || 
      initHourVal < 0 || initHourVal > 23    )
   {
      printf("init value out of bounds for %s!\n",getName()); 
      exit(13);
   }
}

int DayTimeGlobalVar::init()
{
   minVal = initMinVal;
   hourVal = initHourVal;
   return 0;
}

void DayTimeGlobalVar::setMin(const int &newval)
{
   minVal=newval;
   if(minVal < 0 || minVal > 59)
   {
      printf("minutes value out of bounds for %s!\n",getName()); 
      exit(13);
   }
}

void DayTimeGlobalVar::setHour(const int &newval)
{
   hourVal=newval;
   if(hourVal < 0 || hourVal > 23)
   {
      printf("hours value out of bounds for %s!\n",getName()); 
      exit(13);
   }
}

int DayTimeGlobalVar::setFromString(pod_string value)
{
   const char *oldstr = value.c_str();
   if ( oldstr[2] != ':'
        || strlen(oldstr) > 5
        || !isdigit(oldstr[0])
        || !isdigit(oldstr[1])
        || !isdigit(oldstr[3])
        || !isdigit(oldstr[4]) )
   {
      printf("Invalid value (%s) for %s.", oldstr,getName());
      return 1;
   }

   sscanf(oldstr, "%d:%d", &hourVal , &minVal );

   if(minVal  < 0 || minVal  > 59 ||
      hourVal < 0 || hourVal > 23    )
   {
      printf("Invalid value (%s) for %s.", oldstr,getName());
      return -1;
   }
   return 0;
}

pod_string DayTimeGlobalVar::renderToString()
{
   pod_ostringstream outputStream;
   outputStream << std::setfill('0') << std::setw(2) << hourVal << ":" << std::setw(2) << minVal;
   return outputStream.str();
}




