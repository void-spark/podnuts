#ifndef TIMETGLOBVAR_H
#define TIMETGLOBVAR_H

class TimeTGlobalVar : public BasicVar
{
   protected:
      time_t init_val;
      time_t timeVal;

   public:
      TimeTGlobalVar( char *, int, time_t);
      virtual int        init();
      virtual int        setFromString(pod_string value);
      virtual pod_string renderToString();

      time_t get()
      {
         return timeVal;
      };
      void set(time_t newVal)
      {
         timeVal = newVal;
      };
      void setCurrentTime()
      {
         timeVal = time(0);
      }
};

#endif /* !TIMETGLOBVAR_H */
