#ifndef USERGLOBALVAR_H
#define USERGLOBALVAR_H

class UserGlobalVar : public BasicVar
{
   protected:
      UR_OBJECT user;
      pod_string userName;
      const static char* const unsetString;

   public:
      UserGlobalVar( pod_string theName, int theInitType  );
      virtual int    init();
      virtual int    setFromString(pod_string value);
      virtual pod_string renderToString();
      bool set( UR_OBJECT user );
      UR_OBJECT get();
};

class UserObjectCreator : public ObjectCreator
{
   public:
      UserObjectCreator(){};
      BasicVar *getObjectInstance( pod_string name );
      ~UserObjectCreator() {};
};

#endif /* !USERGLOBALVAR_H */
