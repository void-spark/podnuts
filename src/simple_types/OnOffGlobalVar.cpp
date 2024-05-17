#include <string>

#include "../file_io.h"
#include "vars.h"
#include "IntGlobalVar.h"
#include "SelectGlobalVar.h"
#include "OnOffGlobalVar.h"

OnOffGlobalVar::OnOffGlobalVar( char *theName,int theInitType, int theInitVal) : SelectGlobalVar(theName,theInitType,theInitVal)
{
   myOptions.push_back("OFF");
   myOptions.push_back("ON");

   init(&myOptions);
}
