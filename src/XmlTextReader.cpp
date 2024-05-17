#include <iostream>
#include "pod_string.h"
#include "libxml_glue.h"
#include "IoError.h"
#include "XmlTextReader.h"

pod_string XmlError::getMessage()
{
   return _what;
}

XmlTextReader::XmlTextReader( pod_string filename )
{
   buffer = 0;
   
#warning fix error handling here!
   reader = xmlNewTextReaderFilename( filename.c_str() );
   if(reader == 0 )
   {
//      xmlErrorPtr errorPtr = xmlGetLastError();
      
      pod_string error = "Failed to construct XmlTextReader for file: ";
      error += filename;
      error += " .\n";
      error += "Reason: "; 
//      error += errorPtr->message;
      error += " .";
      
/*      if( errorPtr->domain == XML_FROM_IO )
      {
         throw IoError( error );
      }
      else
      {*/
         throw XmlError( error ); 
//      }
            
   }
}

      
XmlTextReader::XmlTextReader( const char* mem, int size )
{
   buffer = 0;
   
   buffer = xmlParserInputBufferCreateMem( mem, size, XML_CHAR_ENCODING_UTF8 );
                                             
   reader = xmlNewTextReader( buffer, 0 );
      
   if(reader == 0 )
   {
//      xmlErrorPtr errorPtr = xmlGetLastError();
      
      pod_string error = "Failed to construct XmlTextReader from memory.\n";
      error += "Reason: "; 
//      error += errorPtr->message;
      error += " .";
      
      throw XmlError( error ); 
            
   }
}
      

XmlTextReader::~XmlTextReader()
{
   xmlFreeTextReader( reader );
   
   if( buffer != 0 )
   {   
      xmlFreeParserInputBuffer( buffer ); 
   }
}

bool XmlTextReader::Read()
{
   int retval = xmlTextReaderRead( reader );
   return ( retval != 0 );
}

int XmlTextReader::NodeType()
{
   return xmlTextReaderNodeType(reader);
}

pod_string XmlTextReader::Name()
{
   xmlChar *xmlName = xmlTextReaderName(reader);
   if( xmlName == 0 )
   {
      std::cerr << "XmlTextReader::Name() : xmlTextReaderName() returned 0." << std::endl;
      abort();
   }
   pod_string name = reinterpret_cast<const char*>(xmlName);
   xmlFree(xmlName);
   
   return name;
}

pod_string XmlTextReader::Value()
{
   xmlChar *xmlValue = xmlTextReaderValue(reader);
   if( xmlValue == 0 )
   {
      std::cerr << "XmlTextReader::Value() : xmlTextReaderValue() returned 0." << std::endl;
      abort();
   }
   pod_string value = reinterpret_cast<const char*>(xmlValue);
   xmlFree(xmlValue);
               
   return value;
}

int XmlTextReader::Depth()
{
   return xmlTextReaderDepth( reader );
}

int XmlTextReader::AttributeCount()
{
   return xmlTextReaderAttributeCount( reader );
}

void XmlTextReader::MoveToAttribute( int no )
{
   int succes = xmlTextReaderMoveToAttributeNo( reader, no );
   if( ! succes )
   {
      throw XmlError( "Value out of range" );
   }
}

pod_string XmlTextReader::GetAttribute( pod_string name )
{
   xmlChar *xmlAttribute = xmlTextReaderGetAttribute( reader, reinterpret_cast<const xmlChar*>( name.c_str() ) );
   if( xmlAttribute == 0 )
   {
      pod_string error = "Attribute not found: " + name;
      throw NoSuchAttribute(error);
   }
   pod_string attribute = reinterpret_cast<const char*>(xmlAttribute);
   xmlFree(xmlAttribute);

   return attribute;
}

bool XmlTextReader::IsEmptyElement()
{
   return xmlTextReaderIsEmptyElement( reader );
}

