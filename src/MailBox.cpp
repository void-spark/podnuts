#include <sstream>
#include <string>
#include "XmlParsingError.h"
#include "libxml_glue.h"
#include "string_misc.h"
#include "MailBox.h"

MailMessage *MailBox::getMessageAndAdvance()
{
   return (MailMessage *)GenericMessageStore::getMessageAndAdvance();
}

MailMessage *MailBox::getMessage(unsigned int number)
{
   return (MailMessage *)GenericMessageStore::getMessage( number );
}

void MailBox::fromXml( XmlTextReader *reader )
{
   try 
   {
      _lastReadValue = stringToInt( reader->GetAttribute( "lastchecked" ) );
   }
   catch ( NoSuchAttribute &e )
   {
      pod_string error = "No 'lastchecked' attribute for element mailbox found in file ";
      error += _boardFileName;
      error += " .";
      throw XmlParsingError( e.getMessage() );
   }
   
   GenericMessageStore::fromXml( reader );
}

int MailBox::getLastReadValue()
{
   return _lastReadValue;
}

void MailBox::setLastReadValue(int val)
{
   _lastReadValue = val;
}

void MailBox::toXML( xmlTextWriterPtr ptr )
{
   pod_ostringstream lastchecked_str;
   lastchecked_str << _lastReadValue;

   xmlTextWriterStartElement( ptr, reinterpret_cast<const xmlChar *>(_rootElementName.c_str()) );
   xmlTextWriterWriteAttribute( ptr, reinterpret_cast<const xmlChar *>("lastchecked"), reinterpret_cast<const xmlChar *>( lastchecked_str.str().c_str() ) );

   messagesContentToXML( ptr );
   
   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("\n"));
   xmlTextWriterEndElement( ptr );
   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("\n"));
}

/*int MailBox::insertMessage(MailMessage *message)
{

   return GenericMessageStore::insertMessage( message );
}*/

int MailBox::insertNewMessage(pod_string sender, pod_string body, bool cc)
{

   pod_string wrappedBody = wordWrap( body, 80 );

   MailMessage *message = new MailMessage( sender,wrappedBody,cc );
   return GenericMessageStore::insertMessage( message );
}

GenericMessage *MailBox::createEmptyMessage()
{
   return new MailMessage();
}

