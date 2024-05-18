#include "../levels.h"
#include "../socket_funcs.h"
#include "../GlobalVars.h"
#include "../Room.h"
#include "../user_objects.h"
#include "../speech_funcs.h"
#include "../wordfind.h"
#include "../StringLibrary.h"
#include "../help.h"
#include "../cmd_main.h"
#include "../podnuts.h"
#include "../logging.h"
#include "../string_misc.h"
#include "../file_locations.h"
#include "../telnet.h"

int matchsite(UR_OBJECT user,char *inpstr);
int matchsite_initial(UR_OBJECT user);
int matchsite_user_check(UR_OBJECT user);
int matchsite_site_check(UR_OBJECT user);

extern "C" void plugin_init()
{
   user_var_add_cust( "matchsite_check_store", new StrObjectCreator(ARR_SIZE,""), USR_SAVE_SOFT);
   user_var_add_cust( "matchsite_type_store", new IntObjectCreator(0), USR_SAVE_SOFT);
   user_var_add_cust( "matchsite_all_store",  new IntObjectCreator(0), USR_SAVE_SOFT);

   cmd_add("matchsite",   LEV_THR, ADM, &matchsite);
}

int matchsite(UR_OBJECT user,char *inpstr)
{
   struct words_struct* words_ptr = &words;

   if(user->nextCommand)
   {


      StrGlobalVar *matchsite_check_store  = (StrGlobalVar*)user->getVar("matchsite_check_store");
      IntGlobalVar *matchsite_all_store    = (IntGlobalVar*)user->getVar("matchsite_all_store");
      IntGlobalVar *matchsite_type_store   = (IntGlobalVar*)user->getVar("matchsite_type_store");

      if (!inpstr[0])
      {
         write_user(user,"Abandoning your matchsite look-up.\n");
         user->nextCommand = 0;
         matchsite_all_store->set(0);
         matchsite_type_store->set(0);
         matchsite_check_store->set("");
         prompt(user);
      }
      else
      {
         switch( matchsite_type_store->get() )
         {
            case 1:
               user->nextCommand = 0;
               words_ptr->word[0][0]=toupper(words_ptr->word[0][0]);
               matchsite_check_store->set(words_ptr->word[0]);
               matchsite_user_check(user);
            return 1;

            case 2:
               user->nextCommand = 0;

               matchsite_check_store->set(words_ptr->word[0]);
               matchsite_site_check(user);
            return 1;

            default:
            /* oops */
            return 1;
         }
      }
   }
   else
   {
      matchsite_initial(user);
   }
   return 1;
}

int matchsite_initial(UR_OBJECT user)
{
   struct words_struct* words_ptr = &words;

   IntGlobalVar *matchsite_all_store  = (IntGlobalVar*)user->getVar("matchsite_all_store");
   IntGlobalVar *matchsite_type_store = (IntGlobalVar*)user->getVar("matchsite_type_store");

   if (words_ptr->word_count >= 2)
   {
      strtolower(words_ptr->word[1]);
      strtolower(words_ptr->word[2]);

      if (words_ptr->word_count==3 && !strcmp(words_ptr->word[2],"-a"))
      {
         matchsite_all_store->set(1);
      }
      else
      {
         matchsite_all_store->set(0);
      }

      if (!strcmp(words_ptr->word[1],"user"))
      {
         write_user(user,"Enter the name of the user to be matched against: ");
         telnet_eor_send(user);
         matchsite_type_store->set(1);
         user->nextCommand=currentCommand;
         no_prompt=1;
         return 0;
      }

      if (!strcmp(words_ptr->word[1],"site"))
      {
         write_user(user,"~OL~FRNOTE:~RS Partial site strings can be given, but NO wildcards.\n");
         write_user(user,"Enter the site to be matched against: ");
         telnet_eor_send(user);
         matchsite_type_store->set(2);
         user->nextCommand=currentCommand;

         no_prompt=1;
         return 0;
      }
   }

   write_user(user,"Usage: matchsite user/site [-a]\n");
   return 0;
}

int matchsite_user_check(UR_OBJECT user)
{
   int found,cnt,on;
   UR_OBJECT u,u_loop;
   StrGlobalVar *matchsite_check_store = (StrGlobalVar*)user->getVar("matchsite_check_store");
   IntGlobalVar *matchsite_all_store   = (IntGlobalVar*)user->getVar("matchsite_all_store");
   StringLibrary * stringLibrary = StringLibrary::getInstance();
   char   name_to_not_match[USER_NAME_LEN+1];
   char   site_to_match[MAX_SITE_LEN+1];
   DIR *my_dir;
   struct dirent *node;

   found=0;
   cnt=0;

   if (!matchsite_all_store->get())
   {
      if ( !(u=get_user( matchsite_check_store->get().c_str() )) )
      {
         write_user_crt(user,stringLibrary->makeString("notloggedon").c_str());
         return 0;
      }
      strcpy( name_to_not_match,u->name);
      strcpy( site_to_match,    u->socket->getPeerSite().c_str());

      for (u_loop=user_first;u_loop!=NULL;u_loop=u_loop->next)
      {
         cnt++;
         if (!strcmp(site_to_match,u_loop->socket->getPeerSite().c_str()) && strcmp(name_to_not_match,u_loop->name) )
         {
            if (++found==1) write_userf(user,"\nUsers logged on from the match site as ~OL%s~RS\n\n",name_to_not_match);
            write_userf(user,"   %c%-*s %s\n",get_visible_prechar(u_loop),USER_NAME_LEN,u_loop->name,u_loop->socket->getPeerSite().c_str());
         }
      }
   }
   else
   {
      /* check all the users..  First, load the name given */
      if ( ( u=get_user( matchsite_check_store->get().c_str() ) ) ) on=TRUE;
      else
      {
      #warning bleh, const char to char
         if(!(u=temp_user_spawn(user,(char*)(matchsite_check_store->get().c_str()),"matchsite() - stg1")) ) return 0;
         on=FALSE;
      }
      strcpy( name_to_not_match,u->name);
      strcpy( site_to_match,on ? u->socket->getPeerSite().c_str() : u->last_site);

      temp_user_destroy(u);

      if(!(my_dir = opendir(USERFILES)))
      {
         write_syslogf("Error in matchsite() - stage 1/all while opening directory '%s' : '%s'.\n",TRUE,USERFILES,strerror(errno));
         write_user(user,"Sorry, you are unable to use the ~OLall~RS option at this time.\n");
         return 0;
      }
      pod_string name;
      int dotPos;
      while ( (node = readdir(my_dir)) )
      {
         if( !is_d_file(node) ) 
         {
            continue;
         }

         name   = node->d_name;
         dotPos = name.find('.',0);
         name = name.substr(0,dotPos);  
         if( (u_loop=temp_user_spawn(NULL,(char*)name.c_str(),"matchsite()")) )
         {
            cnt++;
            if (!strcmp(site_to_match, u_loop->last_site) && strcmp(name_to_not_match,u_loop->name) )
            {
               if (++found==1) write_userf(user,"\nAll users from the same site as ~OL%s~RS\n\n",name_to_not_match);
               write_userf(user,"    %-*s %s\n",USER_NAME_LEN,u_loop->name,u_loop->last_site);
            }
            temp_user_destroy(u_loop);
         }
      }

      if(closedir(my_dir)==-1)
      {
         write_syslogf("Error in matchsite() - stage 1/all while closing directory '%s' : '%s'.\n",TRUE,USERFILES,strerror(errno));
         return 0;
      }
   }
   if (!found) write_userf(user,"No users %shave the same site as %s.\n",matchsite_all_store->get() ? "" : "currently logged on ",name_to_not_match);
   else        write_userf(user,"\nChecked ~OL%d~RS users, ~OL%d~RS had the site as ~OL%s~RS ~FG(%s)\n\n",cnt,found,name_to_not_match,site_to_match);
   return 0;
}

int matchsite_site_check(UR_OBJECT user)
{
   int found,cnt;
   UR_OBJECT u_loop;
   StrGlobalVar *matchsite_check_store = (StrGlobalVar*)user->getVar("matchsite_check_store");
   IntGlobalVar *matchsite_all_store   = (IntGlobalVar*)user->getVar("matchsite_all_store");
   DIR *my_dir;
   struct dirent *node;
   char   name_to_not_match[USER_NAME_LEN+1];

   found=0;
   cnt=0;

   if (!matchsite_all_store->get()) /* check just those logged on */
   {
      for (u_loop=user_first;u_loop!=NULL;u_loop=u_loop->next)
      {
         cnt++;
         if (!strstr(u_loop->socket->getPeerSite().c_str(),matchsite_check_store->get().c_str())) continue;
         if (++found==1) write_userf(user,"\nUsers logged on from the match site as ~OL%s~RS\n\n",matchsite_check_store->get().c_str());
         write_userf(user,"   %c%-*s %s\n",get_visible_prechar(u_loop),USER_NAME_LEN,u_loop->name,u_loop->socket->getPeerSite().c_str());
      }
   }
   else /* check all the users.. */
   {
      if(!(my_dir = opendir(USERFILES)))
      {
         write_syslogf("Error in matchsite() - stage 2/all while opening directory '%s' : '%s'.\n",TRUE,USERFILES,strerror(errno));
         write_user(user,"Sorry, you are unable to use the ~OLall~RS option at this time.\n");
         return 0;
      }
   
      pod_string name;   
      int dotPos;
      while ( (node = readdir(my_dir)) )
      {
         if( !is_d_file(node) ) 
         {
            continue;
         }
         name   = node->d_name;
         dotPos = name.find('.',0);
         name = name.substr(0,dotPos);           
         /* create a user object if user not already logged on */
         if( (u_loop=temp_user_spawn(NULL,(char*)name.c_str(),"matchsite() - stg2")) )
         {
            cnt++;
            if (strstr(u_loop->last_site, matchsite_check_store->get().c_str()) && strcmp(name_to_not_match,u_loop->name) )
            {

               if (++found==1) write_userf(user,"\nAll users that have the site ~OL%s~RS\n\n",matchsite_check_store->get().c_str());
               write_userf(user,"    %-*s %s\n",USER_NAME_LEN,u_loop->name,u_loop->last_site);
            }
            temp_user_destroy(u_loop);
         }

      }
      if(closedir(my_dir)==-1)
      {
         write_syslogf("Error in matchsite() - stage 2/all while closing directory '%s' : '%s'.\n",TRUE,USERFILES,strerror(errno));
         return 0;
      }	

   }
   if (!found) write_userf(user,"No users %shave the same site as %s.\n",matchsite_all_store->get() ? "" : "currently logged on ",matchsite_check_store->get().c_str());
   else        write_userf(user,"\nChecked ~OL%d~RS users, ~OL%d~RS had the site as ~OL%s\n\n",cnt,found,matchsite_check_store->get().c_str());
   return 0;
}
