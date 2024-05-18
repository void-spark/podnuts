#ifndef NEWOLDGLOBVAR_H
#define NEWOLDGLOBVAR_H

class NewOldGlobalVar : public SelectGlobalVar
{
   private:
      StringVector myOptions;

   public:
      static const int OLD = 0;
      static const int NEW = 1;

      NewOldGlobalVar( char *,int, int);
};

#endif /* !NEWOLDGLOBVAR_H */
