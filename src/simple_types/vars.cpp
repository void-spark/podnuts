#include <string>

#include "../general_headers.h"
#include "../file_io.h"
#include "vars.h"

//int vars_cnt=0;

BasicVar::BasicVar(pod_string theName,int theFlags) : name(theName)
{ 
//   name=theName;
   flags=theFlags;
//   vars_cnt++;
//   printf("creating var %s, now %d\n",name.c_str(),vars_cnt);
}

BasicVar::~BasicVar()
{
//   vars_cnt--;
//   printf("deleting var %s, %d left\n",name.c_str(),vars_cnt);
}

void BasicVar::toXML( xmlTextWriterPtr ptr )
{
   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>( renderToString().c_str() ));
}

void BasicVar::fromXML( XmlTextReader * reader )
{
   bool       hasRead       = true;
   bool       inItemElement = false;
   int        ourItemDepth  = -1;
   pod_string name;
   pod_string itemValue;
   
   while( hasRead )
   {
      int type = reader->NodeType();
      if( type == XML_READER_TYPE_ELEMENT || type == XML_READER_TYPE_END_ELEMENT )
      {
         name = reader->Name();

         if( type == XML_READER_TYPE_ELEMENT )
         {
            if( name == "item" && inItemElement == false)
            {
               inItemElement = true;
               ourItemDepth = reader->Depth();
            }
         }
         else if( type == XML_READER_TYPE_END_ELEMENT )
         {
            if( name == "item" && ourItemDepth == reader->Depth() )
            {
               inItemElement = false;
               ourItemDepth = -1;
               return;
            }
         }
      }
      else if( type == XML_READER_TYPE_TEXT && inItemElement )
      {
         itemValue = reader->Value();

         if( setFromString( itemValue ) == -1 )
         {
            std::cerr << "setFromString returned 0! for var: " << name << std::endl;
         }
         // ok!
      }

      hasRead = reader->Read();
   }
}

pod_string BasicVar::toText()
{
   pod_ostringstream outputStream;

   outputStream.width(22);
   outputStream.setf(std::ios::left);
   outputStream << getName() << " : ";

   outputStream << renderToString() << std::endl;

   return outputStream.str();
}


