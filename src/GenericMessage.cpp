#include <string>
#include <sstream>
#include "XmlParsingError.h"
#include "libxml_glue.h"
#include "pod_string.h"
#include "string_misc.h"
#include "GenericMessage.h"

GenericMessage::GenericMessage() : _from(""), _body(""), _time(0)
{
}

GenericMessage::GenericMessage(pod_string from, pod_string body) : _from(from), _body(body), _time(time(0))
{
}

GenericMessage::GenericMessage(pod_string from, pod_string body, int time) : _from(from), _body(body), _time(time)
{
}

GenericMessage::~GenericMessage()
{
}

pod_string GenericMessage::getFrom()
{
   return _from;
}

pod_string GenericMessage::getBody()
{
   return _body;
}

pod_string GenericMessage::getDate()
{
   return longDateFromTime( _time );
}

int    GenericMessage::getTime()
{
   return _time;
}

void GenericMessage::fromXml( XmlTextReader *reader )
{
   _body = "";
   _from = "";
   _time = 0;
   int messageDepth  = reader->Depth();
   
   bool inBody = false;
   bool inHeader = false;
   bool hasRead = false;

   try
   {
      _time = stringToInt( reader->GetAttribute( "time" ) );      
   }
   catch (NoSuchAttribute &e)
   {
      throw XmlParsingError("No 'time' attribute for element 'message' found in function fromXml.");
   }                  
      
   hasRead = reader->Read();
   while( hasRead )
   {
      int type = reader->NodeType();
      int depth = reader->Depth();
      if( type == XML_READER_TYPE_ELEMENT || type == XML_READER_TYPE_END_ELEMENT )
      {
         pod_string name = reader->Name();

         if( type == XML_READER_TYPE_ELEMENT )
         {
            if( name == "header" )
            {
               inHeader = true;
            }
            else if( name == "body" )
            {
               inBody = true;
            }
            else if( inHeader == true )
            {
               parseHeaderElement( reader );
            }
         }
         else if( type == XML_READER_TYPE_END_ELEMENT )
         {
            if( depth == messageDepth )
            {
               return;
            }
            else if( name == "header" )
            {
               inHeader = false;
            }
            else if( name == "body" )
            {
               inBody = false;
            }
         }

      }
      else if( type == XML_READER_TYPE_TEXT ||
               type == XML_READER_TYPE_SIGNIFICANT_WHITESPACE )
      {
         if( inBody == true )
         {
            _body += reader->Value();
         }
      }
      
      hasRead = reader->Read();
   }

   return;
}

void GenericMessage::parseHeaderElement( XmlTextReader *reader )
{
   pod_string name = reader->Name();

   if( name == "from" )
   {
      try
      {
         _from = reader->GetAttribute( "name" );
      }
      catch (NoSuchAttribute &e)
      {
         throw XmlParsingError("No 'name' attribute for element 'from' found in function fromXml.");
      }               
   }
   
   return;
}


GenericMessage::GenericMessage( XmlTextReader *reader )  : _from(""), _body(""), _time(0)
{
   fromXml(reader);
}

void GenericMessage::headerContentToXML( xmlTextWriterPtr ptr )
{
   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("\n"));
   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("         "));
   xmlTextWriterStartElement( ptr, reinterpret_cast<const xmlChar *>("from") );
   xmlTextWriterWriteAttribute( ptr, reinterpret_cast<const xmlChar *>("name"), reinterpret_cast<const xmlChar *>( getFrom().c_str() ) );
   xmlTextWriterEndElement( ptr );

   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("\n"));
   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("         "));
   xmlTextWriterStartElement( ptr, reinterpret_cast<const xmlChar *>("date") );
   xmlTextWriterWriteAttribute( ptr, reinterpret_cast<const xmlChar *>("time"), reinterpret_cast<const xmlChar *>( getDate().c_str() ) );
   xmlTextWriterEndElement( ptr );
}

void GenericMessage::bodyContentToXML( xmlTextWriterPtr ptr )
{
   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>(getBody().c_str()));   
}

void GenericMessage::toXML( xmlTextWriterPtr ptr )
{
   pod_ostringstream timeStream;
   timeStream << getTime();

   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("\n"));
   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("   "));
   xmlTextWriterStartElement( ptr, reinterpret_cast<const xmlChar *>("message") );
   xmlTextWriterWriteAttribute( ptr, reinterpret_cast<const xmlChar *>("time"), reinterpret_cast<const xmlChar *>( timeStream.str().c_str() ) );

   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("\n"));
   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("      "));
   xmlTextWriterStartElement( ptr, reinterpret_cast<const xmlChar *>("header") );

   headerContentToXML( ptr );
      
   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("\n"));
   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("      "));
   xmlTextWriterEndElement( ptr );
   
   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("\n"));
   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("      "));
   xmlTextWriterStartElement( ptr, reinterpret_cast<const xmlChar *>("body") );
   bodyContentToXML( ptr );
   xmlTextWriterEndElement( ptr );
         
   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("\n"));
   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("   "));
   xmlTextWriterEndElement( ptr );
}

