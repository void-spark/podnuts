#include <signal.h>
#include <setjmp.h>
#include <ctype.h>
#include <stdlib.h>
#include "general_headers.h"
#include "telnet.h"
#include "globals.h"
#include "string_misc.h"
#include "crash.h"
#include "shutdown.h"

jmp_buf jmpvar;
CrashActionGlobalVar crash_action("crash_action",BY_LOAD_CONFIG,CrashActionGlobalVar::CRASH_ACT_NONE);
StrGlobalVar rsUserName("rs_user_name",USER_NAME_LEN,EVERY_BOOT,"");

enum rs_which_types
{
   WHICH_NONE = -1,
   WHICH_SHUTDOWN,
   WHICH_REBOOT
};

int talker_exit(int shutdown)
{
   FILE *fp;

   fp=fopen("bootexit","w");
   if (shutdown) fprintf(fp,"SHUTDOWN\n");        
   else          fprintf(fp,"REBOOT\n");
   fclose (fp);

   exit(0);
}

/*** Shutdown the talker ***/
void talker_shutdown(const char *who_did_it_str, int reboot)
{
   if (reboot)
   {
      write_systemf("\07\n~OL~FR-- SYSTEM : ~LIRebooting now!\n\n");
      write_syslogf("*** REBOOT initiated by %s ***\n",FALSE,who_did_it_str);
   }
   else 
   {
      write_systemf("\07\n~OL~FR-- SYSTEM : ~LIShutting down now!\n\n");
      write_syslogf("*** SHUTDOWN initiated by %s ***\n",FALSE,who_did_it_str);
   }
   write_syslogf("*** Saving users...\n",FALSE);
   while(user_first)
   {
      quit(user_first);
   }
   write_syslogf("*** Users saved\n",FALSE);
   socketInterface.closeCycle();
   socketInterface.closeListenSockets();
   if (reboot)
   {
      write_syslogf("*** REBOOT complete %s\n\n",FALSE,long_date(1).c_str());
      talker_exit(0);
   }
   write_syslogf("*** SHUTDOWN complete %s ***\n\n",FALSE,long_date(1).c_str());
   talker_exit(1);
}


/*** Talker signal handler function. Can either shutdown , ignore or reboot
	if a unix error occurs though if we ignore it we're living on borrowed
	time as usually it will crash completely after a while anyway. ***/
void sig_handler(int sig)
{
   force_listen=TRUE;
   static int crashcnt=0;
   switch(sig)
   {
      case SIGTERM:
         if (ignore_sigterm.get())
         {
            write_syslog("SIGTERM signal received - ignoring.\n",TRUE);
            return;
         }
         crash_writelog();
         write_room(NULL,"\n\n~OLSYSTEM:~FR~LI SIGTERM received, initiating shutdown!\n\n");
         talker_shutdown("a termination signal (SIGTERM)",WHICH_SHUTDOWN);

      case SIGSEGV:
         crash_writelog();
         if(crashcnt++ > 2) abort(); // ouch
         switch(crash_action.get())
         {
            case crash_action.CRASH_ACT_NONE:
               write_room(NULL,"\n\n\07~OLSYSTEM:~FR~LI PANIC - Segmentation fault, initiating shutdown!\n\n");
               talker_shutdown("a segmentation fault (SIGSEGV)",WHICH_SHUTDOWN);

            case crash_action.CRASH_ACT_REBOOT:
               write_room(NULL,"\n\n\07~OLSYSTEM:~FR~LI PANIC - Segmentation fault, initiating reboot!\n\n");
               talker_shutdown("a segmentation fault (SIGSEGV)",WHICH_REBOOT);

            case crash_action.CRASH_ACT_IGNORE:
               write_room(NULL,"\n\n\07~OLSYSTEM:~FR~LI WARNING - A segmentation fault has just occured!\n\n");
               write_syslog("WARNING: A segmentation fault occured!\n",1);
               longjmp(jmpvar,0);
         }

      case SIGBUS:
         crash_writelog();
         if(crashcnt++ > 2) abort(); // ouch
         switch(crash_action.get())
         {
            case crash_action.CRASH_ACT_NONE:
               write_room(NULL,"\n\n\07~OLSYSTEM:~FR~LI PANIC - Bus error, initiating shutdown!\n\n");
               talker_shutdown("a bus error (SIGBUS)",WHICH_SHUTDOWN);

            case crash_action.CRASH_ACT_REBOOT:
               write_room(NULL,"\n\n\07~OLSYSTEM:~FR~LI PANIC - Bus error, initiating reboot!\n\n");
               talker_shutdown("a bus error (SIGBUS)",WHICH_REBOOT);

            case crash_action.CRASH_ACT_IGNORE:
               write_room(NULL,"\n\n\07~OLSYSTEM:~FR~LI WARNING - A bus error has just occured!\n\n");
               write_syslog("WARNING: A bus error occured!\n",TRUE);
               longjmp(jmpvar,0);
         }
   }
}

void misc_shutreboot(UR_OBJECT user,int which)
{
   if (toupper(words.word[0][0])=='Y')
   {
      const char *type = which ? "REBOOT" : "SHUTDOWN";
      if ( rs_countdown.get() && rs_which == which )
      {
         if (rs_countdown.get() > 60) write_systemf("\n\07~OLSYSTEM: %s~LI%s INITIATED, rebooting in %d minutes, %d seconds!\n\n",which ? "~FY" : "~FR" ,type,rs_countdown.get()/60,rs_countdown.get()%60);
         else write_systemf("\n\07~OLSYSTEM: ~FY~LI%s INITIATED, rebooting in %d seconds!\n\n",type,rs_countdown.get());
         write_syslogf("%s initiated a %d seconds %s countdown.\n",TRUE,user->name,rs_countdown.get(),type);
         rsUserName.set(user->name);
         resetAnnounce.setCurrentTime();
         user->nextCommand = 0;
         prompt(user);
         return;
      }
      talker_shutdown(user->name,which);
   }
   rs_countdown.set(0);
   resetAnnounce.set(0);
   rs_which.set(WHICH_NONE);
   rsUserName.set("");
   user->nextCommand = 0;
   prompt(user);
   return;
}

/*** Shutdown talker interface func. Countdown time is entered in seconds so
	we can specify less than a minute till reboot. ***/
/*** Reboot talker interface func. ***/
// call with WHICH_SHUTDOWN or WHICH_REBOOT
int shutreboot_com(UR_OBJECT user,int which)
{
   const char *type;
   if(rs_which.get() != WHICH_NONE && rs_which.get() != which)
   {
      write_userf(user,"The %s countdown is currently active, you must cancel it first.\n",which ? "shutdown" : "reboot" );
      return 0;
   }
   type = which ? "reboot" : "shutdown";
   if (!strcmp(words.word[1],"cancel"))
   {
      if (!rs_countdown.get())
      {
         write_userf(user,"The %s countdown is not currently active.\n",type);
         return 0;
      }
      if (rs_countdown.get() && rsUserName.isEmpty())
      {
         write_userf(user,"Someone else is currently setting the %s countdown.\n",type);
         return 0;
      }
      write_systemf("~OLSYSTEM:~RS~FG %s cancelled.\n",type);
      write_syslogf("%s cancelled the %s countdown.\n",TRUE,user->name,type);
      rs_countdown.set(0);
      resetAnnounce.set(0);
      rs_which.set(WHICH_NONE);
      rsUserName.set("");
      return 0;
   }
   if (words.word_count>1 && !isNumber(words.word[1]))
   {
      write_userf(user,"Usage: %s [<secs>/cancel]\n",type);
      return 0;
   }
   if (rs_countdown.get())
   {
      write_userf(user,"The %s countdown is currently active, you must cancel it first.\n",type);
      return 0;
   }
   if (words.word_count<2)
   {
      rs_countdown.set(0);
      resetAnnounce.set(0);
      rs_which.set(WHICH_NONE);
      rsUserName.set("");
   }
   else
   {
      rs_countdown.set(atoi(words.word[1]));
      rs_which.set(which);
   }
   write_userf(user,"\n\07%s~OL~LI*** WARNING - This will %s the talker! ***\n\nAre you sure about this (y/n)? ",which ? "~FY" : "~FR",type);
   telnet_eor_send(user);
   user->nextCommand = currentCommand;
   no_prompt=1;
   return 0;
}

int reboot_com(UR_OBJECT user)
{
   if(user->nextCommand)
   {
      misc_shutreboot(user,WHICH_REBOOT);
      return 0;
   }
   return shutreboot_com(user,WHICH_REBOOT);
}

int shutdown_com(UR_OBJECT user)
{
   if(user->nextCommand)
   {
      misc_shutreboot(user,WHICH_SHUTDOWN);
      return 0;
   }
   return shutreboot_com(user,WHICH_SHUTDOWN);
}

/*** See if timed reboot or shutdown is underway ***/
int check_reboot_shutdown()
{
   int secs;
   char *w[]={ "~FRShutdown","~FYRebooting" };

   if (rsUserName.isEmpty()) return 0;
   rs_countdown.decrease(heartbeat.get());
   if (rs_countdown.get()<=0) talker_shutdown(rsUserName.get().c_str(),rs_which.get());

   /* Print countdown message every minute unless we have less than 1 minute
      to go when we print every 10 secs */
   secs=(int)( time(0) - resetAnnounce.get() );
   if (rs_countdown.get()>=60 && secs>=60) 
   {
      write_systemf("~OLSYSTEM: %s in %d minutes, %d seconds.\n",w[rs_which.get()],rs_countdown.get()/60,rs_countdown.get()%60);
      resetAnnounce.setCurrentTime();
   }
   if (rs_countdown.get()<60 && secs>=10) 
   {
      write_systemf("~OLSYSTEM: %s in %d seconds.\n",w[rs_which.get()],rs_countdown.get());
      resetAnnounce.setCurrentTime();
   }
   return 0;
}

char *crash_action_txt()
{
   char *ca[]={ "NONE","IGNORE","REBOOT" };
   return ca[crash_action.get()];
}
