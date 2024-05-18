#ifndef DYNAMICVARSCONTROLLER_H
#define DYNAMICVARSCONTROLLER_H

#warning was inner class
      class VarTempl
      {
         public:
            int index;
            int save_type;
            ObjectCreator *creator;
      };

class DynamicVarsController
{
   protected:
      typedef std::map<pod_string, VarTempl, std::less<pod_string>, pod_alloc< std::pair<pod_string, VarTempl> >::Type > VarsTemplMap;
      VarsTemplMap varsTemplMap;

#warning waren protected, friend class dus ?
   public:
      inline BasicVar *getptr( BasicVar** & data_ptr, VarTempl *node );
      inline void setptr( BasicVar** & data_ptr, VarTempl *node, BasicVar *newptr );

   public:
      int addVarTempl( pod_string name, ObjectCreator *var, int save_type );
      int createObj(BasicVar** & data_ptr );
      int initObj(BasicVar** & data_ptr );
      int deleteObj(BasicVar** & data_ptr );
      VarTempl *getObj( BasicVar** & data_ptr, pod_string name );
      void toXML( BasicVar** & data_ptr, int save_type, xmlTextWriterPtr ptr );
      void fromXML( XmlTextReader * reader, BasicVar** & data_ptr );
      BasicVar* objGetVar( BasicVar** & data_ptr, pod_string name );
      int objPrintVar( BasicVar** & data_ptr, UR_OBJECT user );
};

#endif /* !DYNAMICVARSCONTROLLER_H */
