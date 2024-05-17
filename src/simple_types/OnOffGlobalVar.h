#ifndef ONOFFGLOBVAR_H
#define ONOFFGLOBVAR_H

class OnOffGlobalVar : public SelectGlobalVar
{
   private:
      StringVector myOptions;

   public:
      static const int OFF = 0;
      static const int ON  = 1;

      OnOffGlobalVar( char *,int, int);
};

#endif /* !ONOFFGLOBVAR_H */
