#include <string>

#include "../general_headers.h"
#include "../file_io.h"
#include "vars.h"
#include "LevelGlobalVar.h"

LevelGlobalVar::LevelGlobalVar( char *theName,int theInitType, int theInitVal) : IntGlobalVar(theName,theInitType,theInitVal)
{
   if(init_val < -1 || init_val > LEV_BOT)
   {
      printf("init value out of bounds for %s!\n",getName()); 
      exit(13);
   }
}

int LevelGlobalVar::set(const int &newval)
{
   val=newval;
   if(val < -1 || val > LEV_BOT)
   {
      printf("value out of bounds for %s!\n",getName()); 
      exit(13);
   }
   return val;
}

int LevelGlobalVar::setFromString(pod_string value)
{
   if ( (val = get_level(value.c_str())) == -2)
   {
      printf("Unknown level specifier '%s' for %s.\n",value.c_str(), getName());
      return -1;
   }

   return 0;
}

pod_string LevelGlobalVar::renderToString()
{
   pod_ostringstream outputStream;
   outputStream <<  getLevelShortName(val);
   return outputStream.str();
}
