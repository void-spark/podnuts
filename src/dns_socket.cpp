#include <map>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "pod_alloc.h"
#include "pod_string.h"
#include "logging.h"
#include "string_misc.h"
#include "libxml_glue.h"
#include "socket_funcs.h"
#include "dns_socket.h"

class DNSSocket : public PlainSocket
{
   protected:
      int dnsQueryIdCounter;
      std::map<int, PlainSocket*, std::less<int>, pod_alloc< std::pair<const int, PlainSocket*> >::Type > socketsMap;

   public:
      DNSSocket();
      ~DNSSocket();

      virtual void initializeSocket();
      virtual int input_handler(pod_string input);
      virtual int disconnect_handler();

      int doQuery( PlainSocket* socket );
};

DNSSocket *newSocketObject = 0;

DNSSocket::DNSSocket() : dnsQueryIdCounter( 0 )
{
}

DNSSocket::~DNSSocket()
{
}

void DNSSocket::initializeSocket()
{
   return;
}

int DNSSocket::disconnect_handler()
{
   newSocketObject = 0;
   return 0;
}

int DNSSocket::input_handler(pod_string input)
{
   static pod_string remainder;
   pod_ostringstream input_str;
   input_str << remainder;

   for(unsigned int cnt=0;cnt < input.size();cnt++)
   {
      input_str << input[cnt];
      if( input[cnt] == 0 )
      {
         const pod_string &data = input_str.str();
         XmlTextReader * reader = new XmlTextReader( data.c_str(), strlen( data.c_str() ) );
         
         pod_string address_string;
         pod_string name_string;
         pod_string id_string;

         bool hasRead = reader->Read();
         while( hasRead )
         {
            int type = reader->NodeType();
            if( type == XML_READER_TYPE_ELEMENT || type == XML_READER_TYPE_END_ELEMENT )
            {
               pod_string name = reader->Name();

               if( type == XML_READER_TYPE_ELEMENT )
               {
                  if( name == "result" )
                  {
                     try
                     {
                        address_string = reader->GetAttribute( "address" );
                        name_string = reader->GetAttribute( "name" );
                        id_string = reader->GetAttribute( "id" );
                     }
                     catch (NoSuchAttribute &e)
                     {
                        write_syslogf( "No 'address', 'name', or 'id' attribute found for element 'result' in DNSSocket::input_handler().", true );
                        return 0;
                     }                           

                  }
               }
               else if( type == XML_READER_TYPE_END_ELEMENT )
               {
               }

            }

            hasRead = reader->Read();
         }
         
         delete reader;

         int id_val = stringToInt( id_string ) ;

         if( socketsMap.count(id_val) == 0 )
         {
            write_syslogf("Got unexpected result, id : %d, result : %s.\n", true, id_val , name_string.c_str() );
         }

         PlainSocket* socket = socketsMap[id_val];
         socketsMap.erase(id_val);

//         std::cout << address_string << ":" << name_string << ":" << id_val << ":" << newSocketObject->getFileDescriptor() << std::endl;
         socket->setPeerSite( name_string );
         socketInterface.addPlainSocket( socket );
         socket->initializeSocket();

         input_str.str("");
      }
   }
   remainder = input_str.str();
   return 0;
}

int dns_socket_init()
{
   newSocketObject = new DNSSocket();

   newSocketObject->connectIp("127.0.0.1",7320);

   socketInterface.addPlainSocket( (PlainSocket*)newSocketObject );

   newSocketObject->initializeSocket();

   return 0;
}

int query_ip_address_external( PlainSocket* socket )
{
   if(newSocketObject != 0)
   {
      return newSocketObject->doQuery( socket );
   }
   else
   {
      return -1;
   }
}

int DNSSocket::doQuery( PlainSocket* socket )
{
   pod_string address;
   int dnsQueryId = dnsQueryIdCounter++;
   socketsMap[dnsQueryId] = socket;

#warning moet ip_num gebrruiken ipv. fd, getpeername, blah, etc.
   struct sockaddr_in query_addr;
#ifndef __CYGWIN__
   unsigned int size = sizeof(struct sockaddr_in);
#else
   int size = sizeof(struct sockaddr_in);
#endif

   memset(&query_addr, 0, size);
   if (getpeername(socket->getFileDescriptor(), (struct sockaddr *)&query_addr, &size) == -1)
   {
      write_syslogf("Error in getpeername(%i) : %s.\n",true,socket->getFileDescriptor(),strerror(errno));
      return -1;
   }

   address = inet_ntoa(query_addr.sin_addr);

   xmlOutputBufferPtr buf;
   xmlCharEncodingHandlerPtr handler = NULL;
   buf = xmlAllocOutputBuffer(handler);

   xmlTextWriterPtr ptr = xmlNewTextWriter( buf );

   xmlTextWriterStartDocument( ptr, NULL, NULL, NULL );

   pod_ostringstream id_str;
   id_str << dnsQueryId;

   xmlTextWriterStartElement( ptr, reinterpret_cast<const xmlChar *>("query") );
   xmlTextWriterWriteAttribute( ptr, reinterpret_cast<const xmlChar *>("address"), reinterpret_cast<const xmlChar *>( address.c_str() ) );
   xmlTextWriterWriteAttribute( ptr, reinterpret_cast<const xmlChar *>("id"), reinterpret_cast<const xmlChar *>( id_str.str().c_str() ) );
   xmlTextWriterEndElement( ptr );

   xmlTextWriterEndDocument( ptr );

   xmlTextWriterFlush( ptr );


   pod_string outputDoc;

   if (buf->conv != NULL)
   {
      outputDoc.assign(reinterpret_cast<const char *>(buf->conv->content), buf->conv->use );
   }
   else
   {
      outputDoc.assign(reinterpret_cast<const char *>(buf->buffer->content), buf->buffer->use );
   }

   xmlFreeTextWriter( ptr );


   getQueueStream() << outputDoc << '\0';


   return 0;
}
