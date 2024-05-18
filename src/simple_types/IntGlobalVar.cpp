#include <string>

#include "../general_headers.h"
#include "../file_io.h"
#include "vars.h"
#include "IntGlobalVar.h"

IntGlobalVar::IntGlobalVar( pod_string theName,int theInitType, int theInitVal) : BasicVar(theName,theInitType)
{
   init_val = theInitVal;
   val = init_val;
}

IntGlobalVar::IntGlobalVar( pod_string theName,int theInitVal) : BasicVar(theName,0)
{
   init_val = theInitVal;
   val = init_val;
}

IntGlobalVar::operator int const& () const
{
   return val;
}


int IntGlobalVar::init()
{
   val = init_val;
   return 0;
}


BasicVar* IntObjectCreator::getObjectInstance(pod_string name)
{
   return new IntGlobalVar(name,_initVal);
};


int IntGlobalVar::setFromString(pod_string value)
{
   if(sscanf(value.c_str(),"%i",&val) != 1)
   {
      return -1;
   }
   return 0;
}

pod_string IntGlobalVar::renderToString()
{
   pod_ostringstream outputStream;
   outputStream <<  val;
   return outputStream.str();
}
