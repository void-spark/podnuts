#include <string>

#include "../general_headers.h"
#include "../string_misc.h"
#include "../file_io.h"
#include "vars.h"
#include "FltArrayGlobalVar.h"

FltArrayGlobalVar::FltArrayGlobalVar( const char *theName, int theFlags, int theInitVal ,unsigned int theSize) : BasicVar(theName,theFlags)
{
   init_val = theInitVal;
   arr_size = theSize;
   array = new float[arr_size];
   arr_cnt = 0;
}

int FltArrayGlobalVar::init()
{
   unsigned int i;

   for(i=0;i < arr_size;i++)
   {
      array[i] = init_val;
   }
   return 0;
}

void FltArrayGlobalVar::toXML( xmlTextWriterPtr ptr)
{
   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("\n"));
   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("      "));
   xmlTextWriterStartElement( ptr, reinterpret_cast<const xmlChar *>("array") );
   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("\n"));
   for( int i = 0; i < arr_size; i++)
   {
      xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("         "));
      xmlTextWriterStartElement( ptr, reinterpret_cast<const xmlChar *>("val") );
      pod_string floatString = floatToString( array[i] );
      xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>( floatString.c_str() ));
      xmlTextWriterEndElement( ptr );
      xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("\n"));
   }
   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("      "));
   xmlTextWriterEndElement( ptr );
   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("\n"));
   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("   "));
}

void FltArrayGlobalVar::fromXML( XmlTextReader * reader )
{
   bool       hasRead       = true;
   bool       inItemElement = false;
   int        ourItemDepth  = -1;
   int        type          = 0;
   int        depth         = 0;
   int        subDepth      = 0;
   pod_string name;
   pod_string itemValue;

   enum
   {
      UNSET,
      LINES_ARRAY,
      LINE_VAL,
      DONE
   };
   int state = UNSET;
   int arrIndex = 0;

   while ( hasRead )
   {      
      type  = reader->NodeType();
      depth = reader->Depth();
      subDepth = depth - ourItemDepth;
      
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
                  state = LINES_ARRAY;
               }
            }
            else if( subDepth == 2 )
            {
               if( name == "val" && state == LINES_ARRAY )
               {
                  state = LINE_VAL;
               }
            }
         }
         else if( type == XML_READER_TYPE_END_ELEMENT && inItemElement )
         {
            if( depth == ourItemDepth && name == "item" )
            {
               inItemElement = false;
               ourItemDepth = -1;
               return;
            }
            else if( subDepth == 1 )
            {
               if( name == "array" && state == LINES_ARRAY )
               {
                  state = DONE;
               }
            }
            else if( subDepth == 2 )
            {
               if( name == "val" && state == LINE_VAL )
               {
                  state = LINES_ARRAY;
               }
            }
         }
      }
      else if( type == XML_READER_TYPE_TEXT && inItemElement )
      {
         itemValue = reader->Value();

         if( state == LINE_VAL )
         {
            if( arrIndex == arr_size )
            {
               write_syslogf("*** BOOT error: %s can contain only %i strings, this was number %i.\n",FALSE,getName(),arr_size, arrIndex + 1);
            }
            else
            {
               array[arrIndex] = stringToFloat(itemValue);
               arrIndex++;
            }
         }
      }
      
      hasRead = reader->Read();
   }
}

float & FltArrayGlobalVar::operator [ ] (unsigned int index)
{
   if(index >= arr_size || index < 0)
   {
      printf("Array out of bounds for %s with index %d!\n",getName(),index);
      exit(13);
   }
   return array[index];
}

pod_string FltArrayGlobalVar::toText()
{
   unsigned int i;
   pod_ostringstream outputStream;

   outputStream.width(22);
   outputStream.setf(std::ios::left);
   outputStream << getName() << " : " << std::endl;
   
   for(i=0;i < arr_size;i++)
   {
      outputStream.width(22);
      outputStream.setf(std::ios::left);
      outputStream << i << " : " << array[i] << std::endl;
   }

   return outputStream.str();
}

