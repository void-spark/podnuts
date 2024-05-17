#include "XmlParsingError.h"
#include "libxml_glue.h"
#include "GenericMessage.h"
#include "MailMessage.h"

MailMessage::MailMessage() : GenericMessage(), _cc( false )
{      
}

MailMessage::MailMessage(pod_string from, pod_string body, bool cc) : GenericMessage(from, body), _cc( cc )
{
}

MailMessage::MailMessage(pod_string from, pod_string body, int time, bool cc) : GenericMessage(from, body, time), _cc( cc )
{
}

MailMessage::~MailMessage()
{
}

MailMessage::MailMessage( XmlTextReader *reader ) : _cc( false )
{
   fromXml( reader );
}

bool   MailMessage::getIsCC()
{
   return _cc;
}

void MailMessage::parseHeaderElement( XmlTextReader *reader )
{
   GenericMessage::parseHeaderElement( reader );
   
   pod_string name = reader->Name();

   if( name == "cc" )
   {
      _cc = true;
   }
   
   return;
}

void MailMessage::headerContentToXML( xmlTextWriterPtr ptr )
{
   GenericMessage::headerContentToXML( ptr );
   if(_cc)
   {
      xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("\n"));
      xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("         "));
      xmlTextWriterStartElement( ptr, reinterpret_cast<const xmlChar *>("cc") );
      xmlTextWriterEndElement( ptr );
   }
}
