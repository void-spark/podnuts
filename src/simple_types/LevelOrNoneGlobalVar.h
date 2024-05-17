#ifndef LEVELORNONEGLOBVAR_H
#define LEVELORNONEGLOBVAR_H

class LevelOrNoneGlobalVar : public IntGlobalVar
{

   public:
      LevelOrNoneGlobalVar( char *,int, int);
      virtual int    setFromString(pod_string value);
      virtual pod_string renderToString();
      virtual int    set(const int &newval);
      ~LevelOrNoneGlobalVar(){};
};

#endif /* !LEVELORNONEGLOBVAR_H */
