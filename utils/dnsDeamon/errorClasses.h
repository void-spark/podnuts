#ifndef ERRORCLASSES_H
#define ERRORCLASSES_H

#include <exception>
#include <stdexcept>

class RuntimeError : public std::exception
{
      std::string _what;
   public:
      RuntimeError(const std::string& what_arg) throw() : _what (what_arg) { }
      virtual ~RuntimeError() throw() {};
      virtual const char* what () const throw()
      {
         return _what.c_str ();
      }
   protected:
      RuntimeError(): exception () { }
};

class IoError : public RuntimeError
{
   public:
      IoError(const std::string& whatarg) : RuntimeError(whatarg) { }
};

class FatalError : public RuntimeError
{
   public:
      FatalError(const std::string& whatarg) : RuntimeError(whatarg) { }
};

class ParseError : public RuntimeError
{
   public:
      ParseError(const std::string& whatarg) : RuntimeError(whatarg) { }
};

class DNSError : public RuntimeError
{
   public:
      DNSError(const std::string& whatarg) : RuntimeError(whatarg) { }
};
#endif /* !ERRORCLASSES_H */
