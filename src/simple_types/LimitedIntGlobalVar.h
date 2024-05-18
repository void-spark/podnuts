#ifndef LIMITEDINTGLOBALVAR_H
#define LIMITEDINTGLOBALVAR_H

class LimitedIntGlobalVar : public IntGlobalVar
{
   protected:
      int maxVal;
      int minVal;
      
   public:
      LimitedIntGlobalVar( char *,int, int,int,int);
      virtual int        init();
      virtual int        setFromString(pod_string value);

      virtual int        set(const int &newval);
};

#endif /* !LIMITEDINTGLOBALVAR_H */
