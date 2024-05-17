#include <string>

#include "../file_io.h"
#include "vars.h"
#include "IntGlobalVar.h"
#include "SelectGlobalVar.h"
#include "YesNoGlobalVar.h"

YesNoGlobalVar::YesNoGlobalVar( char *theName,int theInitType, int theInitVal) : SelectGlobalVar(theName,theInitType,theInitVal)
{
   myOptions.push_back("NO");
   myOptions.push_back("YES");

   init(&myOptions);
}
