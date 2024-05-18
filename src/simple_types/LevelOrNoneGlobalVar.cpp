#include <string>

#include "../general_headers.h"
#include "../file_io.h"
#include "vars.h"
#include "LevelOrNoneGlobalVar.h"

LevelOrNoneGlobalVar::LevelOrNoneGlobalVar( char *theName,int theInitType, int theInitVal) : IntGlobalVar(theName,theInitType,theInitVal)
{
   if(init_val < -2 || init_val > LEV_BOT)
   {
      printf("init value out of bounds for %s!\n",getName()); 
      exit(13);
   }
}

int LevelOrNoneGlobalVar::set(const int &newval)
{
   val=newval;
   if(val < -2 || val > LEV_BOT)
   {
      printf("value out of bounds for %s!\n",getName()); 
      exit(13);
   }
   return val;
}

int LevelOrNoneGlobalVar::setFromString(pod_string value)
{
   if ( (val = get_level(value.c_str())) == -2)
   {
      if (strcmp(value.c_str(), "NONE"))
      {
         printf("Unknown level specifier '%s' for %s.\n",value.c_str(), getName());
         return 1;
      }
   }

   return 0;
}

pod_string LevelOrNoneGlobalVar::renderToString()
{
   pod_ostringstream outputStream;
   if(val != -2)
   {
      outputStream << getLevelShortName(val);
   }
   else
   {
      outputStream << "NONE";
   }
   return outputStream.str();
}
