#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <string.h>
#include "talker_constants.h"
#include "socket_funcs.h"
#include "string_misc.h"
#include "logging.h"
#include "dns_socket.h"

pod_string get_ip_address(struct sockaddr_in acc_addr);

SocketInterface socketInterface;

pod_string SocketInterface::getPortsString()
{
   pod_string str;
   LSocketsList::iterator listenSocketNode, listenSocketNext;
   pod_ostringstream tempStream;

   tempStream << "~FTPorts (";

   for(listenSocketNode=listenSocketsList.begin();listenSocketNode!=listenSocketsList.end();listenSocketNode++)
   {
      listenSocketNext = listenSocketNode;
      listenSocketNext++;

      tempStream << (*listenSocketNode)->getName();
      if( listenSocketNext != listenSocketsList.end() )
      {
         tempStream << "/";
      }
   }

   tempStream << ")   : ";

   for(listenSocketNode=listenSocketsList.begin();listenSocketNode!=listenSocketsList.end();listenSocketNode++)
   {
      listenSocketNext = listenSocketNode;
      listenSocketNext++;

      tempStream << "~FG" << (*listenSocketNode)->getLocalPort();
      if( listenSocketNext != listenSocketsList.end() )
      {
         tempStream << ", ";
      }
   }

   return tempStream.str();
}

int SocketInterface::addListenSocket( ListenSocket *node )
{
   if ( !node )
   {
      write_syslog("In function SocketInterface::addListenSocket, null pointer!\n",1);
      return 1;
   }

   listenSocketsList.push_back(node);
   return 0;
}

int SocketInterface::addPlainSocket( PlainSocket *node )
{
   if ( !node )
   {
      write_syslog("In function SocketInterface::addPlainSocket, null pointer!\n",1);
      return 1;
   }

   plainSocketsList.push_back(node);
   return 0;
}

int SocketInterface::deletePlainSocket( PlainSocket *node )
{
   if ( !node )
   {
      write_syslog("In function SocketInterface::deletePlainSocket, null pointer!\n",1);
      return 1;
   }
   plainSocketsList.remove(node);
   delete node;
   return 0;
}

void SocketInterface::handleRequests()
{
   setupReadMask();
   setupWriteMask();

   /* wait until either a port in the readmask received inpput, or one in the
      writemask is ready for writing, or until we get interupted by a signal
      like SIGTERM, SIGALRM, etc. */
   if (select(FD_SETSIZE,&_readMask,&_writeMask,0,0)==-1)
   {
      if(errno == EINTR) return; /* got interrupted (probably by our SIGALRM), go on */
      write_syslogf("Error in select() \"%s\".\n",true,strerror(errno));
      return;
   }

   /* check listen ports for any pending connections and accept them */
   checkAndHandleNewConnections();

   /* check if sockets write ready and flush their queue out if so */
   writeCycle();

   /* check sockets for input and parse it */
   readCycle();

   /* close any sockets marked to be closed */
   closeCycle();
}

/**
from some STL manual on <list> :
List reallocation occurs when a member function must insert or erase elements of the
controlled sequence. In all such cases, only iterators or references that point at erased
portions of the controlled sequence become invalid.

So in all the loops that might destroy the object pointed to by the iterator, we need to
store the next iterator in advance and not use the current iterator anymore
*/

int sock_close_conn(PlainSocket *socket )
{
   return socketInterface.closeSocketHard(socket);
}

int SocketInterface::closeSocketHard( PlainSocket *socket )
{
   int sock_num;

   sock_num=socket->getFileDescriptor();
   socketInterface.deletePlainSocket(socket);
   close(sock_num);

   return 0;
}

/* closing policy:
  case 1, we decide to close a fully enabled connection (client quit orso) :
     we notify via disconnect_handler() (which can possibly mark as quit, which is ok but useless
     we kill the Sock
  case 2, rest of pod decides to kill the connection (user quit or killed) :
     they tell us by flag_as_closed, and dont use disconnect handler cause they
     want differnt logout etc.
     we kill the Sock, after the loop
*/

/**
   cycle through all the sockets, handling input and disconnects
*/
int SocketInterface::readCycle()
{
   SocketsList::iterator socketNode,nextSocketNode;

   socketNode=plainSocketsList.begin();
   while(socketNode != plainSocketsList.end())
   {
      nextSocketNode=socketNode;
      nextSocketNode++;
      if( !(*socketNode)->isFlaggedForClosing() && readMaskContains(*socketNode) )
      {
         if( (*socketNode)->handleInput() == -1 )
         {
            (*socketNode)->disconnect_handler();
            socketInterface.closeSocketHard(*socketNode);
         }
      }

      socketNode=nextSocketNode;
   }
   return 0;
}

int SocketInterface::writeCycle()
{
   SocketsList::iterator socketNode,nextSocketNode;

   socketNode=plainSocketsList.begin();
   while (socketNode != plainSocketsList.end())
   {
      nextSocketNode=socketNode;
      nextSocketNode++;

      if ( writeMaskContains(*socketNode) )  /* see if socket write ready else continue */
      {
         if (!(*socketNode)->flush_queue()) /* see if client (eg telnet) has closed socket */
         {
            (*socketNode)->disconnect_handler();
            closeSocketHard(*socketNode);
         }
      }

      socketNode=nextSocketNode;
   }
   return 0;
}

/* close all socket marked to be closed */
int SocketInterface::closeCycle()
{
   SocketsList::iterator socketNode,nextSocketNode;

   socketNode=plainSocketsList.begin();
   while (socketNode != plainSocketsList.end())
   {
      nextSocketNode=socketNode;
      nextSocketNode++;

      if( (*socketNode)->isFlaggedForClosing() ) closeSocketHard(*socketNode);

      socketNode=nextSocketNode;
   }
   return 0;
}

/* closes all sockets,
   intended for softboot,shutdown etc. only, doesnt call disconnect handler */
int SocketInterface::closeAllPlainSockets()
{
   SocketsList::iterator socketNode,nextSocketNode;

   socketNode=plainSocketsList.begin();
   while (socketNode != plainSocketsList.end())
   {
      nextSocketNode=socketNode;
      nextSocketNode++;

      closeSocketHard(*socketNode);

      socketNode=nextSocketNode;
   }
   return 0;
}

/*** Initialise sockets on ports ***/

int SocketInterface::initListenSockets()
{
   LSocketsList::iterator listenSocketNode,listenSocketSubNode;

   printf("-- BOOT : Initialising sockets on ports: "); 

   for(listenSocketNode=listenSocketsList.begin();listenSocketNode!=listenSocketsList.end();)
   {
      ListenSocket* currentSocket = *listenSocketNode;
      pod_string this_id = (*listenSocketNode)->getName();
      int this_port  = (*listenSocketNode)->getLocalPort();
      listenSocketNode++; // can't use it anymore if we erase it

      if(currentSocket->getLocalPort() == -1)
      {
         listenSocketsList.remove(currentSocket);
         continue;
      }

      printf(" %s:%d%c", this_id.c_str(), this_port, ( listenSocketNode != listenSocketsList.end() ) ? ',' : '\n');
   }

   for(listenSocketNode=listenSocketsList.begin();listenSocketNode!=listenSocketsList.end();listenSocketNode++)
   {
      pod_string first_id = (*listenSocketNode)->getName();
      int first_port  = (*listenSocketNode)->getLocalPort();

      for(listenSocketSubNode=listenSocketsList.begin();listenSocketSubNode!=listenSocketsList.end();listenSocketSubNode++)
      {
         pod_string secnd_id = (*listenSocketSubNode)->getName();
         int secnd_port  = (*listenSocketSubNode)->getLocalPort();

         if( !strcmp(first_id.c_str(),secnd_id.c_str()) ) continue;

         if( first_port == secnd_port )
         {
            std::cerr << "          " << TALKER_NAME << ": Port numbers for " << first_id << " and " << secnd_id << " must be unique.\n";
            logStream << setLogfile( BOOTLOG ) << noTime << "BOOT FAILURE: Error while parsing configuration file.\n" << pod_send;
            return -1;
         }
      }
   }


   for(listenSocketNode=listenSocketsList.begin();listenSocketNode!=listenSocketsList.end();listenSocketNode++)
   {
      if( (*listenSocketNode)->create() == -1)
      {
         return -1;
      }
   }
   return 0;
}

/* check for connection to listen sockets, and 'accept' it if one found */

int SocketInterface::checkAndHandleNewConnections()
{
   LSocketsList::iterator listenSocketNode;

   for(listenSocketNode=listenSocketsList.begin();listenSocketNode!=listenSocketsList.end();listenSocketNode++)
   {
      if( readMaskContains(*listenSocketNode) ) /* checks if this main port has someone on it*/
      {
         (*listenSocketNode)->AcceptConnection();
      }
   }
   return 0;
}

int SocketInterface::closeListenSockets()
{
   LSocketsList::iterator listenSocketNode;

   for(listenSocketNode=listenSocketsList.begin();listenSocketNode!=listenSocketsList.end();listenSocketNode++)
   {
      close((*listenSocketNode)->getFileDescriptor());
   }
   return 0;
}

int SocketInterface::getListenPortNumber(pod_string id)
{
   LSocketsList::iterator listenSocketNode;
   pod_string idUpper( id );

   strToUpper( idUpper );

   for(listenSocketNode=listenSocketsList.begin();listenSocketNode!=listenSocketsList.end();listenSocketNode++)
   {
      if( (*listenSocketNode)->getName() == idUpper )
      {
         return (*listenSocketNode)->getLocalPort();
      }
   }
   return 0;
}

/*** Set up readmask for select ***/
/*** ( contains list of all the ports listened to ) ***/
int SocketInterface::setupReadMask()
{
   LSocketsList::iterator listenSocketNode;
   SocketsList::iterator socketNode;

   FD_ZERO(&_readMask);  /* set mask (ports listened to) to nothing */

   for(listenSocketNode=listenSocketsList.begin();listenSocketNode!=listenSocketsList.end();listenSocketNode++)
   {
      FD_SET((*listenSocketNode)->getFileDescriptor(),&_readMask); /* add this socket */
   }
   for(socketNode  = plainSocketsList.begin(); socketNode != plainSocketsList.end(); socketNode++ )
   {
      FD_SET((*socketNode)->getFileDescriptor(),&_readMask); /* add this socket */
   }
   return 0;
}

int SocketInterface::setupWriteMask()
{
   SocketsList::iterator socketNode;

   FD_ZERO(&_writeMask);  /* set mask (ports listened to) to nothing */

   for(socketNode=plainSocketsList.begin();socketNode!=plainSocketsList.end();socketNode++)
   {
      if ( !(*socketNode)->isQueueEmpty() ) FD_SET((*socketNode)->getFileDescriptor(),&_writeMask); /* add this socket */
   }

   return 0;
}

void SocketInterface::procShow( std::ostream &output )
{
   SocketsList::iterator socketNode;

   output << gen_seperator_line("sockets info") << "\n";

   for(socketNode  = plainSocketsList.begin();
       socketNode != plainSocketsList.end();
       socketNode++ )
   {
      output << "file descriptor        : " << (*socketNode)->getFileDescriptor() << "\n";
      output << "connection time        : " << (int)((*socketNode)->connect_time) << "\n";
      output << "site                   : " << (*socketNode)->getPeerSite() << "\n";
      output << "nummeric site          : " << (*socketNode)->ip_num << "\n";
      output << "listen port used       : " << (*socketNode)->getLocalPort() << "\n";
      output << "current local port     : " << (*socketNode)->getPeerPort() << "\n";

//      output << "flags                  : " << (*socketNode)->_flags << "\n";
//      output << "queue                  : (" << (*socketNode)->_queue << ") " << (*socketNode)->_queue) << "\n";
//      output << "queue                  : " << (*socketNode)->_queue) << "\n";
//      output << "queue_len              : " << (*socketNode)->_queue_len << "\n";

      output << "isFlaggedForClosing    : " << ( (*socketNode)->isFlaggedForClosing() ? "Yes" : "No" ) << "\n";
      output << "isFlaggedAsCloseOnExecv: " << ( (*socketNode)->isFlaggedAsCloseOnExecv() ? "Yes" : "No" ) << "\n";
      output << "isQueueEmpty           : " << ( (*socketNode)->isQueueEmpty() ? "Yes" : "No" ) << "\n";
      output << "this                   : " << *socketNode << "\n";

      output << gen_seperator_line("") << "\n";
   }
   output << "\n";
}


//+//////////////////////////ListenSocket////////////////////////////////////+/

int ListenSocket::create()
{
   struct sockaddr_in bind_addr;
   int on,size;

   size = sizeof(struct sockaddr_in);

   bind_addr.sin_family=AF_INET;
   bind_addr.sin_addr.s_addr=INADDR_ANY;

   _socket = socket(AF_INET,SOCK_STREAM,0);
   if ( _socket == -1 )
   {
      std::cout << "          " << TALKER_NAME << ": Can't open " << _id << " port listen socket: " << strerror(errno) << "\n";
      logStream << setLogfile( BOOTLOG ) << noTime << "BOOT FAILURE: Can't open " << _id << " port listen socket.\n" << pod_send;
      return -1;
   }

   /* allow reboots on port even with TIME_WAITS */
   on = 1; // as in yes, do this
   setsockopt(_socket,SOL_SOCKET,SO_REUSEADDR,(char *)&on,sizeof(on));

   /* bind sockets and set up listen queues */
   bind_addr.sin_port=htons(_localPort);
   if( bind(_socket,(struct sockaddr *)&bind_addr,size) == -1 )
   {
      std::cout << "          " << TALKER_NAME << ": Can't bind to " << _id << " port: " << strerror(errno) << "\n";
      logStream << setLogfile( BOOTLOG ) << noTime << "BOOT FAILURE: Can't bind to " << _id << " port.\n" << pod_send;
      return -1;
   }

   if( listen(_socket,10) == -1 )
   {
      std::cout << "          " << TALKER_NAME << ": Listen error on " << _id << " port: " << strerror(errno) << "\n";
      logStream << setLogfile( BOOTLOG ) << noTime << "BOOT FAILURE: Listen error on " << _id << " port.\n" << pod_send;
      return -1;
   }
   /*
      "There may not always be a connection waiting after a SIGIO is delivered or select(2)
       or poll(2) return a readability event because the connection might have been removed
       by an asynchronous network error or another thread before accept is called. If this
       happens then the call will block waiting for the next connection to arrive. To ensure
       that accept never blocks, the passed socket s needs to have the O_NONBLOCK flag set."
   */
   fcntl(_socket,F_SETFL,O_NONBLOCK);

   /* this will close a listen socket on exec , done so on a softboot exev'd children like the bot don't keep the
      listen ports open, preventing the new talker to reconnect to them */
   fcntl(_socket,F_SETFD,1);

   return 0;
}

PlainSocket* ListenSocket::AcceptConnection()
{
   int newSocket                = -1;
   PlainSocket *newSocketObject = 0;

   /* Accept the first pending connection request on lsock, making a new
      socket with the same properties as lsock.*/
   newSocket = accept(_socket,0,0);

   /* set to close on an exec, so no child processes 'll inherit and keep open the ports,
      need to reset this on softboot tho! */
   fcntl(newSocket,F_SETFD,1);

   if( newSocket == -1 )
   {
      write_syslogf("Error in accept(%i) \"%s\".\n",true,_socket,strerror(errno));
      return 0;
   }

   newSocketObject = _plainSocketCreator->getObjectInstance( newSocket );
   if( !newSocketObject )
   {
      close(newSocket);
      return 0;
   }
   newSocketObject->resolveName();
/*   socketInterface.addPlainSocket( newSocketObject );
   newSocketObject->initializeSocket();*/

   return newSocketObject;
}

void PlainSocket::resolveName()
{
/*
#warning moet ip_num grbruiken ipv. fd, getpeername, blah, etc.
   struct sockaddr_in sin;
#ifndef __CYGWIN__
   unsigned int size = sizeof(struct sockaddr_in);
#else
   int size = sizeof(struct sockaddr_in);
#endif

   memset(&sin, 0, size);
   if (getpeername(_socket, (struct sockaddr *)&sin, &size) == -1)
   {
      write_syslogf("Error in getpeername(%i) : %s.\n",TRUE,_socket,strerror(errno));
      abort();
   }
   _peerSite          = get_ip_address(sin);      // DNS name of addr
*/
   query_ip_address_external( this );
}

pod_string ListenSocket::getName()
{
   return _id;
}

ListenSocket::~ListenSocket()
{
}

//-//////////////////////////ListenSocket////////////////////////////////////-/


//+//////////////////////////PlainSocket////////////////////////////////////+/
/*** Get net address of accepted connection ***/
pod_string get_ip_address(struct sockaddr_in acc_addr)
{
   pod_string site;
   struct hostent *host;

   host = gethostbyaddr((char *)&acc_addr.sin_addr,sizeof(acc_addr.sin_addr),AF_INET);

   if (host)
   {
      if((strlen(host->h_name)+1) > MAX_SITE_LEN)
      {
         write_syslogf("Error in get_ip_address(), ip_name exeeds %d chars.\n",true,MAX_SITE_LEN);
         site = "url.to.long.error"; /* just so it's obvious what went wrong . */
      }
      else
      {
         site = host->h_name; // copy the hostname to 'site'
      }
   }
   else
   {
   /* put number adress in 'site' instead */
      site = inet_ntoa(acc_addr.sin_addr);
   }

   /* I doubt the functionality of this, aren't urls case sensitive ? */
   strToLower(site);
   return site;
}

PlainSocket::PlainSocket( int fileDescriptor , pod_string peerSite ) : _flags(0)
{
   constructorFunc(fileDescriptor);
   _peerSite          = peerSite;
}

PlainSocket::PlainSocket(int fileDescriptor) : _flags(0)
{
   constructorFunc(fileDescriptor);
   _peerSite          = ip_num;
}

PlainSocket::PlainSocket() : _flags(0)
{
}

void PlainSocket::connectIp( pod_string ip, int port )
{
   struct sockaddr_in connect_addr;
   struct sockaddr_in sin;
   int newSocket    = -1;
#ifndef __CYGWIN__
   unsigned int size = sizeof(struct sockaddr_in);
#else
   int size = sizeof(struct sockaddr_in);
#endif

   newSocket = socket(AF_INET,SOCK_STREAM,0);
   if ( newSocket == -1 )
   {
      abort();
   }

   memset(&connect_addr, 0, sizeof(sockaddr_in));
   connect_addr.sin_family = AF_INET;
   connect_addr.sin_port   = htons (port);
   inet_aton(ip.c_str(), (in_addr*)&(connect_addr.sin_addr.s_addr) );

/*   if( connect(newSocket,(struct sockaddr *)&connect_addr,size) == -1 )
   {
      abort();
   }*/
   while( connect(newSocket,(struct sockaddr *)&connect_addr,size) == -1 );

   /* set to close on an exec, so no child processes 'll inherit and keep open the ports,
      need to reset this on softboot tho! */
   fcntl(newSocket,F_SETFD,1);

   connect_time = time(0);                       /* connection time */

   setFileDescriptor( newSocket );                     /* socket descriptor (for write(),close() etc. ) */

   strcpy(ip_num,       inet_ntoa(connect_addr.sin_addr)); /* number addr */
   _peerSite          = ip_num;
   _peerPort          = (int)ntohs(connect_addr.sin_port); /* port */


   memset(&sin, 0, size);
   if (getsockname(newSocket, (struct sockaddr *)&sin, &size) == -1)
   {
      write_syslogf("Error in getsockname(%i) : %s.\n",true,newSocket,strerror(errno));
      abort();
   }
   _localPort         = (int)ntohs(sin.sin_port);                         /* used listen port */


   flag_as_close_on_execv(true);

   return;
}

void PlainSocket::constructorFunc( int fileDescriptor )
{
#ifndef __CYGWIN__
   unsigned int size = sizeof(struct sockaddr_in);
#else
   int size = sizeof(struct sockaddr_in);
#endif

   struct sockaddr_in sin;

   connect_time = time(0);                       /* connection time */

   setFileDescriptor( fileDescriptor );                     /* socket descriptor (for write(),close() etc. ) */

   memset(&sin, 0, size);
   if (getsockname(fileDescriptor, (struct sockaddr *)&sin, &size) == -1)
   {
      write_syslogf("Error in getsockname(%i) : %s.\n",true,fileDescriptor,strerror(errno));
      abort();
   }
   _localPort         = (int)ntohs(sin.sin_port);                         /* used listen port */

   memset(&sin, 0, size);
   if (getpeername(fileDescriptor, (struct sockaddr *)&sin, &size) == -1)
   {
      write_syslogf("Error in getpeername(%i) : %s.\n",true,fileDescriptor,strerror(errno));
      abort();
   }
   strcpy(ip_num,       inet_ntoa(sin.sin_addr)); /* number addr */
   _peerPort          = (int)ntohs(sin.sin_port); /* port */
}


PlainSocket::~PlainSocket()
{
   kill_queue();
}

pod_string PlainSocket::getPeerSite()
{
   return _peerSite;
}

void PlainSocket::setPeerSite( pod_string site )
{
   _peerSite =  site;
}

int PlainSocket::getPeerPort()
{
   return _peerPort;
}

int PlainSocket::flag_as_closed()
{
   _flags |= SOCKET_QUIT;
   return 0;
}

int PlainSocket::flag_as_close_on_execv(bool value)
{
   fcntl(_socket, F_SETFD, value);
   return 0;
}

bool PlainSocket::isFlaggedAsCloseOnExecv()
{
   return fcntl(_socket, F_GETFD, 0);
}


bool PlainSocket::isFlaggedForClosing()
{
   return (_flags & SOCKET_QUIT);
}

int PlainSocket::write_queue(pod_string & str)
{
   _queueStringStream << str;
   return 0;
}

std::ostream & PlainSocket::getQueueStream()
{
   return _queueStringStream;
}


int PlainSocket::flush_queue()
{
   unsigned int write_count;
   ssize_t write_retval;
   pod_string queueCopy = _queueStringStream.str();
   unsigned int queueSize = queueCopy.size();

   if( !queueSize ) return 1;
   write_retval = write_checked( queueCopy.data(), queueSize );
//   write_retval = write_checked( queueCopy.data(), (queueSize > 10) ? 10 : queueSize );
   if( write_retval == -1 )
   {
      return 0;
   }

   write_count = write_retval;

   if(write_count > queueSize)
   {
      // not supposed to happen I think.
      abort();
   }
   else if(write_count == queueSize)
   {
      _queueStringStream.str("");
      return 1;
   }
   else // if(write_count < queueSize)
   {
      queueCopy.erase( 0, write_count );
      _queueStringStream.str( queueCopy );
      _queueStringStream.flush();
      return 1;
   }
}

int PlainSocket::kill_queue()
{
   fd_set writemask;
   struct timeval timeout;

   if( _queueStringStream.str().size() ) /* anything left in queue ? */
   {
      FD_ZERO(&writemask);             /* clear mask */
      FD_SET(_socket,&writemask); /* add our port */
      timeout.tv_sec=0;                /* don't wait! , we're in a hurry :) */
      timeout.tv_usec=0;
      if (select(FD_SETSIZE,NULL,&writemask,NULL,&timeout)==-1)  /* okey, check status of client */
      {
         if(errno == EINTR); /* got interrupted, not gonna try again */
         else write_syslogf("Error in kill_queue()->select() \"%s\".\n",true,strerror(errno)); /* well, oops :) */
      }
      else if (FD_ISSET(_socket,&writemask) )  flush_queue();

      if ( _queueStringStream.str().size() > 0)
      {
         /* chew up the leftovers, can't say I did'nt try :) */
         _queueStringStream.str("");
      }
   }
   return 0;
}

bool PlainSocket::isQueueEmpty()
{
   return !_queueStringStream.str().size();
}

/* wrapper for the system's 'write()' , logs errors */
ssize_t PlainSocket::write_checked(const void *buf, size_t count)
{
   ssize_t val;
   if( (val = ::write(_socket,buf,count)) == -1)
   {
      if(errno == EPIPE) return val;
      write_syslogf("Error in write_checked() \"%s\", while writing the string : \"%.*s\" of max length %i, to sock# : %i.\n",true,strerror(errno),(int)count,(char*)buf,count,_socket);
      if(errno == EAGAIN) return 0;
   }
   return val;
}

int PlainSocket::write_len(char *str,size_t count)
{
   return write_checked(str,count);
}

int PlainSocket::write(char *str)
{
   return write_checked(str,strlen(str));
}

/* return codes : 0 - read some data or didn't but you can retry
                 -1 - socket closed, peer disconnected, etc. socket should be closed */
int PlainSocket::handleInput()
{
   static char ingoing_str[ARR_SIZE];
   ssize_t len;
   if ( (len=read( getFileDescriptor(),ingoing_str,sizeof(ingoing_str)-1) ) == -1 )
   {
      if(errno == EINTR)
      {
         return 0;
      }
      else if(errno == ECONNRESET)
      { // A router has decreed that the other end no longer exists and has severed the connection
         return -1;
      }
      else
      {
         write_syslogf("Error in read in sock_cycle() : \"%s\"\n",true,strerror(errno));
      }
   }
   else if(!len) /* see if client (eg telnet) has closed socket */
   {
      return -1;
   }
   else
   {
      pod_string input(ingoing_str,len);
      
      input_handler(input); // here things happen, flags can be set
   }
   return 0;
}

//-//////////////////////////PlainSocket////////////////////////////////////-/
