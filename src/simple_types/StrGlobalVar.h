#ifndef STRGLOBALVAR_H
#define STRGLOBALVAR_H

class StrGlobalVar : public BasicVar
{
   protected:
      pod_string initStr;
      pod_string myStr;
      unsigned int length;

   public:
      StrGlobalVar( pod_string , int ,int , pod_string);
      virtual int    init();
      virtual int    setFromString(pod_string value);
      virtual pod_string renderToString();
      bool set(pod_string str);
      pod_string get();
      bool isEmpty();
};

class StrObjectCreator : public ObjectCreator
{
   protected:
      pod_string _initVal;
      int        _lengthVal;

   public:
      StrObjectCreator(int length, pod_string initVal) : _initVal(initVal) , _lengthVal(length) {};
      BasicVar *getObjectInstance(pod_string name);
      ~StrObjectCreator() {};
};

#endif /* !STRGLOBALVAR_H */
