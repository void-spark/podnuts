#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "general_headers.h"
#include "string_misc.h"
#include "time_utils.h"
#include "color.h"
#include "who.h"

extern char *noyes1[];
extern char *who_user[];

int vis_override(UR_OBJECT looker, UR_OBJECT looked_at)
{
   if( (looked_at->level >= looker->level) && looked_at != looker) return 0;
   return 1;
}
/*** Show who is on ***/
int who(UR_OBJECT user, int people)
{
   UR_OBJECT u;
   RoomsVector::iterator roomNode;

   char tag[] = "~LI?~RS";
   int sharp_who=0,nummeric_id=0;
   int total=0,invis=0,logins=0;
   bool first_room = true;
   int mins,idle,wrote_room;
   char portstr[5];

   if(words.word_count >= 2)
   {
      strtolower(words.word[1]);
      if ( !strcmp(words.word[1],"-s") ) sharp_who = 1;
      if ( !strcmp(words.word[1],"-n") ) nummeric_id = 1;
   }

   write_seperator_line(user,NULL);
   if(people)         write_userf(user,"~FT    Current users logged in on POD: %s\n",long_date(1).c_str());
   else if(sharp_who) write_userf(user,"~FTA sharp echo returns these details %s\n",long_date(1).c_str());
   else               write_userf(user,"~FT   A quick ping of POD reveals........  use .swho for a more powerful scan.\n");;
   write_seperator_line(user,NULL);
   
   if (people) write_user(user,"~FTName         : Levl Line Igall Visi Idle Mins Port Site/Service\n\n");
   else if(sharp_who) write_user(user,"~FTName          Rank  Mins/Stat/Idle Mood Age/Gender    Species~RS\n\n");

   roomNode = roomsList.begin();
   do
   {
      if(!first_room)
      {
         roomNode++;
      }
      first_room = false;

      wrote_room=0;
      for(u=user_first;u!=NULL;u=u->next) 
      {
         if(u->room == (*roomNode) || (roomNode == roomsList.end() && u->login))
         {
            if (u->type==CLONE_TYPE) continue; /* don't see at clones at all */
            if (u->cloaked && !vis_override(user,u) ) 
            {
               continue;
            }

            if(!secret_room_vis(user,u)) continue;

            mins=(int)(time(0) - u->last_login)/60;
            
            idle=get_idle_mins(u);
            if (u->socket->getLocalPort() == socketInterface.getListenPortNumber("U")) strcpy(portstr,"MAIN");
            else                  strcpy(portstr," WIZ");

            if (u->login) /* ignore user on login */
            {
               if (people) /* unless we're doing .people */
               {
                  if(!nummeric_id) write_userf(user,"~FY[Login %2d]   : %-4s   %2d   %3s  %3s  %3d %4s %4s %s:%d\n",u->login,"-",u->socket->getFileDescriptor(),"-","-",idle,"-",portstr,u->socket->getPeerSite().c_str(),u->socket->getPeerPort() );
                  else             write_userf(user,"~FY[Login %2d]   : %-4s   %2d   %3s  %3s  %3d %4s %4s %s:%d\n",u->login,"-",u->socket->getFileDescriptor(),"-","-",idle,"-",portstr,u->socket->ip_num,u->socket->getPeerPort());
                  logins++;
               }
               continue;
            }
         
            ++total; /* finally, we actually see this user, cheer! ;) */
            if (!u->vis) /* invisible user, count, don't see if we're lower level */
            {
               ++invis;
               if (!vis_override(user,u)) continue;
            }
            else if((*roomNode)->inked) invis++;
            if (people) 
            {   
               if(!nummeric_id) write_userf(user,"%-13s: %-4s   %2d   %3s  %3s %4d %4d %4s %s\n",u->name,getLevelShortName(u->level),u->socket->getFileDescriptor(),noyes1[u->ignall],noyes1[u->vis],idle,mins,portstr,u->socket->getPeerSite().c_str());
               else             write_userf(user,"%-13s: %-4s   %2d   %3s  %3s %4d %4d %4s %s\n",u->name,getLevelShortName(u->level),u->socket->getFileDescriptor(),noyes1[u->ignall],noyes1[u->vis],idle,mins,portstr,u->socket->ip_num);
               continue;
            }
            if(!wrote_room && !sharp_who)
            {
               if(!(*roomNode)->inked) write_userf(user,"~FY%s...\n",(*roomNode)->name);
               else           write_userf(user,"~FY%s...\n   ~FM~OL~LII~RS ~FM~OL~LIN~RS ~FM~OL~LIK~RS ~FM~OL~LIE~RS ~FM~OL~LID~RS\n",(*roomNode)->name);
               wrote_room = TRUE;
            }
            if(!(*roomNode)->inked || (user->level >= LEV_THR) )
            {
               StrGlobalVar *color     = (StrGlobalVar*)u->getVar("Color");

               if(sharp_who)
               {
                  StrGlobalVar *species   = (StrGlobalVar*)u->getVar("species");
                  StrGlobalVar *mood      = (StrGlobalVar*)u->getVar("mood");
                  StrGlobalVar *age       = (StrGlobalVar*)u->getVar("Age");
                  StrGlobalVar *gend_desc = (StrGlobalVar*)u->getVar("Gender");

                  write_userf(user,"%s%-*s~RS  %-4s~RS  %4d~RS %s~RS %-4d~RS %-*s~RS %*s~RS %-*s~RS  %-*s~RS\n",
                                                                                     color->get().c_str(),
                            USER_NAME_LEN,                                           u->name,
                                                                                     getLevelShortName(u->level),
                                                                                     mins,
                                                                                     get_stat(u),
                                                                                     idle,
                            MOOD_LEN+(color_com_count( mood->get() )*3),     mood->get().c_str(),
                            AGE_LEN,                                                 age->get().c_str(),
                            GEND_DESC_LEN,                                           gend_desc->get().c_str(),
                            SPECIES_LEN,                                             species->get().c_str() );
               }
               else
               {
                  StrGlobalVar *desc = (StrGlobalVar*)u->getVar("desc");
                  IntGlobalVar *timezone  = (IntGlobalVar*)u->getVar("timezone");

                  struct tm *tm_struct;
                  tm_struct = get_current_utctime();
                  int localMin  = tm_struct->tm_min;
                  int localHour = (tm_struct->tm_hour + timezone->get() + 24) % 24;

                  if(u->tag && (user->level >= LEV_THR)) strcpy(tag,"~LI?~RS");
                  else                               strcpy(tag," ");
                  write_userf(user,"%s%c %02d%02d %s%-*s~RS  %-*s~RS\n",
                                             tag,
                                             get_visible_prechar(u),
                                             localHour,
                                             localMin,
                                             color->get().c_str(),
                               USER_NAME_LEN,u->name,
                               USER_DESC_LEN,desc->get().c_str() );
               }
            }
         }
      }
   } while( roomNode != roomsList.end() );

   if(sharp_who || people)
   {
      pod_stringstream outputStream;
      outputStream << "\n~FTThere are " << total-invis << " visible, " << invis << " invisible users.\n~FTTotal of " << total << " users";
      if (people)
      {
         outputStream << " and " << logins << " logins.\n\n";
      }
      else
      {
         outputStream << ".\n\n";
      }
      write_user(user,outputStream.str().c_str());
   }
   else
   {
      write_seperator_line(user,NULL);
      write_user(user,"\n");
   }
   return 0;
}
