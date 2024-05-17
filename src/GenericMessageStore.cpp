#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "libxml_glue.h"
#include "IoError.h"
#include "XmlParsingError.h"
#include "GenericMessageStore.h"

pod_string GenericMessageStore::getFileName()
{
   return _boardFileName;
}

int GenericMessageStore::hasNextMessage()
{
   return currentMessage < messages.size();
}

void GenericMessageStore::advance()
{
   currentMessage++;
}

GenericMessage *GenericMessageStore::getMessageAndAdvance()
{
   GenericMessage *returnVal = messages[currentMessage];
   currentMessage++;
   return returnVal;
}

GenericMessage *GenericMessageStore::getMessage(unsigned int number)
{
   if( number >= messages.size())
   {
      return 0;
   }
   return messages[number];
}

void GenericMessageStore::open(int flags)
{
   /* check if file exist */
   struct stat statbuf;    // file info buffer
   int fileExists = (stat(_boardFileName.c_str(), &statbuf) == 0);

   if( !fileExists )
   {
      if( flags & GenericMessageStore::CREATE )
      {
         return;
      }
      else
      {
         pod_string error = "No such file : ";
         error += _boardFileName;
         error += " .";
         throw NoSuchFile(error);
      }
   }

   bool inRootElement = false;
   bool succes = false;
   bool hasRead = false;
   XmlTextReader *reader = 0;

   reader = new XmlTextReader( _boardFileName );   
      
   hasRead = reader->Read();
   while( hasRead )
   {
      int type = reader->NodeType();
      if( type == XML_READER_TYPE_ELEMENT || type == XML_READER_TYPE_END_ELEMENT )
      {
         pod_string name = reader->Name();

         if( type == XML_READER_TYPE_ELEMENT )
         {
            if( name == _rootElementName )
            {
               inRootElement = true;
               fromXml( reader );   
               continue;            
            }
         }
         else if( type == XML_READER_TYPE_END_ELEMENT )
         {
            if( name == _rootElementName && inRootElement == true )
            {
               inRootElement = false;
               succes = true;
            }
         }

      }
      
      hasRead = reader->Read();
   }
   
   delete reader;
      
   if( !succes )
   {
      pod_string error = "No '";
      error += _rootElementName;
      error += "' root element found in file ";
      error += _boardFileName;
      error += " .";
      throw XmlParsingError(error);
   }
}

void GenericMessageStore::fromXml( XmlTextReader *reader )
{
   int rootDepth  = reader->Depth();
   bool hasRead       = false;

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
            if( name == "message" )
            {
               GenericMessage *newMessage = createEmptyMessage();
               newMessage->fromXml( reader );
               messages.push_back(newMessage);
               continue;
            }
         }
         else if( type == XML_READER_TYPE_END_ELEMENT )
         {
            if( depth == rootDepth )
            {
               return;
            }
         }
      }

      hasRead = reader->Read();
   }
}

void GenericMessageStore::messagesContentToXML( xmlTextWriterPtr ptr )
{
   MessagesVector::iterator messageNode;
   
   for(messageNode=messages.begin();messageNode != messages.end();messageNode++)
   {
      (*messageNode)->toXML( ptr );
   }
}

void GenericMessageStore::toXML( xmlTextWriterPtr ptr )
{
   xmlTextWriterStartElement( ptr, reinterpret_cast<const xmlChar *>(_rootElementName.c_str()) );

   messagesContentToXML( ptr );
   
   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("\n"));
   xmlTextWriterEndElement( ptr );
   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("\n"));
}

int GenericMessageStore::save()
{
   pod_string filename;
   filename = _boardFileName.c_str();
      
   xmlTextWriterPtr ptr = xmlNewTextWriterFilename( filename.c_str(),  0 );      
   xmlTextWriterStartDocument( ptr, NULL, NULL, NULL );
   toXML(ptr);
   xmlTextWriterEndDocument( ptr );
   xmlFreeTextWriter( ptr );    

   return 0;
}

int GenericMessageStore::message_count()
{
   return messages.size();
}

int GenericMessageStore::insertMessage(GenericMessage *message)
{
   messages.push_back( message );

   return 0;
}

int GenericMessageStore::wipeAllMessages()
{
   if(unlink(_boardFileName.c_str()) == -1)
   {
   #warning throw error!
//      write_syslogf("Error in wipeAllMessages while unlinking file '%s' : '%s'.\n",TRUE,filename,strerror(errno));
      return -1;
   }

   MessagesVector::iterator messageNode;
   for(messageNode=messages.begin();messageNode != messages.end();messageNode++)
   {
      delete ( *messageNode );
   }
   messages.clear();

   return 0;
}

/*
 * Expects an ainteger array the size of the amount of messages, and deletes every message for
 * which the value in the array is non-zero.
 * returns the amount of actually deleted messages
 */
int GenericMessageStore::wipeMessages(int *deleteMask)
{
   int i=0;
   int cnt=0;
   int size = messages.size();

   for(i = (size-1); i >= 0; i--)
   {
      if(deleteMask[i])
      {
         cnt++;
         delete messages[i];
         messages.erase(messages.begin()+i);
      }
   }
   return cnt;
}

GenericMessageStore::~GenericMessageStore()
{
   MessagesVector::iterator messageNode;
   for(messageNode=messages.begin();messageNode != messages.end();messageNode++)
   {
      delete ( *messageNode );
   }
   messages.clear();
}

GenericMessage *GenericMessageStore::createEmptyMessage()
{
   return new GenericMessage();
}

