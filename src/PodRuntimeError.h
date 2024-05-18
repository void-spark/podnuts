#ifndef PODRUNTIMEERROR_H
#define PODRUNTIMEERROR_H

#include <exception>
#include <stdexcept>
#include "pod_string.h"

class PodRuntimeError : public std::exception
{
   protected:
      pod_string _what;
   public:
      PodRuntimeError(const pod_string& what_arg) throw() : _what (what_arg) { }
      virtual ~PodRuntimeError() throw() {};
      virtual const char* what () const throw() { return _what.c_str (); }
   protected:
      PodRuntimeError(): exception () { }
};

#endif /* !PODRUNTIMEERROR_H */
