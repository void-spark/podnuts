#include <string>

#include "../general_headers.h"
#include "../file_io.h"
#include "../xalloc.h"
#include "vars.h"
#include "strArrGlobVar.h"

strArrGlobVar::strArrGlobVar(  pod_string theName, int theInitType, int theSize, pod_string theInitStr ) : BasicVar(theName,theInitType), strings(theSize)
{
   arr_size = theSize;
   arr_cnt = 0;
   init_str = theInitStr;
}

int strArrGlobVar::init()
{
   int i;

   for( i = 0; i < arr_size; i++)
   {
      strings[i] = init_str;
   }
   return 0;
}

void strArrGlobVar::toXML( xmlTextWriterPtr ptr)
{
   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("\n"));
   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("      "));
   xmlTextWriterStartElement( ptr, reinterpret_cast<const xmlChar *>("array") );
   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("\n"));
   for( int i = 0; i < arr_size; i++)
   {
      if( !strings[i].empty() )
      {
         xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("         "));
         xmlTextWriterStartElement( ptr, reinterpret_cast<const xmlChar *>("val") );
         xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>( strings[i].c_str() ));
         xmlTextWriterEndElement( ptr );
         xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("\n"));
      }
   }
   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("      "));
   xmlTextWriterEndElement( ptr );
   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("\n"));
   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("   "));
}

void strArrGlobVar::fromXML(  XmlTextReader * reader )
{
   enum
   {
      UNSET,
      LINES_ARRAY,
      LINE_VAL,
      DONE
   };

   bool       hasRead       = true;
   bool       inItemElement = false;
   int        ourItemDepth  = -1;
   int        type          = 0;
   int        depth         = 0;
   int        subDepth      = 0;
   int        state         = UNSET;
   int        arrIndex      = 0;
   pod_string name;
   pod_string itemValue;
   
   while( hasRead )
   {
      type     = reader->NodeType();
      depth    = reader->Depth();
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
               strings[arrIndex] = itemValue;
               arrIndex++;
            }
         }
      }

      hasRead = reader->Read();
   }
}


pod_string strArrGlobVar::get( unsigned int index)
{
   if( index >= strings.size() )
   {
      abort();
   }
   return strings[index];
}

void strArrGlobVar::set( unsigned int index, pod_string newString )
{
   if( index >= strings.size() )
   {
      abort();
   }
   strings[index] = newString;
}

pod_string strArrGlobVar::toText()
{
   int i;
   pod_ostringstream outputStream;

   outputStream.width(22);
   outputStream.setf(std::ios::left);
   outputStream << getName() << " : " << std::endl;

   for( i = 0; i < arr_size; i++ )
   {
      outputStream.width(22);
      outputStream.setf(std::ios::left);
      outputStream << i << " : " << strings[i] << std::endl;
   }

   return outputStream.str();
}

BasicVar* StrArrObjectCreator::getObjectInstance(pod_string name)
{
   return new strArrGlobVar( name,0,_sizeVal,_initVal);
};

