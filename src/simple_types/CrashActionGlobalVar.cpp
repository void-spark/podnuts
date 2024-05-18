#include <string>

#include "../file_io.h"
#include "vars.h"
#include "IntGlobalVar.h"
#include "SelectGlobalVar.h"
#include "CrashActionGlobalVar.h"

CrashActionGlobalVar::CrashActionGlobalVar( char *theName,int theInitType, int theInitVal) : SelectGlobalVar(theName,theInitType,theInitVal)
{
   myOptions.push_back("NONE");
   myOptions.push_back("IGNORE");
   myOptions.push_back("REBOOT");

   init(&myOptions);
}
