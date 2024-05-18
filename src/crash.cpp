#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "general_headers.h"
#include "string_misc.h"
#include "crash.h"

struct crash_struct {
   char name[USER_NAME_LEN+1];
   char inpstr[ARR_SIZE];
};

struct crash_struct crash_array[6];

int crash_save(UR_OBJECT user, char *inpstr)
{
   int i;

   strcpy(crash_array[5].name, user->name);
   strcpy(crash_array[5].inpstr, inpstr);
   for(i=0;i<5;i++)
   {
      strcpy(crash_array[i].name, crash_array[i+1].name);
      strcpy(crash_array[i].inpstr, crash_array[i+1].inpstr);
   }
   return 0;
}

int crash_writelog()
{
   logStream << setLogfile( CRASHLOG ) << noTime << long_date(1) << "\n" << pod_send;

   for( int i = 0; i < 5; i++)
   {
      logStream << setLogfile( CRASHLOG ) << noTime << crash_array[i].name << " | " << crash_array[i].inpstr <<  "\n" << pod_send;
   }

   logStream << setLogfile( CRASHLOG ) << noTime << "\n" << pod_send;
   return 0;
}

