#include "general_headers.h"
#include "string_misc.h"
#include "GlobalVars.h"
#include "time_utils.h"
#include "globals.h"
#include "hist.h"

int get_curr_quarter()
{
   struct tm *tm_struct;
   tm_struct = get_current_localtime();

   return ((tm_struct->tm_hour%24)*4)+(int)((tm_struct->tm_min%60)/15);
}

int get_curr_daypiece()
{
   struct tm *tm_struct;
   tm_struct = get_current_localtime();

   return ((tm_struct->tm_wday%7)*12)+(int)((tm_struct->tm_hour%24)/2);
}

IntGlobalVar hist_last_minute  ("hist_last_minute",   EVERY_BOOT, 0);
IntArrayGlobalVar hist_15mins  ("hist_15mins",        EVERY_BOOT, 0,15);
IntArrayGlobalVar histw_15mins ("histw_15mins",       EVERY_BOOT, 0,15);

IntGlobalVar usrcnt_hist_curr  ("usrcnt_hist_curr",   EVERY_BOOT, get_curr_quarter());
FltArrayGlobalVar usrcnt_hist  ("usrcnt_hist",        EVERY_BOOT, 0,24*4);
FltArrayGlobalVar histw_1day   ("histw_1day",         EVERY_BOOT, 0,24*4);

IntGlobalVar hist_last_daypiece("hist_last_daypiece", EVERY_BOOT, get_curr_daypiece());
FltArrayGlobalVar hist_7days   ("hist_7days",         EVERY_BOOT, 0,12*7);
FltArrayGlobalVar histw_7day   ("histw_7day",         EVERY_BOOT, 0,12*7);


void hist_init()
{
   struct tm *tm_struct;
   tm_struct = get_current_localtime();

   hist_last_minute.set( tm_struct->tm_min % 15 );
   glob_add_var_cust( &hist_last_minute );
   glob_add_var_cust( &hist_15mins );
   glob_add_var_cust( &histw_15mins );

   glob_add_var_cust( &usrcnt_hist_curr );
   glob_add_var_cust( &usrcnt_hist );
   glob_add_var_cust( &histw_1day );

   glob_add_var_cust( &hist_last_daypiece );
   glob_add_var_cust( &hist_7days );
   glob_add_var_cust( &histw_7day );
}

int hist_24hrs_save()
{
   char filename[80];
   FILE *fp;
   int cnt;

   sprintf(filename,"%s/%s.%d",LOGFILES,"hist24hrs",(int)time(0));   
      
   fp=fopen(filename,"w");
   fprintf(fp,"t:%s\n" ,long_date(2).c_str());
   fprintf(fp,"f:%d\n" ,(usrcnt_hist_curr.get()+1)%(24*4));
   for(cnt=0;cnt<(24*4);cnt++) fprintf(fp,"u:%d:%.1f\n" ,cnt,usrcnt_hist[cnt]);
   for(cnt=0;cnt<(24*4);cnt++) fprintf(fp,"w:%d:%.1f\n" ,cnt,histw_1day[cnt]);
   fclose (fp);
   
   return 0;
}

int hist_7days_save()
{
   char filename[80];
   FILE *fp;
   int cnt;

   sprintf(filename,"%s/%s.%d",LOGFILES,"hist7days",(int)time(0));   
      
   fp=fopen(filename,"w");
   fprintf(fp,"t:%s\n" ,long_date(2).c_str());
   fprintf(fp,"f:%d\n" ,(hist_last_daypiece.get()+1)%(12*7));
   for(cnt=0;cnt<(12*7);cnt++) fprintf(fp,"u:%d:%.1f\n" ,cnt,hist_7days[cnt]);
   for(cnt=0;cnt<(12*7);cnt++) fprintf(fp,"w:%d:%.1f\n" ,cnt,histw_7day[cnt]);
   fclose (fp);
   
   return 0;
}

int hist_update()
{
   int cnt;
   float sum,sumw;
   float avg,avgw;

   struct tm *tm_struct;
   tm_struct = get_current_localtime();

   if(hist_last_daypiece.get() != get_curr_daypiece())
   {
      sum=0.0;
      sumw=0.0;
      
      if(get_curr_daypiece() == 0) hist_7days_save();
      
      for(cnt=usrcnt_hist_curr.get();cnt > usrcnt_hist_curr.get()-8;cnt--)  
      {
         sum  += usrcnt_hist[cnt];
         sumw += histw_1day[cnt];
      }
      avg  = sum/8;
      avgw = sumw/8;

      hist_last_daypiece.set(get_curr_daypiece());

      hist_7days[hist_last_daypiece.get()]=avg;
      histw_7day[hist_last_daypiece.get()]=avgw;
   }
   if(usrcnt_hist_curr.get() != get_curr_quarter())
   {
      sum=0.0;
      sumw=0.0;


      if(get_curr_quarter() == 0) hist_24hrs_save();
      for(cnt=0;cnt < 15;cnt++)
      {
         sum  += (float)hist_15mins[cnt];
         sumw += (float)histw_15mins[cnt];
      }
      avg  = sum/15;
      avgw = sumw/15;

      usrcnt_hist_curr.set(get_curr_quarter());

      usrcnt_hist[usrcnt_hist_curr.get()]=avg;
      histw_1day[usrcnt_hist_curr.get()]=avgw;
   }
   if(hist_last_minute.get() != tm_struct->tm_min%15)
   {
      hist_last_minute.set(tm_struct->tm_min%15);
      hist_15mins[hist_last_minute.get()] = get_num_of_unhidden_users();
      histw_15mins[hist_last_minute.get()] = get_num_of_unhidden_wizs();
   }
   return 0;
}

int hist_info(UR_OBJECT user) /* take this out later on, just for debugging */
{
   struct tm *tm_struct;
   tm_struct = get_current_localtime();
   int cnt;
   
   write_seperator_line(user,"hist");

   write_userf(user,"15 min array (last min %i):\n",hist_last_minute.get());
   for(cnt=0;cnt < 15    ;cnt++) write_userf(user,"%i:%i\n",cnt,hist_15mins[cnt]);

   write_userf(user,"one day array (last quart %i):\n",usrcnt_hist_curr.get());
   for(cnt=0;cnt < (24*4);cnt++) write_userf(user,"%i:%.1f\n",cnt,usrcnt_hist[cnt]); 

   write_userf(user,"7 day array (last daypiece(8hrs) %i (day %i)):\n",hist_last_daypiece.get(),tm_struct->tm_wday);
   for(cnt=0;cnt < (12*7);cnt++) write_userf(user,"%i:%.1f\n",cnt,hist_7days[cnt]); 
   
   write_seperator_line(user,NULL);
   write_user(user,"\n");
   return 0;
}


