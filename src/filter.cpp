#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iomanip>
#include "general_headers.h"
#include "filter.h"

int filter_level(UR_OBJECT user)
{
   FILE *fp;
   int filter=0,num_matches,levelchk=0;
   char line[82],filename[80];
   char name[80], lvl[80];

   sprintf(filename,"%s/%s",LOGFILES,"users_levelfilter");
   if ( (fp=fopen(filename, "r")) == NULL) 
   {
      logStream << setLogfile( FILTERLOG ) << noTime << "Can't open file <" << filename <<  "> in filter.c:filter_level\n" << pod_send;
      return 0;
   }
   while(!feof(fp))
   {
      fgets(line,81,fp); 
      num_matches = sscanf(line,"%s %s",name,lvl); 
      if (num_matches < 2) continue;
      if (name[0]=='#') continue; /* skip comment lines */
      if (!strcmp(name,user->name)) 
      {

         if (!strcmp(lvl,getLevelShortName(LEV_BOT)))      user->level=LEV_BOT;
         else if (!strcmp(lvl,getLevelShortName(LEV_FIV))) user->level=LEV_FIV;
         else if (!strcmp(lvl,getLevelShortName(LEV_FOU))) user->level=LEV_FOU;
         else if (!strcmp(lvl,getLevelShortName(LEV_THR))) user->level=LEV_THR;
         else if (!strcmp(lvl,getLevelShortName(LEV_TWO))) user->level=LEV_TWO;
         else if (!strcmp(lvl,getLevelShortName(LEV_ONE))) user->level=LEV_ONE;
         else if (!strcmp(lvl,getLevelShortName(LEV_ZER))) user->level=LEV_ZER;
         filter=1;
         levelchk=1;
         break;
      }
   }
   
   if ((!levelchk)&&(user->level>=LEV_FOU))
   {
      logStream << setLogfile( FILTERLOG ) << "~OL~FYFailed level check :~RS " << std::setw(11) << user->name << " " << getLevelShortName(user->level) << "\n" << pod_send;
      user->level=LEV_THR;
   }
   fclose(fp);
   if (filter)
   {
      logStream << setLogfile( FILTERLOG ) << "Filter level check : " << std::setw(11) << user->name << " " << getLevelShortName(user->level) << "\n" << pod_send;
   }
   return 0;
}

int filter_passwd(UR_OBJECT user)
{
   logStream << setLogfile( FILTERLOG ) << "~OL~FRFailed passwd check: ~RS" << std::setw(11) << user->name << " " << user->socket->getPeerSite() << "\n" << pod_send;
   return 0;
}
