#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <queue>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <ext/stdio_filebuf.h>
#include <time.h>
#include "tinyxml.h"
#include "errorClasses.h"
#include "dnsdeamon.h"

const unsigned int read_fd  = 0;
const unsigned int write_fd = 1;

std::queue<dns_request*> requestsQueue;

int open_listen_socket(int port);
int accept_socket(int listenFd);
std::string doLookup( std::string address );
void parse_request( int talkerSocketFd );

int microsleep( long microSeconds )
{
//   struct timespec delay;
//   struct timespec remainder;
   struct timeval interval;
   interval.tv_sec = 0;
   interval.tv_usec = 100000;
   while( select(0, 0, 0, 0, &interval) == -1 )
   {
      if( errno == EINTR )
      {
         continue;
      }
      return -1;
   }

/*   delay.tv_sec  = 0;
   delay.tv_nsec = 100000000;

   while( nanosleep( &delay, &remainder ) == -1 )
   {
      if( errno == EINTR )
      {
         delay = remainder;
         continue;
      }
      return -1;
   }*/
   return 0;
}

main( int argc, char *argv[] )
{
   int childs            = 10;
   int port              = 7320;
   bool doFork           = false;
   int  listenFd         = -1;
   int  talkerSocketFd   = -1;
   fd_set readMask;
   struct timeval timeout;
   std::filebuf   *socketStreamBuffer = 0;
   std::iostream  *socketStream       = 0;
   childProcess childsProcs[childs];

   std::cout << "DNSDeamon v1.0" << std::endl;

   char opt;
   while( ( opt = getopt(argc,argv, "fhc:p:") ) != -1 )
   {
      switch(opt)
      {
         case 'f':
            doFork = true;
            break;
         case 'c':
            sscanf(optarg,"%d",&childs);
            break;
         case 'p':
            sscanf(optarg,"%d",&port);
            break;
         case '?':
         case 'h':
         default:
            std::cout << "Usage: " << argv[0] << " [-f] -p port -c childs." << std::endl;
            exit(1);
      }
   }

   if (doFork)
   {
      if ( fork() )
      {
         exit(0);
      }
      std::cout << "Fork succesfull, running with pid " << getpid() << "." << std::endl;
   }
   else
   {
      std::cout << "Running with pid " << getpid() << "." << std::endl;
   }
   std::cout << "Listening on port " << port << std::endl;
   std::cout << "Spawning " << childs << " childs." << std::endl;

   for( int cnt = 0; cnt < childs; cnt++)
   {
      try
      {
         childsProcs[cnt].spawn();
      }
      catch ( FatalError e )
      {
         std::cerr << "Fatal error spawning child: " << e.what() << std::endl;
         exit(11);
      }
   }

   listenFd    = open_listen_socket(port);

   while( true )
   {
      talkerSocketFd = accept_socket(listenFd);
      #warning, last param is a buffer size that used to be 1, newest libstdc++ that does odd things tho , misses each second char
      socketStreamBuffer = new __gnu_cxx::stdio_filebuf<char>( talkerSocketFd, std::ios::in|std::ios::out, 512 );
      socketStream  = new std::iostream( socketStreamBuffer );

      if( !socketStream->good())
      {
         std::cerr << "socketStream error :" << std::endl;
         std::cerr << "fail = " << socketStream->fail() << std::endl;
         std::cerr << "bad  = " << socketStream->bad()  << std::endl;
         std::cerr << "eof  = " << socketStream->eof()  << std::endl;
         exit(12);
      }

      const int OK           = 0;
      const int DISCONNECTED = 1;

      int status = OK;

      std::cout << "Accepted new connection." << std::endl;

      while( status == OK )
      {
         FD_ZERO(&readMask);
         FD_SET(talkerSocketFd, &readMask);
         for (int cnt = 0; cnt < childs; cnt++)
         {
            if( childsProcs[cnt].isIdle() )
            {
               childsProcs[cnt].takeOffQueue();
            }
            if( !childsProcs[cnt].isIdle() )
            {
               FD_SET( childsProcs[cnt].getReadFd(), &readMask);
            }
         }
         timeout.tv_sec  = 1;
         timeout.tv_usec = 0;

         int retval = select(FD_SETSIZE, &readMask, 0, 0, &timeout);

         if( retval == -1 ) // error
         {
            if( errno != EINTR )
            {
               std::cerr << "Error in parent select: " << strerror(errno) << "." << std::endl;
            }
            continue;
         }

         if( retval == 0 ) // timeout
         {
            continue;
         }

         std::cout << "Parent select signalled event." << std::endl;
         if (FD_ISSET(talkerSocketFd, &readMask))
         {
            try
            {
               parse_request( talkerSocketFd );
            }
            catch( IoError e )
            {
               std::cerr << "Error doing parse_request: " << e.what() << std::endl;
               delete socketStream;
               socketStream = 0;
               delete socketStreamBuffer;
               socketStreamBuffer = 0;
               status = DISCONNECTED;
               continue;
            #warning set all still running queries as unneeded, when they return, ignore the in process \
                     result
            }
            catch( FatalError e )
            {
               std::cerr << "Fatal error doing parse_request: " << e.what() << std::endl;
               exit(13);
            }
         }
         for (int cnt = 0; cnt < childs; cnt++)
         {
            if ( FD_ISSET(childsProcs[cnt].getReadFd(), &readMask) )
            {
               childsProcs[cnt].processResult( socketStream );
            }
         }
      }
   }
}

std::string doLookup( std::string address )
{
   int cnt;
   std::string site;
   struct hostent *host;
   struct in_addr queried_address;
   const int retries = 10;

   if( !inet_aton( address.c_str(), &queried_address ) )
   {
      throw ParseError( "Adress not in valid dot-notation num.num.num.num" );
   }

   for(cnt = 0; cnt < retries; cnt++)
   {
      host = gethostbyaddr((char *)&queried_address,sizeof(queried_address),AF_INET);

      if( !host )
      {
         switch (h_errno)
         {
            case TRY_AGAIN:
               std::cerr << "err: TRY_AGAIN\n" << std::endl;
               if( microsleep( 100000 ) == -1)
               {
                  throw DNSError("Microsleep failure.");
			   }
               break;
            case NETDB_INTERNAL:
               throw DNSError("NETDB_INTERNAL");
            case HOST_NOT_FOUND:
               throw DNSError("HOST_NOT_FOUND");
            case NO_RECOVERY:
               throw DNSError("NO_RECOVERY");
            case NO_DATA:
               throw DNSError("NO_DATA");
            default:
               throw DNSError("UNRECOGNIZED_DNS_ERROR");
         }
      }
      else
      {
         break;
      }
   }

   if( !host )
   {
       throw DNSError( "Giving up, to many retries." );
   }

   site = host->h_name;

   for(cnt = 0; cnt < retries; cnt++)
   {
      host = gethostbyname(site.c_str());

      if( !host )
      {
         switch (h_errno)
         {
            case TRY_AGAIN:
               std::cerr << "err: TRY_AGAIN\n" << std::endl;
               if( microsleep( 100000 ) == -1)
               {
                  throw DNSError("Microsleep failure.");
			   }
               break;
            case NETDB_INTERNAL:
               throw DNSError("NETDB_INTERNAL");
            case HOST_NOT_FOUND:
               throw DNSError("HOST_NOT_FOUND");
            case NO_RECOVERY:
               throw DNSError("NO_RECOVERY");
            case NO_DATA:
               throw DNSError("NO_DATA");
            default:
               throw DNSError("UNRECOGNIZED_DNS_ERROR");
         }
      }
      else
      {
         break;
      }
   }

   if( !host )
   {
       throw DNSError( "Giving up, to many retries." );
   }

   for ( cnt = 0; host->h_addr_list[cnt]; cnt++ )
   {
      if (queried_address.s_addr == ((struct in_addr *) host->h_addr_list[cnt])->s_addr)
      {
         break;
      }
   }
   if (!host->h_addr_list[cnt])
   {
      throw DNSError("'Ip to name' lookup does not match 'name to ip' lookup, spoofing?");
   }

   return site;
}

childProcess::childProcess()
{
   request           =  0;
   pid               = -1;
   parent_pid        = -1;
   childReadFd       = -1;
   childWriteFd      = -1;
   parentReadFd      = -1;
   parentWriteFd     = -1;
   childReadStream   =  0;
   childWriteStream  =  0;
   parentReadStream  =  0;
   parentWriteStream =  0;
}

void childProcess::spawn()
{
   int to_child_fds[2];
   int from_child_fds[2];

   if ( pipe( to_child_fds   ) == -1 ||
        pipe( from_child_fds ) == -1 )
   {
      std::string errorStr = "Error in function pipe while doing spawn : ";
      throw FatalError( errorStr + strerror(errno));
   }

   parent_pid = getpid();
   pid = fork();

   switch( pid )
   {
      case -1: /* error */
         {
            std::string errorStr = "Error in function fork while doing spawn : ";
            throw FatalError( errorStr + strerror(errno));
         }
         break;

      case 0:  /* child */
         close( to_child_fds[write_fd] );
         close( from_child_fds[read_fd] );
         childReadFd      = to_child_fds[read_fd];
         childReadStreamBuffer = new __gnu_cxx::stdio_filebuf<char>( childReadFd, std::ios_base::in, 1 );
         childReadStream  = new std::ifstream;
         childReadStream->std::basic_ios<char>::rdbuf(childReadStreamBuffer);
         childWriteFd     = from_child_fds[write_fd];
         childWriteStreamBuffer = new __gnu_cxx::stdio_filebuf<char>( childWriteFd, std::ios_base::out, BUFSIZ );
         childWriteStream  = new std::ofstream;
         childWriteStream->std::basic_ios<char>::rdbuf(childWriteStreamBuffer);
         run();
         break;

      default: /* parent */
         close( to_child_fds[read_fd] );
         close( from_child_fds[write_fd] );
         parentReadFd      = from_child_fds[read_fd];
         parentReadStreamBuffer = new __gnu_cxx::stdio_filebuf<char>( parentReadFd, std::ios_base::in, 1 );
         parentReadStream  = new std::ifstream;
         parentReadStream->std::basic_ios<char>::rdbuf(parentReadStreamBuffer);
         parentWriteFd     = to_child_fds[write_fd];
         parentWriteStreamBuffer = new __gnu_cxx::stdio_filebuf<char>( parentWriteFd, std::ios_base::out, BUFSIZ );
         parentWriteStream  = new std::ofstream;
         parentWriteStream->std::basic_ios<char>::rdbuf(parentWriteStreamBuffer);
   }
}

void childProcess::run()
{
   fd_set          fds;
   struct timeval  timeout;
   std::string     address;
   std::string     result;
   bool            inputWaiting;

   while(true)
   {
      FD_ZERO(&fds);
      FD_SET(childReadFd, &fds);

      timeout.tv_sec = 1;
      timeout.tv_usec = 0;

      int sel_ret = select(FD_SETSIZE, &fds, 0, 0, &timeout);

      if( getppid() != parent_pid ) // parent dead
      {
         exit(1);
      }

      if( sel_ret == -1 ) // error
      {
         if( errno != EINTR )
         {
            std::cerr << "Error in child select: " << strerror(errno) << "." << std::endl;
         }
         continue;
      }

      if( sel_ret == 0 ) // timeout
      {
         continue;
      }

      std::getline( *childReadStream, address, '\0' );

      try
      {
         result = doLookup( address );
      }
      catch ( ParseError e )
      {
         std::cerr << "Parse error doing doLookup: " << e.what() << std::endl;
         result = address;
      }
      catch ( DNSError e )
      {
         std::cerr << "DNS error doing doLookup: " << e.what() << std::endl;
         result = address;
      }

      *childWriteStream << result << '\0';
      childWriteStream->flush();
   }
}

void parse_request( int talkerSocketFd )
{
   int charsRead=0;
   char charBuffer[512];
   std::string stringDataRead;
   static std::string buffer;
   dns_request *dnsRequest;

   charsRead = read( talkerSocketFd, charBuffer, sizeof(charBuffer) );
   if(charsRead == -1)
   {
      if( errno == EAGAIN || errno == EINTR )
      {
         return;
      }
      throw IoError( strerror(errno) );
   }
   if(charsRead == 0)
   {
      throw IoError( "read returned 0 bytes" );
   }
   stringDataRead.assign(charBuffer,charsRead);

   while(true)
   {
      std::string::size_type firstZero = stringDataRead.find( '\0' );

      if( firstZero == std::string::npos )
      {
         buffer.append( stringDataRead );
         return;
      }
      else
      {
         buffer.append( stringDataRead, 0, firstZero );
         stringDataRead.erase( 0, firstZero + 1 );
      }

      std::istringstream inputStream( buffer );

      TiXmlDocument inputDoc;
      inputStream >> inputDoc;
      buffer = "";

      std::cout << "Received query : " << inputDoc << std::endl;

      TiXmlElement  *rootElementPtr = inputDoc.FirstChildElement("query");
      if(rootElementPtr == 0)
      {
         throw FatalError("no query element");
      }

      const char* address_string = rootElementPtr->Attribute ( "address" );
      if(address_string == 0)
      {
         throw FatalError("no address attribute");
      }

      const char* id_string = rootElementPtr->Attribute ( "id" );
      if(id_string == 0)
      {
         throw FatalError("no id attribute");
      }

      dnsRequest = new dns_request;

      if(!dnsRequest)
      {
         throw FatalError("couldn't create new dns_request structure");
      }

      dnsRequest->id      = id_string;
      dnsRequest->ip_addr = address_string;
      requestsQueue.push(dnsRequest);
   }
}

void childProcess::takeOffQueue()
{
   if( requestsQueue.size() == 0)
   {
      return;
   }
   request = requestsQueue.front();
   requestsQueue.pop();

   *parentWriteStream << request->ip_addr << '\0' << std::flush;

   if( parentWriteStream->fail() )
   {
      std::cerr << "SocketStream is fail." << std::endl;
   }
   if( parentWriteStream->bad() )
   {
      std::cerr << "SocketStream is bad."<< std::endl;
   }
   if( parentWriteStream->eof() )
   {
      std::cerr << "SocketStream is eof." << std::endl;
   }
}

void childProcess::processResult( std::iostream *socketStream )
{
   std::string name;
   std::getline( *parentReadStream, name, '\0' );

   TiXmlDocument outputDoc;

   TiXmlElement rootNode("result");
   rootNode.SetAttribute ( "address" , request->ip_addr );
   rootNode.SetAttribute ( "name" , name );
   rootNode.SetAttribute ( "id" ,      request->id );
   outputDoc.InsertEndChild( rootNode );

   *socketStream << outputDoc << '\0' << std::flush;

   std::cout << "Sent result    : " << outputDoc << std::endl;

   delete request;
   request = 0;
}

int open_listen_socket(int port)
{
   struct sockaddr_in bind_addr;
   int on,size;
   int _socket;

   size = sizeof(struct sockaddr_in);

   bind_addr.sin_family=AF_INET;
   bind_addr.sin_addr.s_addr=INADDR_ANY;

   _socket = socket(AF_INET,SOCK_STREAM,0);
   if ( _socket == -1 )
   {
      printf("Can't open listen socket on port %d: %s\n" ,port, strerror(errno) );
      exit(1);
   }

   /* allow reboots on port even with TIME_WAITS */
   on = 1; // as in yes, do this
   setsockopt(_socket,SOL_SOCKET,SO_REUSEADDR,(char *)&on,sizeof(on));

   /* bind sockets and set up listen queues */
   bind_addr.sin_port=htons(port);
   if( bind(_socket,(struct sockaddr *)&bind_addr,size) == -1 )
   {
      printf("Can't bind socket on port %d: %s\n" ,port, strerror(errno) );
      exit(2);
   }

   if( listen(_socket,10) == -1 )
   {
      printf("Listen error on port %d: %s\n" ,port, strerror(errno) );
      exit(3);
   }
   /*
      "There may not always be a connection waiting after a SIGIO is delivered or select(2)
       or poll(2) return a readability event because the connection might have been removed
       by an asynchronous network error or another thread before accept is called. If this
       happens then the call will block waiting for the next connection to arrive. To ensure
       that accept never blocks, the passed socket s needs to have the O_NONBLOCK flag set."
   */
   // fcntl(_socket,F_SETFL,O_NONBLOCK);

   return _socket;
}

int accept_socket(int listenFd)
{
   int newSocket                = -1;

   /* Accept the first pending connection request on lsock, making a new
      socket with the same properties as lsock.*/
   newSocket = accept(listenFd,0,0);
   if( newSocket == -1 )
   {
      printf("Error in accept on fd %d: %s.\n",listenFd,strerror(errno));
      exit(4);
   }
   /* Set the socket to non blocking, cause we'll be using select as poller */
   fcntl(newSocket,F_SETFL,O_NONBLOCK);

   return newSocket;
}
