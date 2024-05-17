#ifndef DAYTIMEGLOBALVAR_H
#define DAYTIMEGLOBALVAR_H

class DayTimeGlobalVar : public BasicVar
{

   protected:
      int initMinVal;
      int initHourVal;
      int minVal;
      int hourVal;

   public:
      DayTimeGlobalVar( char *,int, int, int);
      int    getHour() { return hourVal; }
      void   setHour(const int &newval);
      int    getMin() { return minVal; }
      void   setMin(const int &newval);
      virtual int        init();
      virtual int        setFromString(pod_string value);
      virtual pod_string renderToString();

};

#endif /* !DAYTIMEGLOBALVAR_H */
