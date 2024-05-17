#include <time.h>
#include <sys/time.h>
#include "time_utils.h"

struct tm *get_current_localtime()
{
   struct tm *tm_struct;
   time_t tm_num;

   /* Set up the structure */
   time(&tm_num);
   tm_struct = localtime(&tm_num);

   return tm_struct;
}

struct tm *get_current_utctime()
{
   struct tm *tm_struct;
   time_t tm_num;

   /* Set up the structure */
   time(&tm_num);
   tm_struct = gmtime(&tm_num);

   return tm_struct;
}

long timeval_diff_usec(struct timeval *a,struct timeval *b)
{
   return (a->tv_usec - b->tv_usec) +
          ((a->tv_sec - b->tv_sec) * 1000000);
}
