#ifndef XMLPARSINGERROR_H
#define XMLPARSINGERROR_H

#include "PodRuntimeError.h"

class XmlParsingError : public PodRuntimeError
{
   public:
      XmlParsingError(const pod_string& whatarg) : PodRuntimeError(whatarg) { }
};

#endif /* !XMLPARSINGERROR_H */
