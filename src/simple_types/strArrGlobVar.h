#ifndef STRARRGLOBVAR_H
#define STRARRGLOBVAR_H

class strArrGlobVar : public BasicVar
{
   protected:
      typedef std::vector<pod_string, pod_alloc< pod_string >::Type > StringsVector;

      int init_val;
      int arr_size;
      int arr_cnt;
      StringsVector strings;
      pod_string init_str;

   public:
      strArrGlobVar(  pod_string theName, int theInitType, int theSize, pod_string theInitStr = "");
      virtual int        init();
      virtual void       toXML( xmlTextWriterPtr ptr);
      virtual void       fromXML( XmlTextReader * reader );
      virtual pod_string toText();
      pod_string get( unsigned int index);
      void set( unsigned int index, pod_string newString );
};

class StrArrObjectCreator : public ObjectCreator
{
   protected:
      pod_string _initVal;
      int        _sizeVal;

   public:
      StrArrObjectCreator(int size, pod_string initVal) : _initVal(initVal) , _sizeVal(size) {};
      BasicVar *getObjectInstance(pod_string name);
      ~StrArrObjectCreator() {};
};


#endif /* !STRARRGLOBVAR_H */
