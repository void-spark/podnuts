#ifndef YESNOGLOBVAR_H
#define YESNOGLOBVAR_H

class YesNoGlobalVar : public SelectGlobalVar
{
   private:
      StringVector myOptions;

   public:
      static const int NO = 0;
      static const int YES = 1;

      YesNoGlobalVar( char *,int, int);
};

#endif /* !YESNOGLOBVAR_H */
