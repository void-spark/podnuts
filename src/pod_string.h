#ifndef POD_STRING_H
#define POD_STRING_H

#include <memory>
#include <string>
#include <sstream>
#include "pod_alloc.h"

#define ARR_SIZE 2049

typedef std::basic_string        < char, std::char_traits<char>, pod_alloc<char>::Type > pod_string;
typedef std::basic_ostringstream < char, std::char_traits<char>, pod_alloc<char>::Type > pod_ostringstream;
typedef std::basic_istringstream < char, std::char_traits<char>, pod_alloc<char>::Type > pod_istringstream;
typedef std::basic_stringstream  < char, std::char_traits<char>, pod_alloc<char>::Type > pod_stringstream;
typedef std::basic_stringbuf     < char, std::char_traits<char>, pod_alloc<char>::Type > pod_stringbuf;

#endif /* !POD_STRING_H */
