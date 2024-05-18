#include <string>

#include "../file_io.h"
#include "vars.h"
#include "IntGlobalVar.h"
#include "SelectGlobalVar.h"

SelectGlobalVar::SelectGlobalVar( char *theName,int theInitType, int theInitVal) : IntGlobalVar(theName,theInitType,theInitVal)
{
}

SelectGlobalVar::SelectGlobalVar( char *theName,int theInitType, int theInitVal, StringVector *vect): IntGlobalVar(theName,theInitType,theInitVal)
{
   init(vect);
}

void SelectGlobalVar::init(StringVector *initStringVector)
{
   selectList = initStringVector;
   if( isOutOfRange(val) )
   {
      printf("init value out of bounds for %s!\n",getName());
      exit(13);
   }
}

SelectGlobalVar::~SelectGlobalVar()
{
}

bool SelectGlobalVar::isOutOfRange(int test)
{
   if( val < 0 || val >= (signed int)selectList->size() )
   {
      return 1;
   }
   return 0;
}

int SelectGlobalVar::set(const int &newval)
{
   val=newval;
   if( isOutOfRange(val) )
   {
      printf("value out of bounds for %s!\n",getName());
      abort();
   }
   return val;
}

int  SelectGlobalVar::setFromString(pod_string value)
{
   int size   = selectList->size();
   int newVal = -1;

   for(int cnt = 0; cnt < size; cnt++)
   {
      if (value == (*selectList)[cnt] )
      {
         newVal = cnt;
         break;
      }
   }

   if(newVal == -1)
   {
      printf("%s must be ", getName());
      for(int cnt = 0; cnt < size; cnt++)
      {
         if(cnt == 0)
         {
            printf("%s",(*selectList)[cnt].c_str());
         }
         else if(cnt == size-1)
         {
            printf(", %s",(*selectList)[cnt].c_str());
         }
         else
         {
            printf(" or %s",(*selectList)[cnt].c_str());
         }
      }
      printf(", found '%s'.\n",value.c_str());
      return -1;
   }

   val = newVal;
   return 0;
}

pod_string SelectGlobalVar::renderToString()
{
   pod_ostringstream outputStream;
   if( !isOutOfRange(val) )
   {
      outputStream << (*selectList)[val];
   }
   return outputStream.str();
}
