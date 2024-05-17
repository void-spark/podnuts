#ifndef FLTARRAYGLOBALVAR_H
#define FLTARRAYGLOBALVAR_H

class FltArrayGlobalVar : public BasicVar
{
   protected:
      int init_val;
      unsigned int arr_size;
      unsigned int arr_cnt;
      float *array;

   public:
      FltArrayGlobalVar( const char *,int, int,unsigned int);
      virtual int    init();
      virtual void   toXML( xmlTextWriterPtr ptr);
      virtual void   fromXML( XmlTextReader * reader );
      float & operator [ ] (unsigned int index);
      virtual pod_string toText();
};

#endif /* !FLTARRAYGLOBALVAR_H */
