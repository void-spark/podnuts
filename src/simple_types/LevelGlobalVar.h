#ifndef LEVELGLOBVAR_H
#define LEVELGLOBVAR_H

class LevelGlobalVar : public IntGlobalVar
{

   public:
      LevelGlobalVar( char *,int, int);
      virtual int        setFromString(pod_string value);
      virtual pod_string renderToString();
      virtual int    set(const int &newval);
};

#endif /* !LEVELORNONEGLOBVAR_H */
