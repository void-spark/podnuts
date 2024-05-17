#ifndef SELECTGLOBALVAR_H
#define SELECTGLOBALVAR_H

#include <vector>
typedef std::vector<pod_string, pod_alloc< pod_string >::Type > StringVector;

class SelectGlobalVar : public IntGlobalVar
{
   protected:
      StringVector *selectList;
      bool isOutOfRange(int test);

   public:
      SelectGlobalVar( char *,int, int);
      SelectGlobalVar( char *,int, int, StringVector *);
      virtual int        setFromString(pod_string value);
      virtual pod_string renderToString();
      virtual void       init(StringVector *);

      virtual int        set(const int &newval);
      ~SelectGlobalVar();
};

#endif /* !SELECTGLOBALVAR_H */
