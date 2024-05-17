#ifndef INTARRAYGLOBALVAR_H
#define INTARRAYGLOBALVAR_H

class IntArrayGlobalVar : public BasicVar
{
   protected:
      int init_val;
      unsigned int arr_size;
      unsigned int arr_cnt;
      int *array;

   public:
      IntArrayGlobalVar( const char *theName, int flags, int theInitVal ,unsigned int theSize);
      virtual int    init();
      virtual void   toXML( xmlTextWriterPtr ptr);
      virtual void   fromXML( XmlTextReader * reader );
      int & operator [ ] (unsigned int index);
      virtual pod_string toText();
};

#endif /* !INTARRAYGLOBALVAR_H */
