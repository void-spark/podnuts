#ifndef POD_ALLOC_H
#define POD_ALLOC_H

#include <memory>
#include <string.h>

void *xalloc(size_t size, char *desc);
void xfree(void *r);

#ifdef DEBUG_STL
class __custom_alloc
{
   public:
      // this one is needed for proper vcl_simple_alloc wrapping
      typedef char value_type;

      static void*  allocate(size_t n)
      {
         return 0 == n ? 0 : xalloc(n, "stl");
      }

      static void*  reallocate(void *p, size_t old_sz, size_t new_sz)
      {
         void* result = allocate(new_sz);
         size_t copy_sz = new_sz > old_sz? old_sz : new_sz;
         memcpy(result, p, copy_sz);
         deallocate(p, old_sz);
         return result;
      }

      static void deallocate(void* p)
      {
         xfree(p);
      }

      static void deallocate(void* p, size_t)
      {
         xfree(p);
      }
};

inline bool operator == ( const __custom_alloc&, const __custom_alloc& )
{
  return true;
}

inline bool operator != ( const __custom_alloc&, const __custom_alloc& )
{
  return false;
}
#endif /* DEBUG_STL */

template <class T>
struct pod_alloc
{
#ifdef DEBUG_STL
    typedef std::__allocator<T, __custom_alloc > Type;
#else
    typedef std::allocator<T> Type;
#endif
//    typedef std::__allocator<T,std::__debug_alloc< __new_alloc<0> > > Type;
//    typedef std::__allocator<T,std::__debug_alloc< std::__malloc_alloc_template<0> > > Type;
//    typedef std::__allocator<T,std::debug_alloc< std::malloc_alloc<0> > > Type;
};

// New-based allocator.  Typically slower than default alloc below.
// Typically thread-safe and more storage efficient.

/*class __custom_alloc
{
   public:
      // this one is needed for proper vcl_simple_alloc wrapping
      typedef char value_type;

      static void*  allocate(size_t n)
      {
         return 0 == n ? 0 : ::operator new(n);
      }

      static void*  reallocate(void *p, size_t old_sz, size_t new_sz)
      {
         void* result = allocate(new_sz);
         size_t copy_sz = new_sz > old_sz? old_sz : new_sz;
         memcpy(result, p, copy_sz);
         deallocate(p, old_sz);
         return result;
      }

      static void deallocate(void* p)
      {
         ::operator delete(p);
      }

      static void deallocate(void* p, size_t)
      {
         ::operator delete(p);
      }
};*/

#endif /* !POD_ALLOC_H */
