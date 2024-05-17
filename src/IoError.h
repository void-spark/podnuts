#ifndef IOERROR_H
#define IOERROR_H

#include "PodRuntimeError.h"

class IoError : public PodRuntimeError
{
   public:
      IoError(const pod_string& whatarg) : PodRuntimeError(whatarg) { }
};

class NoSuchFile : public IoError
{
   public:
      NoSuchFile(const pod_string& whatarg) : IoError(whatarg) { }
};

#endif /* !IOERROR_H */
