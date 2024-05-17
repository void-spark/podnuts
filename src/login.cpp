#include <ctype.h>
#include <string.h>
#include <unistd.h>
#if (defined __GLIBC__ && __GLIBC__ >= 2) || defined __CYGWIN__
#include <crypt.h>
#endif
#include "general_headers.h"
#include "telnet.h"
#include "who.h"
#include "version.h"
#include "string_misc.h"
#include "loadsave_user.h"
#include "softboot.h"
#include "banning.h"
#include "filter.h"
#include "more.h"
#include "login.h"

int unterminated_prompt(UR_OBJECT user, char* text)
{
   write_user(user,text);
   telnet_eor_send(user);
   return 0;
}

/*** Count up attempts made by user to login ***/
int attempts(UR_OBJECT user)
{
   user->attempts++;
   if (user->attempts==3) 
   {
      write_user(user,"\nI'm sorry, it appears as though you're having trouble.  ");
      write_user(user,"Please try again at a later date or e-mail pod@tursiops.org for assistance.\n\n");
      disconnect_user(user);
      return 0;
   }
   user->login=LOGIN_CHECK_NAME;
   user->pass[0]='\0';
   unterminated_prompt(user,"User login: ");
   sendEchoOn(user);
   return 0;
}

bool is_in_file3( pod_string filename, pod_string str)
{
   std::ifstream input;
   
   input.open( filename.c_str() );
   pod_string buffer;
   
   while(!input.eof())
   {
      input >> buffer;
      if(buffer == str)
      {
         input.close();
         return true;
      }
   }
   input.close();
   
   return false;
}

int login(UR_OBJECT user, char *inpstr)
{
   UR_OBJECT u;
   char input[ARR_SIZE];
   char filename[80];
   time_t boot = firstBootTime.get();

   input[0]='\0';
   sscanf(inpstr,"%s",input);
   if (!strcasecmp(input,"exit")) 
   {
      write_user(user,"\n\n*** Abandoning login attempt ***\n\n");
      /*write_system_admf("~OL~FR-- quit  ~RS~OL: %s ~FR--~RS\n",user->site);*/
      disconnect_user(user); 
      return 0;
   }
   switch(user->login) 
   {
      case LOGIN_CHECK_NAME:
      if(input[0]<33) 
      {
         unterminated_prompt(user,"\nUser login: ");
      }
      else if (!strcmp(input,"quit"))
      {
         write_user(user,"\n\n*** Abandoning login attempt ***\n\n");
         write_system_admf("~OL~FR-- quit  ~RS~OL: %s ~FR--~RS\n",user->socket->getPeerSite().c_str());
         disconnect_user(user); 
      }
      else if (!strcmp(input,"who")) 
      {
         if( site_banned( user->socket->getPeerSite() ) )
         {
            write_userf(user,"Your domain is banned. You are not allowed to use the \'who\' command from the\nlogin.\n");
            write_syslogf("Attempted who from banned site %s.\n",TRUE,user->socket->getPeerSite().c_str());
            write_system_admf("~OL~FR-- who   ~RS~OL: %s ~FR(banned!) --~RS\n",user->socket->getPeerSite().c_str());
         }
         else 
         {
            who(user,0);
            write_syslogf("who on port %d from %s:%d.\n",TRUE,user->socket->getLocalPort(),user->socket->getPeerSite().c_str(),user->socket->getPeerPort() );
            write_system_admf("~OL~FR-- who   ~RS~OL: %s ~FR--~RS\n",user->socket->getPeerSite().c_str() );
         }
         unterminated_prompt(user,"\nUser login: ");
      }
      else if (!strcmp(input,"stats_info"))
      {
         char dummy[30];

         pod_string filename = "datafiles/ignorelogin";
         if(!is_in_file3(filename,user->socket->getPeerSite()))
         {
            write_system_admf("~OL~FR-- stats_info   ~RS~OL: %s ~FR--~RS\n",user->socket->getPeerSite().c_str());
         }

         write_userf(user,"\n");
         write_userf(user,"url: http:////pod.tursiops.org\n");
         write_userf(user,"contact: pod@tursiops.org\n");
         write_userf(user,"connect: telnet:////tursiops.org:7300\n");
//altconnect: proto://address/etc (Alternative connect address - may be telnet:// or http://)
         write_userf(user,"uptime: %d\n",(int)(time(0)-firstBootTime.get()));
         strftime(dummy, 16, "%Y%m%d %H%M%S", localtime(&boot) );
         write_userf(user,"bootdate: %s\n", dummy);
//totalnewbielogins: integer (Most newbies you have had on)
         write_userf(user,"totallogins: %d\n",total_logins.get());
         write_userf(user,"newbies: yes\n");
         write_userf(user,"residents: yes\n");
         write_userf(user,"users: %d\n",get_num_of_users());
         write_userf(user,"maxusers: %d\n",max_users.get());
//         write_userf(user,"\n");
//maximumusers: integer (Most users on this reboot)
//maximumusers_date: YYYYMMDD HHMMSS (Date and Time attained)
//maximumeverusers: integer (Most users you have ever had on)
//maximumeverusers_date: YYYYMMDD HHMMSS (Date attained)
/*
               Talker devoted to those that love cetaceans and the oceans with all their heart. Those not interested in dolphins need not apply. Urgent action issues are discussed as well as a FAMILY based closeness of users.
*/
         write_userf(user,"description: PODnuts, a talker for dolphins! :)\n");

         disconnect_user(user);
      }
      else if (!strcmp(input,"version"))
      {
         show_version(user);
         write_system_admf("~OL~FR-- version   ~RS~OL: %s ~FR--~RS\n",user->socket->getPeerSite().c_str());
         unterminated_prompt(user,"\nUser login: ");
      }
      else if (!strcasecmp(input,"newuser"))
      {
         if (site_banned(user->socket->getPeerSite()))
         {
            sprintf(filename,"%s/%s",DATAFILES,BANMSGFILE);             
            if( !more_ext(user,NULL,filename,FALSE) ) write_user(user,"Unable to find the banmessage file.\n");
            write_syslogf("Attempted login by banned user %s from site %s.\n",TRUE,user->name,user->socket->getPeerSite().c_str());
            disconnect_user(user);
         }
            else if (user->socket->getLocalPort() == socketInterface.getListenPortNumber("W"))
            {
               write_user(user,"\nSorry, new logins cannot be created on this port.\n\n");
               write_syslogf("Attempted new user login on wiz port with name %s from site %s.\n",TRUE,user->name,user->socket->getPeerSite().c_str());
               disconnect_user(user);
            }
            else if (minlogin_level.get()>-1)
            {
               sprintf(filename,"%s/%s",DATAFILES,MINL_NEW_MSGFILE);
               if( !more_ext(user,NULL,filename,FALSE) ) write_user(user,"\nSorry, new logins cannot be created at this time.\n\n");
               write_syslogf("Attempted new user login with name %s from site %s, refused by minlogin level.\n",TRUE,user->name,user->socket->getPeerSite().c_str());
               disconnect_user(user);
            }
            else
            {
               write_system_admf("~OL~FR-- newbie ~RS~OL: ~FY%s~RS~OL : %s ~FR--~RS\n",user->name,user->socket->getPeerSite().c_str());
               write_user(user,"\n");
               more_ext(user,NULL,"datafiles/login1",FALSE);
               unterminated_prompt(user,"Hit <ENTER> to continue:");
               user->login=LOGIN_NEWBIE_1;
            }
      }
      else if(strlen(input)<3)
      {
         write_user(user,"\nName too short.\n");
         write_user(user,"Usernames must be larger than 3 characters and smaller than 12.\n\n");
         attempts(user);
      }
      else if (strlen(input)>USER_NAME_LEN) 
      {
         write_user(user,"\nName too long.\n");
         write_user(user,"Usernames must be larger than 3 characters and smaller than 12.\n\n");
         attempts(user);
      }
      else if(!is_alpha_str(input))
      {
         write_user(user,"\nPlease only use letters in your username.\n\n");
         attempts(user);  
      }
      else
      {
         if (!allow_caps_in_name.get()) strtolower(input);
         input[0]=toupper(input[0]);
         if (user_banned(input)) 
         {
            write_user(user,"\nYou are banned from this talker.\n\n");
            write_syslogf("Attempted login by banned user %s from site %s.\n",TRUE,input,user->socket->getPeerSite().c_str());
            disconnect_user(user);
            return 0;
         }
         strcpy(user->name,input);
         /* If user has hung on another login clear that session */
         for(u=user_first;u!=NULL;u=u->next)
         {
            if (u->login && u!=user && !strcmp(u->name,user->name))
            {
               disconnect_user(u);
               break;
            }
         }
         if (!load_user(user))  /* new user */
         {    
            if (site_banned(user->socket->getPeerSite()))
            {
               sprintf(filename,"%s/%s",DATAFILES,BANMSGFILE);
               if (!more_ext(user,NULL,filename,FALSE))
               {
                  write_user(user,"Unable to find the banmessage file.\n");
               }
               write_syslogf("Attempted login by banned user %s from site %s.\n",TRUE,user->name,user->socket->getPeerSite().c_str());
               disconnect_user(user);
               return 0;
            }
            write_user(user,"\nThat is not a recognized username.\n");
            write_user(user,"Login using 'newuser' to create a new account.\n\n");
            write_syslogf("Attempted login with non existing username :%s from site %s.\n",TRUE,user->name,user->socket->getPeerSite().c_str());
            /*write_system_admf("~OL~FR-- quit  ~RS~OL: %s ~FR--~RS\n",user->site);*/
            attempts(user);
            return 0;
         }
         else /* existing user */
         {    
            if (user->socket->getLocalPort() == socketInterface.getListenPortNumber("W") && user->level<wizport_level.get())
            {
               write_userf(user,"\nSorry, only users of level %s and above can log in on this port.\n\n",getLevelName(wizport_level.get()));
               disconnect_user(user);  
            }
            else if (user->level<minlogin_level.get()) 
            {
               write_user(user,"\nSorry, the talker is locked out to users of your level.\n\n");
               disconnect_user(user);  
            }
            else
            {
               write_system_admf("~OL~FR-- passwd ~RS~OL: ~FY%s~RS~OL : %s ~FR--~RS\n",user->name,user->socket->getPeerSite().c_str());
               unterminated_prompt(user,"User Password: ");
               echo_off(user);
               user->login=LOGIN_CHECK_PW;
            }
         }
      }
      return 0;

      case LOGIN_CHECK_PW:
         if (strlen(input)<3)
         {
            write_user(user,"\n\nPassword too short.\n\n");
            attempts(user);
            return 0;
         }
         if (strlen(input)>PASS_LEN)
         {
            write_user(user,"\n\nPassword too long.\n\n");
            attempts(user);
            return 0;
         }
         if (!strcmp(user->pass,(char *)crypt(input,"NU")))
         {
            sendEchoOn(user);
            connect_user(user);
            return 0;
         }
         write_user(user,"\n\nIncorrect login.\n\n");
         attempts(user);

         filter_passwd(user);
         return 0;

      case LOGIN_NEWBIE_1:            
            more_ext(user,NULL,"datafiles/rules1",FALSE);
            unterminated_prompt(user,"Hit <ENTER> to continue:");
            user->login=LOGIN_NEWBIE_2;
            return 0;

/*      case LOGIN_NEWBIE_2:
            write_userf(user,"Are you sure you want to continue into POD using the name '%s'?\n\n",user->name);
            unterminated_prompt(user,"Answer Y or N: ");
            user->login=LOGIN_NEWBIE_3;
            return 0;*/

      case LOGIN_NEWBIE_2:
            write_userf(user,"\nPick a username to identify yourself to others and to the system.\n");
            write_userf(user,"3-11 characters in lower-case are recommended.  Avoid funny characters\n");
            write_userf(user,"or long, hard to remember or hard to type usernames...\n");
            write_user(user,"\nWhat username would you like?\n");
            user->login=LOGIN_NEWBIE_3_2;
            return 0;

      case LOGIN_NEWBIE_3:
            if (!strcasecmp(input,"n") || !strcasecmp(input,"no"))
            {
               write_user(user,"\nWhat username would you like?\n");
//               unterminated_prompt(user,"\nEnter the name you wish to use : ");
               user->login=LOGIN_NEWBIE_3_2;
               return 0;
            }
            else if (!strcasecmp(input,"y") || !strcasecmp(input,"yes")) 
            {
               write_userf(user,"\nTo protect your account and make it your own, you will need to enter a password to ensure that only you can use it.\n\n");
               unterminated_prompt(user,"Enter password : ");
               echo_off(user);
               user->login=LOGIN_NEWBIE_4;
               return 0;
            }
            write_userf(user,"\nWarning, unrecognized reply '%s'.\n\n",input);
            unterminated_prompt(user,"Answer Y or N: ");
            return 0;

      case LOGIN_NEWBIE_3_2:
           if(input[0]<33) 
           {
               write_user(user,"\nWhat username would you like?\n");
//              unterminated_prompt(user,"\nEnter the name you wish to use : ");
           }
           else if(strlen(input)<3) 
           {
              write_user(user,"\nName too short.\n\n");  
              write_user(user,"\nWhat username would you like?\n");
//              unterminated_prompt(user,"\nEnter the name you wish to use : ");
           }
           else if (strlen(input)>USER_NAME_LEN) 
           {
              write_user(user,"\nName too long.\n\n");
              write_user(user,"\nWhat username would you like?\n");
//              unterminated_prompt(user,"\nEnter the name you wish to use : ");
           }
           else if(!is_alpha_str(input))
           {
              write_user(user,"\nOnly letters are allowed in a name.\n\n");
              write_user(user,"\nWhat username would you like?\n");
//              unterminated_prompt(user,"\nEnter the name you wish to use : ");
           }
           else
           {
              pod_string nameString = input;
              if (!allow_caps_in_name.get()) 
              {
                 strToLower(nameString);
              }
              capitalize(nameString);
              if (user_banned((char*)nameString.c_str())) 
              {
                 write_user(user,"\nYou are banned from this talker.\n\n");
                 write_syslogf("Attempted login by banned user %s from site %s.\n",TRUE,input,user->socket->getPeerSite().c_str());
                 disconnect_user(user);
                 return 0;
              }
              strcpy(user->name,nameString.c_str());
              /* If user has hung on another login clear that session */
              for(u=user_first;u!=NULL;u=u->next) 
              {
                 if (u->login && u!=user && !strcmp(u->name,user->name)) 
                 {
                    disconnect_user(u);  
                    break;
                 }
              }
              if (!user_exists(user)) 
              {
                 write_system_admf("~OL~FR-- newbie ~RS~OL: ~FY%s~RS~OL : %s ~FR--~RS\n",user->name,user->socket->getPeerSite().c_str());
                 write_userf(user,"Are you sure you want to continue into POD using the name '%s'?\n\n",user->name);
                 unterminated_prompt(user,"Answer Y or N: ");
                 user->login=LOGIN_NEWBIE_3;
/*                 write_userf(user,"\nTo protect your account and make it your own, you will need to enter a password to ensure that only you can use it.\n\n");
                 unterminated_prompt(user,"Enter password : ");
                 echo_off(user);
                 user->login=LOGIN_NEWBIE_4;*/
              }
              else
              {
               write_user(user,"\nInvalid name, please enter an un-used username\n\n");
               write_user(user,"\nWhat username would you like?\n");
//               unterminated_prompt(user,"\nEnter the name you wish to use : ");
               user->login=LOGIN_NEWBIE_3_2;
              }
           }
          return 0;             

      case LOGIN_NEWBIE_4:
         if (strlen(input)<3) 
         {
            write_user(user,"\n\nPassword too short.\n\n");  
            unterminated_prompt(user,"Enter password : ");
            return 0;
         }
         if (strlen(input)>PASS_LEN) 
         {
            write_user(user,"\n\nPassword too long.\n\n");
            unterminated_prompt(user,"Enter password : ");
            return 0;
         }
         strcpy(user->pass,(char *)crypt(input,"NU"));
         write_system_admf("~OL~FR-- pass_valid  ~RS~OL: ~FY%s~RS~OL : %s ~FR--~RS\n",user->name,user->socket->getPeerSite().c_str());
         unterminated_prompt(user,"\n\nType again to verify : ");
         user->login=LOGIN_NEWBIE_5;
         
         return 0;
         
      case LOGIN_NEWBIE_5:
         if (strcmp(user->pass,(char*)crypt(input,"NU"))) 
         {
            write_user(user,"\n\nPasswords do not match.\n\n");
            unterminated_prompt(user,"Enter password : ");
            user->login=LOGIN_NEWBIE_4;
            return 0;
         }
         sendEchoOn(user);
         write_user(user,"\n\nPOD requires that you enter your gender.\n");
         write_user(user,"This is used solely for the purpose of correct english and grammar.\n\n");
         unterminated_prompt(user,"Enter (M)ale or (F)emale: ");
         user->login=LOGIN_NEWBIE_6;
         return 0;

      case LOGIN_NEWBIE_6:
      {
         StrGlobalVar *gend_choice   = (StrGlobalVar*)user->getVar("GendChoice");
         if (!strcasecmp(input,"m") || !strcasecmp(input,"male"))
         {
            gend_choice->set("male");
            write_user(user,"Gender set to Male.\n\n");
            more_ext(user,NULL,"datafiles/disclaimer",FALSE);
            unterminated_prompt(user,"\nEnter 'continue' or 'end' : ");
            user->login=LOGIN_NEWBIE_7;
            return 0;
         }
         if (!strcasecmp(input,"f") || !strcasecmp(input,"female"))
         {
            gend_choice->set("female");
            write_user(user,"Gender set to Female.\n\n");
            more_ext(user,NULL,"datafiles/disclaimer",FALSE);
            unterminated_prompt(user,"Enter 'continue' or 'end' : ");
            user->login=LOGIN_NEWBIE_7;
            return 0;
         }
         write_userf(user,"\nWarning, unrecognized reply '%s'.\n\n",input);
         unterminated_prompt(user,"Enter (M)ale or (F)emale: ");
         return 0;
      }

      case LOGIN_NEWBIE_7:
         if (!strcasecmp(input,"end"))
         {
            write_user(user,"\n\n*** Disconnecting now ***\n\n");
            /*write_system_admf("~OL~FR-- end  ~RS~OL: %s ~FR--~RS\n",user->site);*/
            disconnect_user(user);
            return 0;
         }
         else if (!strcasecmp(input,"continue"))
         {
            more_ext(user,NULL,"datafiles/login2",FALSE);
            unterminated_prompt(user,"Hit <ENTER> to continue:");
            user->login=LOGIN_NEWBIE_8;
            return 0;
         }
         write_userf(user,"\nWarning, unrecognized reply '%s'.\n\n",input);
         unterminated_prompt(user,"Enter 'continue' or 'end' : ");
         return 0;

      case LOGIN_NEWBIE_8:
         new_init_user(user);
            
         save_user(user);
         
         write_syslogf("New user \"%s\" created.\n",TRUE,user->name);
         connect_user(user);
         return 0;
      
   }
   return 0;
}

