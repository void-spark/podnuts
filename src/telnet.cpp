#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "general_headers.h"
#include "xalloc.h"
#include "string_misc.h"
#include "time_utils.h"
#include "rfc854.h"
#include "telnet.h"

#define EOR             239     /* end of record (transparent mode) */
#define TELOPT_ECHO     1       /* echo */
#define TELOPT_EOR      25      /* end or record */
#define TELOPT_NAWS     31      /* window size */

/*** Clear the screen ***/

int cls(UR_OBJECT user)
{
   IntGlobalVar *color_on    = (IntGlobalVar*)user->getVar("ColorOn");
   int i;

   if( !color_on->get() )
   {
      for(i=0;i<5;++i)
      {
         write_user(user,"\n\n\n\n\n\n\n\n\n\n");
      }
   }
   else
   {
      write_user(user,"\033[2J");
   }
   return 0;
}

int telnet_eor_send(UR_OBJECT user)
{
   if( user->socket->isUsEnabled( TELOPT_EOR ) )
   {
      user->socket->sendCommand( EOR );
   }
   return 0;
}

/*** Tell telnet to echo characters ***/
int sendEchoOn(UR_OBJECT user)
{
   logStream << setLogfile( DEBUGLOG ) << user->name << " : sendEchoOn() :" << " user->remoteEchoOn is now " << user->remoteEchoOn << "\n" << pod_send;
   
   if (password_echo.get())
   {
      return 0;
   }
   
   if( !user->remoteEchoOn )
   {
      logStream << setLogfile( DEBUGLOG ) << user->name << " : sendEchoOn() :" << " user->socket->requestUsDisable( TELOPT_ECHO );\n" << pod_send; 
      user->socket->requestUsDisable( TELOPT_ECHO );
   }
   return 0;
}

/*** Tell telnet not to echo characters - for password entry ***/
int echo_off(UR_OBJECT user)
{
   user->remoteEchoOn = user->socket->isUsEnabled( TELOPT_ECHO );
   
   logStream << setLogfile( DEBUGLOG ) << user->name << " : echo_off() :" << " user->remoteEchoOn is now " << user->remoteEchoOn << "\n" << pod_send;

   if (password_echo.get())
   {
      return 0;
   }
   
   logStream << setLogfile( DEBUGLOG ) << user->name << " : echo_off() :" << " user->socket->requestUsEnable( TELOPT_ECHO );\n" << pod_send; 
   
   user->socket->requestUsEnable( TELOPT_ECHO );
   return 0;
}

