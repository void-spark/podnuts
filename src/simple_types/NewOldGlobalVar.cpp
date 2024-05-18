#include <string>

#include "../file_io.h"
#include "vars.h"
#include "IntGlobalVar.h"
#include "SelectGlobalVar.h"
#include "NewOldGlobalVar.h"

NewOldGlobalVar::NewOldGlobalVar( char *theName,int theInitType, int theInitVal) : SelectGlobalVar(theName,theInitType,theInitVal)
{
   myOptions.push_back("OLD");
   myOptions.push_back("NEW");

   init(&myOptions);
}
