#include <string>

#include "../GlobalVars.h"
#include "../file_io.h"
#include "../socket_funcs.h"
#include "../Room.h"
#include "../user_objects.h"
#include "../softboot.h"
#include "vars.h"
#include "UserGlobalVar.h"

const char* const UserGlobalVar::unsetString = "--UNSET--";

UserGlobalVar::UserGlobalVar( pod_string theName, int theInitType ) : BasicVar(theName,theInitType)
{
   init();
}

int UserGlobalVar::init()
{
   userName = unsetString;
   user = 0;
   return 0;
}

int UserGlobalVar::setFromString(pod_string value)
{
   userName = value;
   user = 0;
   if( userName != unsetString )
   {
      set_user_link( &user, userName );
   }
   return 0;
}

pod_string UserGlobalVar::renderToString()
{
   return userName;
}

bool UserGlobalVar::set( UR_OBJECT user )
{
   this->user = user;
   if( user )
   {
      userName = user->name;
   }
   else
   {
      userName = unsetString;
   }
   return true;
};

UR_OBJECT UserGlobalVar::get()
{
   return user;
};

BasicVar* UserObjectCreator::getObjectInstance(pod_string name)
{
   return new UserGlobalVar( name, 0 );
};
