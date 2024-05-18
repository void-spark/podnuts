#include "pod_string.h"
#include "string_misc.h"
#include "logging.h"
#include "socket_funcs.h"
#include "rfc854.h"
#include "TelnetSender.h"
#include "TelnetHandler.h"
#include "libxml_glue.h"
#include "QMethod.h"
#include "TelnetSocket.h"

#define EOR             239     /* end of record (transparent mode) */

#define TELOPT_ECHO     1       /* echo */
#define TELOPT_SGA      3       /* suppress go ahead */
#define TELOPT_EOR      25      /* end or record */
#define TELOPT_NAWS     31      /* window size */

TelnetSocket::TelnetSocket( int fileDescriptor, TelnetHandler *telnetHandler ):
PlainSocket( fileDescriptor ), telnetHandler(telnetHandler), qMethod(telnetHandler, this )
{
   debug = false;
}

TelnetSocket::TelnetSocket( int fileDescriptor , pod_string dns_name, TelnetHandler *telnetHandler ) :
PlainSocket( fileDescriptor, dns_name  ), telnetHandler(telnetHandler), qMethod(telnetHandler, this )
{
   debug = false;
}

TelnetSocket::~TelnetSocket()
{
   delete telnetHandler;
}

void TelnetSocket::initializeSocket()
{
   telnetHandler->handleConnect();
}

int TelnetSocket::disconnect_handler()
{
   telnetHandler->handleDisconnect();
   return 0;
}

bool compareNextChar( pod_string buffer, pod_string::size_type pos, unsigned char character )
{
   if( pos + 1 < buffer.size() )
   {
      if( (unsigned char)buffer[pos+1] == character )
      {
         return true;
      }
   }
   return false;
}

int TelnetSocket::input_handler(pod_string input)
{
   pod_string output;
   unsigned int in_pos = 0;
   pod_string::size_type size = input.size();
   int subnegotiation = -1;
   pod_string subnegotiationString;

   if( debug )
   {
      logStream << setLogfile( DEBUGLOG ) << getPeerSite() << ":" << getPeerPort() << " : "
                << "TelnetSocket::input_handler() called with chars : ";
      for(unsigned int cnt=0;cnt < size;cnt++)
      {
         unsigned char curChar = input[cnt];
         logStream << "[" << (unsigned int)curChar << "]";
      }
      logStream << std::endl << pod_send;
   }

   while( in_pos < size )
   {
      unsigned char curChar = input[in_pos];

      bool isIAC = ( curChar == RFC854::IAC );

      if( isIAC && ( in_pos + 1 < size ) && (unsigned char)input[in_pos+1] == RFC854::IAC )
      {
         isIAC = false;
         in_pos++;
         curChar = input[in_pos];
      }

      if( subnegotiation != -1 && !isIAC )
      {
         subnegotiationString += curChar;
         in_pos++;

         continue;
      }

      if( RFC854::isUSASCIIGraphic( curChar ) )
      {
         if( debug )
         {
            logStream << setLogfile( DEBUGLOG ) << getPeerSite() << ":" << getPeerPort() << " : "
                      << "TelnetSocket::input_handler() found USASCIIGraphic:" << (unsigned int)curChar << std::endl
                      << pod_send;
         }
         output.push_back( input[in_pos] );
      }
      else if ( RFC854::isUSASCIIControl( curChar ) )
      {
         if( debug )
         {
            logStream << setLogfile( DEBUGLOG ) << getPeerSite() << ":" << getPeerPort() << " : "
                      << "TelnetSocket::input_handler() found USASCIIControl:" << (unsigned int)curChar << std::endl
                      << pod_send;
         }
         
         if( curChar == RFC854::NUL )         
         {
           // NOOP
         }
         else if( curChar == RFC854::LF )
         {
            if( compareNextChar( input, in_pos, RFC854::CR ) )
            {
               // [LF][CR]
               output.push_back( '\n' );
               in_pos += 2;
               continue;
            }
            
            // [LF]
            output.push_back( '\n' );
         }
         else if( curChar == RFC854::CR )
         {
            if( compareNextChar( input, in_pos, RFC854::LF ) )
            {
               // [CR][LF]
               output.push_back( '\n' );
               in_pos += 2;
               continue;
            }
            else if( compareNextChar( input, in_pos, RFC854::NUL ) )
            {
               // [CR][NUL]
               output.push_back( '\n' );
               in_pos += 2;
               continue;
            }

            // [CR]
            output.push_back( '\n' );
         }
         #warning I consider since these are all telnet commands to give them each a handler function         
         else if( curChar == RFC854::BEL )
         {
            output.push_back( RFC854::BEL );
         }
         else if( curChar == RFC854::BS )
         {
            output.push_back( RFC854::BS );
         }
         else if( curChar == RFC854::HT )
         {
            output.push_back( RFC854::HT );
         }
         else if( curChar == RFC854::VT )
         {
            output.push_back( RFC854::VT );
         }
         else if( curChar == RFC854::FF )
         {
            output.push_back( RFC854::FF );
         }
         else
         {
            output.push_back( input[in_pos] );
         }
      }
      else if ( RFC854::isUSASCIIUncovered( curChar ) )
      {
         if( debug )
         {
            logStream << setLogfile( DEBUGLOG ) << getPeerSite() << ":" << getPeerPort() << " : "
                      << "TelnetSocket::input_handler() found USASCIIUncovered:" << (unsigned int)curChar << std::endl
                      << pod_send;
         }
         if ( isIAC && in_pos + 1 < size)
         {
            if( debug )
            {
               logStream << setLogfile( DEBUGLOG ) << getPeerSite() << ":" << getPeerPort() << " : "
                         << "TelnetSocket::input_handler() found IAC" << std::endl
                         << pod_send;
            }
            
            if( output.size() > 0 )
            {
               telnetHandler->handleText( output );
               output = "";
            }

            unsigned char cmdChar = (unsigned char)input[in_pos+1];
            if( cmdChar == RFC854::WILL && in_pos + 2 < size )
            {
               if( debug )
               {
                  logStream << setLogfile( DEBUGLOG ) << getPeerSite() << ":" << getPeerPort() << " : "
                            << "TelnetSocket::input_handler() found WILL:" << (unsigned int)input[in_pos+2] << std::endl
                            << pod_send;
               }
               qMethod.parseWill( (unsigned char)input[in_pos+2] );
               in_pos += 3;
               continue;
            }
            else if( cmdChar == RFC854::WONT && in_pos + 2 < size )
            {
               if( debug )
               {
                  logStream << setLogfile( DEBUGLOG ) << getPeerSite() << ":" << getPeerPort() << " : "
                            << "TelnetSocket::input_handler() found WONT:" << (unsigned int)input[in_pos+2] << std::endl
                            << pod_send;
               }
               qMethod.parseWont( (unsigned char)input[in_pos+2]);
               in_pos += 3;
               continue;
            }
            else if( cmdChar == RFC854::DO && in_pos + 2 < size )
            {
               if( debug )
               {
                  logStream << setLogfile( DEBUGLOG ) << getPeerSite() << ":" << getPeerPort() << " : "
                            << "TelnetSocket::input_handler() found DO:" << (unsigned int)input[in_pos+2] << std::endl
                            << pod_send;
               }
               qMethod.parseDo( (unsigned char)input[in_pos+2] );
               in_pos += 3;
               continue;
            }
            else if( cmdChar == RFC854::DONT && in_pos + 2 < size )
            {
               if( debug )
               {
                  logStream << setLogfile( DEBUGLOG ) << getPeerSite() << ":" << getPeerPort() << " : "
                            << "TelnetSocket::input_handler() found DONT:" << (unsigned int)input[in_pos+2] << std::endl
                            << pod_send;
               }
               qMethod.parseDont( (unsigned char)input[in_pos+2]);
               in_pos += 3;
               continue;
            }
            else if ( cmdChar == RFC854::SB && in_pos + 2 < size)
            {
               if( debug )
               {
                  logStream << setLogfile( DEBUGLOG ) << getPeerSite() << ":" << getPeerPort() << " : "
                            << "TelnetSocket::input_handler() found SB:" << (unsigned int)input[in_pos+2] << std::endl
                            << pod_send;
               }
               unsigned char optionChar = (unsigned char)input[in_pos+2];
               subnegotiation = optionChar;
               subnegotiationString = "";
               in_pos += 3;
               continue;
            }
            else if ( cmdChar == RFC854::SE )
            {
               if( debug )
               {
                  logStream << setLogfile( DEBUGLOG ) << getPeerSite() << ":" << getPeerPort() << " : "
                            << "TelnetSocket::input_handler() found SE" << std::endl;
                            
                  
                  logStream << "TelnetSocket::input_handler() found subnegotation with chars : ";
                  for( unsigned int cnt = 0; cnt < subnegotiationString.size() ; cnt++)
                  {
                     unsigned char curSubNegChar = (unsigned char)subnegotiationString[cnt];
                     logStream << "[" << (unsigned int)curSubNegChar << "]";
                  }
                  logStream << std::endl << pod_send;
               }

               telnetHandler->handleSubNegotiation( (unsigned char)subnegotiation, subnegotiationString );

               subnegotiation = -1;
               subnegotiationString = "";
               in_pos += 2;
               continue;
            }
            else
            {
               if( debug )
               {
                  logStream << setLogfile( DEBUGLOG ) << getPeerSite() << ":" << getPeerPort() << " : "
                            << "TelnetSocket::input_handler() found unknown cmdChar:" << (unsigned int)cmdChar << std::endl
                            << pod_send;
               }
               in_pos += 2;
               continue;
            }
         }
         else
         {
            output.push_back( input[in_pos] );
         }

      }
      in_pos++;
   }

   telnetHandler->handleText( output );

   return 0;
}

bool TelnetSocket::isUsEnabled( unsigned char code )
{
   return qMethod.isUsEnabled( code );
}

bool TelnetSocket::isHimEnabled( unsigned char code )
{
   return qMethod.isHimEnabled( code );
}

void TelnetSocket::requestHimEnable( unsigned char code )
{
   return qMethod.requestHimEnable( code );
}

void TelnetSocket::requestUsEnable( unsigned char code )
{
   return qMethod.requestUsEnable( code );
}

void TelnetSocket::requestHimDisable( unsigned char code )
{
   return qMethod.requestHimDisable( code );
}

void TelnetSocket::requestUsDisable( unsigned char code )
{
   return qMethod.requestUsDisable( code );
}

pod_string TELNETCmdString( unsigned char cmd )
{
   pod_string result;
   if( cmd == RFC854::WILL )
   {
      result = "WILL";
   }
   else if( cmd == RFC854::WONT )
   {
      result = "WONT";
   }
   else if( cmd == RFC854::DO )
   {
      result = "DO";
   }
   else if( cmd == RFC854::DONT )
   {
      result = "DONT";
   }
   else if( cmd == EOR )
   {
      result = "EOR";
   }
   else
   {
      result = intToString(cmd);
   }

   return result;
}

pod_string TELOPTString( unsigned char telopt )
{
   pod_string result;
   if( telopt == TELOPT_EOR )
   {
      result = "TELOPT_EOR";
   }
   else if( telopt == TELOPT_SGA )
   {
      result = "TELOPT_SGA";
   }   
   else if( telopt == TELOPT_ECHO )
   {
      result = "TELOPT_ECHO";
   }
   else if( telopt == TELOPT_NAWS )
   {
      result = "TELOPT_NAWS";
   }
   else
   {
      result = intToString(telopt);
   }

   return result;
}

void TelnetSocket::sendCommand( unsigned char cmd )
{
   if( debug )
   {
      logStream << setLogfile( DEBUGLOG ) << getPeerSite() << ":" << getPeerPort() << " : "
                << "TelnetSocket::sendCommand() Sent : [IAC][" << TELNETCmdString(cmd) << "]"  << std::endl
                << pod_send;
   }
   
   std::ostream & outputStream = getQueueStream();
   outputStream << RFC854::IAC << cmd;
}

void TelnetSocket::sendCommand( unsigned char cmd1, unsigned char cmd2 )
{
   if( debug )
   {
      logStream << setLogfile( DEBUGLOG ) << getPeerSite() << ":" << getPeerPort() << " : "
                << "TelnetSocket::sendCommand() Sent : [IAC][" << TELNETCmdString(cmd1) << "][" << TELOPTString(cmd2) << "]" << std::endl
                << pod_send;
   }
   
   std::ostream & outputStream = getQueueStream();
   outputStream << RFC854::IAC << cmd1 << cmd2;
}

TelnetHandler* TelnetSocket::getTelnetHandler()
{
   return telnetHandler;
}

void TelnetSocket::toXML( xmlTextWriterPtr ptr )
{
   qMethod.toXML( ptr );
}

void TelnetSocket::fromXML( XmlTextReader * reader )
{
   qMethod.fromXML( reader );
}


