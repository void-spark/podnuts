#ifndef XMLTEXTREADER_H
#define XMLTEXTREADER_H

#include "PodRuntimeError.h"

class XmlError : public PodRuntimeError
{
   public:
      XmlError(const pod_string& whatarg) : PodRuntimeError(whatarg) { }
      pod_string getMessage();
};

class NoSuchAttribute : public XmlError
{
   public:
      NoSuchAttribute(const pod_string& whatarg) : XmlError(whatarg) { }
};


class XmlTextReader
{
   protected:
      xmlTextReaderPtr reader;
      xmlParserInputBufferPtr buffer;

   public:
      XmlTextReader( pod_string filename );
      XmlTextReader( const char* mem, int size );
      ~XmlTextReader(); 
      
      bool       Read();      
      int        NodeType();
      pod_string Name();
      pod_string Value();
      int        Depth();
      pod_string GetAttribute( pod_string name );
      int        AttributeCount();      
      void       MoveToAttribute( int no );
      bool       IsEmptyElement();
      
};

#endif /* !XMLTEXTREADER_H */
