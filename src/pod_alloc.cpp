//#include <iostream>
#include <exception> // for std::bad_alloc
#include <new>
#include "pod_alloc.h"

#ifdef DEBUG_NEW
void* operator new(std::size_t size)
{
    return xalloc( size, "new" );

//    std::cout << "new" << std::endl;
//    return ::operator new(size);
}
void* operator new[](std::size_t size)
{
    return xalloc( size, "new" );
//    std::cout << "new[]" << std::endl;
//    return ::operator new[](size);
}
void operator delete(void* ptr)
{
    xfree( ptr );
//   std::cout << "delete" << std::endl;
//   ::operator delete(ptr);
}
void operator delete[](void* ptr)
{
    xfree( ptr );
//   std::cout << "delete[]" << std::endl;
//   ::operator delete[](ptr);
}
#endif /* DEBUG_NEW */

/*void* C::operator new (size_t size)
{
 void *p=malloc(size);
 if (p==0) // did malloc succeed?
   throw std::bad_alloc(); // ANSI/ISO compliant behavior
 return p;
}

The matching delete is as follows:

void operator delete (void *p)
{
 free(p);
}
  */
