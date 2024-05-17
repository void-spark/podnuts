#include <string>

#include "../general_headers.h"
#include "../file_io.h"
#include "vars.h"
#include "LimitedIntGlobalVar.h"

LimitedIntGlobalVar::LimitedIntGlobalVar( char *theName,int theInitType, int theInitVal,int min,int max) : IntGlobalVar(theName,theInitType,theInitVal)
{
   minVal=min;
   maxVal=max;
   
   if( init_val < minVal && minVal != -1 || 
       init_val > maxVal && maxVal != -1 )
   {
      printf("init value(%d) out of bounds (%d-%d) for %s!\n",init_val,minVal,maxVal,getName()); 
      exit(13);
   }
}

int LimitedIntGlobalVar::init()
{
   val = init_val;
   if( val < minVal && minVal != -1 || 
       val > maxVal && maxVal != -1 )
   {
      printf("init value(%d) out of bounds (%d-%d) for %s!\n",init_val,minVal,maxVal,getName()); 
      exit(13);
   }
   return 0;
}


int LimitedIntGlobalVar::set(const int &newval)
{
   val=newval;
   if( val < minVal && minVal != -1 || 
       val > maxVal && maxVal != -1 )
   {
      printf("value out of bounds for %s!\n",getName()); 
      exit(13);
   }
   return val;
}

int LimitedIntGlobalVar::setFromString(pod_string value)
{
   IntGlobalVar::setFromString(value);

   if( val < minVal && minVal != -1 ||
       val > maxVal && maxVal != -1 )
   {
      printf("value out of bounds for %s!\n",getName());
      return -1;
   }

   return 0;
}
