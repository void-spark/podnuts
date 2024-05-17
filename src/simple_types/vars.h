#ifndef VARS_H
#define VARS_H

#include <string>
#include "../libxml_glue.h"
#include "../pod_string.h"

class BasicVar
{
   protected:
      int flags;
      pod_string name;
      
   public:
      BasicVar(pod_string,int);
      virtual void   toXML( xmlTextWriterPtr ptr );
      virtual void   fromXML( XmlTextReader * reader );
      virtual int    init()=0;
#warning make below two lines =0 someday
      virtual int    setFromString(pod_string value) { abort(); }
      virtual pod_string renderToString() { abort(); }
      virtual pod_string toText();

      const char *getName() { return name.c_str(); };
   
      int getFlags() { return flags; };

      virtual ~BasicVar();
};

class ObjectCreator
{
   public:
      virtual BasicVar *getObjectInstance(pod_string name)=0;
      virtual ~ObjectCreator() {};
};

#endif /* !VARS_H */
