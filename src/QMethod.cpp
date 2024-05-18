#include <iostream>
#include "pod_string.h"
#include "string_misc.h"
#include "rfc854.h"
#include "TelnetSender.h"
#include "TelnetHandler.h"
#include "libxml_glue.h"
#include "QMethod.h"

QMethodData::QMethodData()
{
   us   = NO;
   usq  = EMPTY;
   him  = NO;
   himq = EMPTY;
}

QMethod::QMethod( TelnetHandler *telnetHandler, TelnetSender *telnetSender )
{
   this->telnetHandler = telnetHandler;
   this->telnetSender  = telnetSender;
   
   debug = false;
}

QMethod::~QMethod()
{
}

/*
 *   An option is enabled if and only if its state is YES.  Note that
 *   us/usq and him/himq could be combined into two six-choice states.
 */
bool QMethod::isUsEnabled( unsigned char code )
{
   return data[code].us == QMethodData::YES;
}

bool QMethod::isHimEnabled( unsigned char code )
{
   return data[code].him == QMethodData::YES;
}

/*
 *     "Error" below means that producing diagnostic information may be a
 *    good idea, though it isn't required.
 *
 *    Upon receipt of WILL, we choose based upon him and himq:
 *       NO            If we agree that he should enable, him=YES, send
 *                     DO; otherwise, send DONT.
 *       YES           Ignore.
 *       WANTNO  EMPTY Error: DONT answered by WILL. him=NO.
 *            OPPOSITE Error: DONT answered by WILL. him=YES*,
 *                     himq=EMPTY.
 *       WANTYES EMPTY him=YES.
 *            OPPOSITE him=WANTNO, himq=EMPTY, send DONT.
 *
 *    * This behavior is debatable; DONT will never be answered by WILL
 *      over a reliable connection between TELNETs compliant with this
 *      RFC, so this was chosen (1) not to generate further messages,
 *      because if we know we're dealing with a noncompliant TELNET we
 *      shouldn't trust it to be sensible; (2) to empty the queue
 *      sensibly.
 */

void QMethod::parseWill( unsigned char code )
{
   if( debug )
   {
      std::cout << "QMethod::parseWill( " << (unsigned int)code << " )" << std::endl;
   }
   parseEnable( true, code );
}

void QMethod::parseDo( unsigned char code )
{
   if( debug )
   {
      std::cout << "QMethod::parseDo( " << (unsigned int)code << " )" << std::endl;
   }
   parseEnable( false, code );
}

void QMethod::parseEnable( bool isWill, unsigned char code )
{
   int & status              = isWill ? data[code].him : data[code].us;
   int & q                   = isWill ? data[code].himq : data[code].usq;
   unsigned char acceptCmd   = isWill ? RFC854::DO   : RFC854::WILL;
   unsigned char declineCmd  = isWill ? RFC854::DONT : RFC854::WONT;
   bool mayEnable            = isWill ? telnetHandler->mayHeEnable( code ) : telnetHandler->mayWeEnable( code );

   if( debug )
   {
      std::cout << "QMethod::parseEnable( " << isWill << ", " << (unsigned int)code << " )" << std::endl;
   }

   switch( status )
   {
      case QMethodData::NO:
            if( debug )
            {
               std::cout << "status = NO" << std::endl;
            }
            if( mayEnable )
            {
               if( debug )
               {
                  std::cout << "mayEnable = true" << std::endl;
               }
               status = QMethodData::YES;
               telnetSender->sendCommand( acceptCmd, code );
               handleEnable( isWill, code );
            }
            else
            {
               if( debug )
               {
                  std::cout << "mayEnable = false" << std::endl;
               }
               telnetSender->sendCommand( declineCmd, code );
            }
         break;
      case QMethodData::YES:
            if( debug )
            {
               std::cout << "status = YES" << std::endl;
            }
         break;
      case QMethodData::WANTNO:
            if( debug )
            {
               std::cout << "status = WANTNO" << std::endl;
            }
            std::cerr << "Error: Disable answered by Enable." << std::endl;
            if( q == QMethodData::EMPTY )
            {
               status = QMethodData::NO;
            }
            else
            {
               status = QMethodData::YES;
               q = QMethodData::EMPTY;
               handleEnable( isWill, code );
            }
         break;
      case QMethodData::WANTYES:
            if( debug )
            {
               std::cout << "status = WANTYES" << std::endl;
            }
            if( q == QMethodData::EMPTY )
            {
               if( debug )
               {
                  std::cout << "q = EMPTY" << std::endl;
               }
               status = QMethodData::YES;
               handleEnable( isWill, code );
            }
            else
            {
               if( debug )
               {
                  std::cout << "q != EMPTY" << std::endl;
               }
               status = QMethodData::WANTNO;
               q = QMethodData::EMPTY;
               telnetSender->sendCommand( declineCmd, code );
            }
         break;
      default:
         std::cerr << "Unexpected state." << std::endl;
   }
}

/*    Upon receipt of WONT, we choose based upon him and himq:
 *        NO            Ignore.
 *        YES           him=NO, send DONT.
 *        WANTNO  EMPTY him=NO.
 *             OPPOSITE him=WANTYES, himq=NONE, send DO.
 *        WANTYES EMPTY him=NO.*
 *             OPPOSITE him=NO, himq=NONE.**
 *
 *     * Here is the only spot a length-two queue could be useful; after
 *       a WILL negotiation was refused, a queue of WONT WILL would mean
 *       to request the option again. This seems of too little utility
 *       and too much potential waste; there is little chance that the
 *       other side will change its mind immediately.
 *
 *     ** Here we don't have to generate another request because we've
 *        been "refused into" the correct state anyway.
 */
void QMethod::parseWont( unsigned char code )
{
   if( debug )
   {
      std::cout << "QMethod::parseWont( " << (unsigned int)code << " )" << std::endl;
   }
   parseDisable( true, code );
}

void QMethod::parseDont( unsigned char code )
{
   if( debug )
   {
      std::cout << "QMethod::parseDont( " << (unsigned int)code << " )" << std::endl;
   }
   parseDisable( false, code );
}

void QMethod::parseDisable( bool isWont, unsigned char code )
{
   int & status              = isWont ? data[code].him : data[code].us;
   int & q                   = isWont ? data[code].himq : data[code].usq;
   unsigned char acceptCmd   = isWont ? RFC854::DO   : RFC854::WILL;
   unsigned char declineCmd  = isWont ? RFC854::DONT : RFC854::WONT;

   switch( status )
   {
      case QMethodData::NO:
         break;
      case QMethodData::YES:
            status = QMethodData::NO;
            telnetSender->sendCommand( declineCmd, code );
            handleDisable( isWont, code );
         break;
      case QMethodData::WANTNO:
            if( q == QMethodData::EMPTY )
            {
               status = QMethodData::NO;
            }
            else
            {
               status = QMethodData::WANTYES;
               q = QMethodData::EMPTY;
               telnetSender->sendCommand( acceptCmd, code );
            }
         break;
      case QMethodData::WANTYES:
            if( q == QMethodData::EMPTY )
            {
               status = QMethodData::NO;
            }
            else
            {
               status = QMethodData::NO;
               q = QMethodData::EMPTY;
            }
         break;
      default:
         std::cerr << "Unexpected state." << std::endl;
   }
}

/*
 *     If we decide to ask him to enable:
 *        NO            him=WANTYES, send DO.
 *        YES           Error: Already enabled.
 *        WANTNO  EMPTY If we are queueing requests, himq=OPPOSITE;
 *                      otherwise, Error: Cannot initiate new request
 *                      in the middle of negotiation.
 *            OPPOSITE Error: Already queued an enable request.
 *       WANTYES EMPTY Error: Already negotiating for enable.
 *            OPPOSITE himq=EMPTY.
 */
void QMethod::requestHimEnable( unsigned char code )
{
   if( debug )
   {
      std::cout << "QMethod::requestHimEnable( " << (unsigned int)code << " )" << std::endl;
   }
   requestEnable( true, code );
}

void QMethod::requestUsEnable( unsigned char code )
{
   if( debug )
   {
      std::cout << "QMethod::requestUsEnable( " << (unsigned int)code << " )" << std::endl;
   }
   requestEnable( false, code );
}

void QMethod::requestEnable( bool isHim, unsigned char code )
{
   int & status              = isHim ? data[code].him : data[code].us;
   int & q                   = isHim ? data[code].himq : data[code].usq;
   unsigned char acceptCmd   = isHim ? RFC854::DO   : RFC854::WILL;
   unsigned char declineCmd  = isHim ? RFC854::DONT : RFC854::WONT;

   switch( status )
   {
      case QMethodData::NO:
            status = QMethodData::WANTYES;
            telnetSender->sendCommand( acceptCmd, code );
         break;
      case QMethodData::YES:
            std::cerr << "Error: Already enabled." << std::endl;
         break;
      case QMethodData::WANTNO:
            if( q == QMethodData::EMPTY )
            {
               q = QMethodData::OPPOSITE;
            }
            else
            {
               std::cerr << "Error: Already queued an enable request." << std::endl;
            }
         break;
      case QMethodData::WANTYES:
            if( q == QMethodData::EMPTY )
            {
               std::cerr << "Error: Already negotiating for enable." << std::endl;
            }
            else
            {
               q = QMethodData::EMPTY;
            }
         break;
      default:
         std::cerr << "Unexpected state." << std::endl;
   }
}

/*
 *    If we decide to ask him to disable:
 *       NO            Error: Already disabled.
 *       YES           him=WANTNO, send DONT.
 *       WANTNO  EMPTY Error: Already negotiating for disable.
 *            OPPOSITE himq=EMPTY.
 *       WANTYES EMPTY If we are queueing requests, himq=OPPOSITE;
 *                     otherwise, Error: Cannot initiate new request
 *                     in the middle of negotiation.
 *            OPPOSITE Error: Already queued a disable request.
 */
void QMethod::requestHimDisable( unsigned char code )
{
   if( debug )
   {
      std::cout << "QMethod::requestHimDisable( " << (unsigned int)code << " )" << std::endl;
   }
   requestDisable( true, code );
}

void QMethod::requestUsDisable( unsigned char code )
{
   if( debug )
   {
      std::cout << "QMethod::requestUsDisable( " << (unsigned int)code << " )" << std::endl;
   }
   requestDisable( false, code );
}

void QMethod::requestDisable( bool isHim, unsigned char code )
{
   int & status              = isHim ? data[code].him : data[code].us;
   int & q                   = isHim ? data[code].himq : data[code].usq;
   unsigned char acceptCmd   = isHim ? RFC854::DO   : RFC854::WILL;
   unsigned char declineCmd  = isHim ? RFC854::DONT : RFC854::WONT;

   switch( status )
   {
      case QMethodData::NO:
            std::cerr << "Error: Already disabled." << std::endl;
         break;
      case QMethodData::YES:
            status = QMethodData::WANTNO;
            telnetSender->sendCommand( declineCmd, code );
            handleDisable( isHim, code );
         break;
      case QMethodData::WANTNO:
            if( q == QMethodData::EMPTY )
            {
               std::cerr << "Error: Already negotiating for disable." << std::endl;
            }
            else
            {
               q = QMethodData::EMPTY;
            }
         break;
      case QMethodData::WANTYES:
            if( q == QMethodData::EMPTY )
            {
               q = QMethodData::OPPOSITE;
            }
            else
            {
               std::cerr << "Error: Already queued a disable request." << std::endl;
            }
         break;
      default:
         std::cerr << "Unexpected state." << std::endl;
   }
}

void QMethod::handleEnable( bool isHim, unsigned char code )
{
   if( debug )
   {
      std::cout << "handleEnable:" << isHim << ", " << (unsigned int)code << std::endl;
   }
   if(isHim)
   {
      telnetHandler->doThemEnable( code );
   }
   else
   {
      telnetHandler->doWeEnable( code );
   }
}

void QMethod::handleDisable( bool isHim, unsigned char code )
{
   if( debug )
   {
      std::cout << "handleDisable:" << isHim << ", " << (unsigned int)code << std::endl;
   }
   if(isHim)
   {
      telnetHandler->doThemDisable( code );
   }
   else
   {
      telnetHandler->doWeDisable( code );
   }
}

void QMethod::toXML( xmlTextWriterPtr ptr )
{
   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("\n"));
   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("      "));
   xmlTextWriterStartElement( ptr, reinterpret_cast<const xmlChar *>("array") );
   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("\n"));
   
   pod_string teloptName = "telopt";   
   for( int i = 0; i < 256; i++)
   {
      xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("         "));
      xmlTextWriterStartElement( ptr, reinterpret_cast<const xmlChar *>( (teloptName + intToString( i )).c_str() ) );

      xmlTextWriterWriteAttribute( ptr, reinterpret_cast<const xmlChar *>("us"), reinterpret_cast<const xmlChar *>( intToString( data[i].us ).c_str() ) );
      xmlTextWriterWriteAttribute( ptr, reinterpret_cast<const xmlChar *>("usq"), reinterpret_cast<const xmlChar *>( intToString( data[i].usq ).c_str() ) );
      xmlTextWriterWriteAttribute( ptr, reinterpret_cast<const xmlChar *>("him"), reinterpret_cast<const xmlChar *>( intToString( data[i].him ).c_str() ) );
      xmlTextWriterWriteAttribute( ptr, reinterpret_cast<const xmlChar *>("himq"), reinterpret_cast<const xmlChar *>( intToString( data[i].himq ).c_str() ) );
   
      xmlTextWriterEndElement( ptr );
      xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("\n"));
   }
   
   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("      "));
   xmlTextWriterEndElement( ptr );
   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("\n"));
   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("   "));
}

void QMethod::fromXML( XmlTextReader * reader )
{
   enum
   {
      UNSET,
      ARRAY,
      VAL,
      DONE
   };
   
   bool            hasRead        = true;
   pod_string      name;   
   bool            inItemElement  = false;
   int             ourItemDepth   = -1;
   int             state          = UNSET;
   int             index          = -1;
   int             type           = 0;
   int             depth          = 0;

   while( hasRead )
   {
      type  = reader->NodeType();
      depth = reader->Depth();
      int subDepth = depth - ourItemDepth;
      
      if( type == XML_READER_TYPE_ELEMENT || type == XML_READER_TYPE_END_ELEMENT )
      {
         name = reader->Name();

         if( type == XML_READER_TYPE_ELEMENT )
         {
            if( inItemElement == false )
            {
               if( name == "item" )
               {
                  inItemElement = true;
                  ourItemDepth = depth;
               }
            }            
            else if( subDepth == 1 )
            {
               if( name == "array" && state == UNSET )
               {
                  state = ARRAY;
               }
            }
            else if( subDepth == 2 )
            {
               if( name.substr(0,6) == "telopt" && state == ARRAY )
               {
                  state = VAL;
                  index = stringToInt( name.substr( 6 ) );
                  
                  try
                  {                  
                     data[index].him  = stringToInt( reader->GetAttribute( "him" ));
                     data[index].himq = stringToInt( reader->GetAttribute( "himq" ) );
                     data[index].us   = stringToInt( reader->GetAttribute( "us" ) );
                     data[index].usq  = stringToInt( reader->GetAttribute( "usq" ) );
                  }
                  catch (NoSuchAttribute &e)
                  {
                     std::cerr << "QMethod::fromXML() - Missing attribute!" << std::endl;
                     abort();
                  }                              
               }
            }
         }
         if( ( type == XML_READER_TYPE_END_ELEMENT || reader->IsEmptyElement() ) && inItemElement )
         {
            if( depth == ourItemDepth && name == "item" )
            {
               inItemElement = false;
               ourItemDepth = -1;
               return;
            }
            else if( subDepth == 1 )
            {
               if( name == "array" && state == ARRAY )
               {
                  state = DONE;
               }
            }
            else if( subDepth == 2 )
            {
               if( name.substr(0,6) == "telopt" && state == VAL )
               {
                  state = ARRAY;
               }
            }
         }
      }

      hasRead = reader->Read();
   }
}

