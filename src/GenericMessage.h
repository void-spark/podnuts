#ifndef GENERICMESSAGE_H
#define GENERICMESSAGE_H

#include "pod_string.h"

class GenericMessage
{
   protected:
      pod_string _from;
      pod_string _body;
      int    _time;

   public:
      GenericMessage();
      GenericMessage( XmlTextReader *reader );
      GenericMessage( pod_string from, pod_string body );
      GenericMessage( pod_string from, pod_string body, int time );
      virtual ~GenericMessage();
      
      pod_string getFrom();
      pod_string getBody();
      pod_string getDate();
      int    getTime();

      virtual void toXML( xmlTextWriterPtr ptr );
      virtual void headerContentToXML( xmlTextWriterPtr ptr );
      virtual void bodyContentToXML( xmlTextWriterPtr ptr );
      virtual void fromXml( XmlTextReader *reader );
      
   protected:
      virtual void parseHeaderElement( XmlTextReader *reader );

};
#endif /* !GENERICMESSAGE_H */
