#ifndef MAILCOPYTOVAR_H
#define MAILCOPYTOVAR_H

#define MAX_COPIES     20 /* of smail */

#define user_get_copyto(user)      \
        ((CopyToVar*)(user->getVar("copyto")))


class CopyToVar : public BasicVar
{
   protected:
      char lines[MAX_COPIES][USER_NAME_LEN+1];

   public:
      CopyToVar(pod_string name);
      void   toXML( xmlTextWriterPtr ptr  );
      void   fromXML( XmlTextReader * reader );
      int    init();
      pod_string CopyToVar::toText();
      int send(UR_OBJECT user,char* message);
      int to(UR_OBJECT user,char *inpstr);

      ~CopyToVar(){};
};

class CopyToObjectCreator : public ObjectCreator
{
   public:
      BasicVar *getObjectInstance(pod_string name);
};

int nocopies(UR_OBJECT user,char *inpstr);
int copies_to(UR_OBJECT user,char *inpstr);

#endif /* !MAILCOPYTOVAR_H */
