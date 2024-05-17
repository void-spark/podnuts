#include <string>

#include "../file_io.h"
#include "vars.h"
#include "TimeTGlobalVar.h"

TimeTGlobalVar::TimeTGlobalVar( char *theName, int theInitType, time_t theInitVal) : BasicVar(theName,theInitType)
{
   init_val = theInitVal;
}

int TimeTGlobalVar::init()
{
   timeVal = init_val;
   return 0;
}

int TimeTGlobalVar::setFromString(pod_string value)
{
   sscanf(value.c_str(),"%i",(int*)(&timeVal));
   return 0;
}

pod_string TimeTGlobalVar::renderToString()
{
   pod_ostringstream outputStream;
   outputStream <<  timeVal;
   return outputStream.str();
}
