#include "general_headers.h"
#include "string_misc.h"
#include "socket_funcs.h"
#include "TelnetSocket.h"
#include "UserTelnetHandler.h"
#include "UserTelnetSockObjectCreator.h"

PlainSocket* UserTelnetSockObjectCreator::getObjectInstance( int fileDescriptor )
{
   UserTelnetHandler *telnetHandler = new UserTelnetHandler();

   TelnetSocket* newSocket = new TelnetSocket( fileDescriptor, telnetHandler );
   
   telnetHandler->setSocket( newSocket );

   return newSocket;
}
