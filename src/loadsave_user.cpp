#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "general_headers.h"
#include "string_misc.h"
#include "macro.h"
#include "loadsave_user.h"

int load_user( UR_OBJECT user )
{
   if(user->load())
   {
      return 1;
   }
   else
   {
      return 0;
   }
}

/*** Save a users stats ***/
int save_user( UR_OBJECT user )
{
   user->save();
   return 1;
}
