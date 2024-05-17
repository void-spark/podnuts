#ifndef INTGLOBVAR_H
#define INTGLOBVAR_H

class IntGlobalVar : public BasicVar
{
   protected:
      int init_val;
      int val;

   public:
      IntGlobalVar( pod_string ,int, int);
      IntGlobalVar( pod_string ,int);
      virtual int        init();
      virtual int        setFromString(pod_string value);
      virtual pod_string renderToString();

      int    get() {return val; }
      virtual int    set(const int &newval) {val=newval; return val;}
      void  decrease(const int &amount) { val -= amount;}
      void   increase(const int &amount) { val += amount;}
      operator int const& () const;
      bool operator == (const int &value)
      {
         if(val==value)
         {
            return true;
         }
         return false;
      };
      const IntGlobalVar & operator ++ () { val++;return *this; }
      const IntGlobalVar & operator -- () { val--;return *this; }
/*      void operator ++ (int) 
      { 
        int result = val;
        val++;
        return result; 
      }
      void operator -- (int) 
      { 
        int result = val;
        val--;
        return result; 
      }*/
};

class IntObjectCreator : public ObjectCreator
{
   protected:
      int _initVal;

   public:
      IntObjectCreator(int initVal) : _initVal(initVal) {};
      BasicVar *getObjectInstance(pod_string name);
      ~IntObjectCreator() {};
};

#endif /* !INTGLOBVAR_H */
