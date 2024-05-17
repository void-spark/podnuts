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

#include "../tinyxml/tinyxml.h"
#include "errorClasses.h"

main( int argc, char *argv[] )
{
   struct sockaddr_in connect_addr;
   int newSocket    = -1;
   int size         = 0;
   std::filebuf   *socketStreamBuffer = 0;
   std::iostream  *socketStream       = 0;

   size = sizeof(struct sockaddr_in);

   newSocket = socket(AF_INET,SOCK_STREAM,0);
   if ( newSocket == -1 )
   {
      abort();
   }

   memset(&connect_addr, 0, sizeof(sockaddr_in));
   connect_addr.sin_family = AF_INET;
   connect_addr.sin_port   = htons (7320);
   inet_aton("127.0.0.1", (in_addr*)&(connect_addr.sin_addr.s_addr) );

   while( connect(newSocket,(struct sockaddr *)&connect_addr,size) == -1 );

   #warning, last param is a buffer size that used to be 1, newest libstdc++ that does odd things tho , misses each second char
   socketStreamBuffer = new __gnu_cxx::stdio_filebuf<char>( newSocket, std::ios::in|std::ios::out, true, 512 );
   socketStream  = new std::iostream( socketStreamBuffer );

   TiXmlDocument outputDoc;

   std::string address = "172.0.0.1";
   int dnsQueryId = 1;

   TiXmlElement rootNode("query");
   rootNode.SetAttribute ( "address" , address.c_str() );
   std::ostringstream id_str;
   id_str << dnsQueryId;
   rootNode.SetAttribute ( "id" , id_str.str().c_str() );
   outputDoc.InsertEndChild( rootNode );
//   *socketStream << outputDoc << '\0';
//   socketStream->flush();
   *socketStream << "<query address=\"172.0.0.1\" id=\"1\" />" << '\0';
   socketStream->flush();
//   write( newSocket, "<query address=\"172.0.0.1\" id=\"1\" />\0", strlen("<query address=\"172.0.0.1\" id=\"1\" />\0") );
   
   std::cout << "Sent query    : " << outputDoc << std::endl;

   int charsRead=0;
   char charBuffer[512];

   while(true)
   {
     charsRead = read( newSocket, charBuffer, sizeof(charBuffer) );
     if( charsRead == -1)
     {
       break;
     }
     std::string feedback( charBuffer, charsRead );
     std::cout << feedback << std::endl << std::flush;
   }

   
   
}