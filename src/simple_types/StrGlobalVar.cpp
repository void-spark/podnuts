#include <string>

#include "../GlobalVars.h"
#include "../file_io.h"
#include "vars.h"
#include "StrGlobalVar.h"

StrGlobalVar::StrGlobalVar( pod_string theName, int len,int theInitType, pod_string theInitStr) : BasicVar(theName,theInitType)
{
   if (len < 0 )
   {
      abort();
   }
   myStr   = "";
   length  = len;

   initStr = theInitStr;
}

int StrGlobalVar::init()
{
   if(!set(initStr))
   {
      printf("in class StrGlobalVar: Init string to long for glob var %s\n",name.c_str());
      abort();
   }
   return 0;
}   

int StrGlobalVar::setFromString(pod_string value)
{
   myStr = value;

   return 0;
}

pod_string StrGlobalVar::renderToString()
{
   return myStr;
}

bool StrGlobalVar::set(pod_string str)
{
   if(str.size() > length)
   {
      return false;
   }
   myStr = str;
   return true;
};

pod_string StrGlobalVar::get()
{
   return myStr;
};

bool StrGlobalVar::isEmpty()
{
   if( myStr.size() == 0 )
   {
      return true;
   }
   return false;
}

BasicVar* StrObjectCreator::getObjectInstance(pod_string name)
{
   return new StrGlobalVar(name,_lengthVal,0,_initVal);
};
