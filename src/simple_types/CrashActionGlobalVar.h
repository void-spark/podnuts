#ifndef CRASHACTIONGLOBVAR_H
#define CRASHACTIONGLOBVAR_H

class CrashActionGlobalVar : public SelectGlobalVar
{
   private:
      StringVector myOptions;

   public:
      static const int CRASH_ACT_NONE   = 0;
      static const int CRASH_ACT_IGNORE = 1;
      static const int CRASH_ACT_REBOOT = 2;

      CrashActionGlobalVar( char *,int, int);
};

#endif /* !CRASHACTIONGLOBVAR_H */
