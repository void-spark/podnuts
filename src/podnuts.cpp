/*****************************************************************************

            ______   _______   _____                   __
           |   __ \ |       | |     \    .-----.--.--.|  |_.-----.
           |    __/ |   -   | |  --  |   |     |  |  ||   _|__ --|
           |___|    |_______| |_____/____|__|__|_____||____|_____|
                                   |______|
                 
    Based on rNUTS 3.0.2 by Engi & Slugger which is a heavily modified
           form of NUTS version 3.3.3 (C)Neil Robertson 1996
       Neil Robertson                Email    : neil@ogham.demon.co.uk

                  POD_nuts coding by Vaghn & Crandonkphin
                         Extra assistance by Dolfin
                               VERSION 2.20.1

                Email     : pod@ncohafmuta.com
             Homepage     : http://pod.ncohafmuta.com/

cvs id : $Id: podnuts.cpp 384 2004-01-21 17:10:41Z pod $

*****************************************************************************/

/* includes (moved crandonkphin) */
#include <stdlib.h>
#ifdef _AIX
#include <sys/select.h>
#endif
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <unistd.h>
#include <setjmp.h>
#include <dlfcn.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <sys/wait.h>
#include <iostream>
#include <iomanip>

#if (defined __GLIBC__ && __GLIBC__ >= 2) || defined __CYGWIN__
#include <crypt.h>
#endif

#include "GlobalVars.h"
#include "socket_funcs.h"
#include "wordfind.h"
#include "Room.h"
#include "macro.h"
#include "file_locations.h"
#include "levels.h"
#include "talker_constants.h"
#include "podnuts.h"
#include "user_objects.h"
#include "xalloc.h"
#include "globals.h"
#include "color.h"
#include "time_utils.h"
#include "string_misc.h"
#include "logging.h"
#include "telnet.h"
#include "loadsave_config.h"
#include "loadsave_user.h"
#include "admin.h"
#include "set.h"
#include "crash.h"
#include "help.h"
#include "banning.h"
#include <sys/uio.h>
#include "more.h"
#include "who.h"
#include "dyn_str.h"
#include "clones.h"
#include "examine_stat.h"
#include "filter.h"
#include "account_reg.h"
#include "parse.h"
#include "speech_funcs.h"
#include "boards.h"
#include "tandem.h"
#include "info_port.h"
#include "MailPort.h"
#include "system.h"
#include "bot.h"
#include "login.h"
#include "softboot.h"
#include "smail.h"
#include "curse.h"
#include "cmd_main.h"
#include "move.h"
#include "ignore.h"
#include "hist.h"
#include "dns_socket.h"
#include "shutdown.h"
#include "UserTelnetHandler.h"
#include "UserTelnetSockObjectCreator.h"
#include "StringLibrary.h"
#include "gender.h"
#include "version.h"
/* end of includes  */

#define GLB_STR_FILE "datafiles/glob_strings"

/* variables moved from podnuts.h to podnuts.c by crandonkphin */
/* global variables */

UR_OBJECT user_first,user_last;
RM_OBJECT room_first,room_last;

char **glob_argv;

int do_events_now;

class SpecialRoomNames globalSpecialRoomNames;

LevelOrNoneGlobalVar minlogin_level    ( "minlogin_level",    BY_LOAD_CONFIG, -1  );
OnOffGlobalVar       system_logging    ( "system_logging",    BY_LOAD_CONFIG, OnOffGlobalVar::ON   );
YesNoGlobalVar       ban_swearing      ( "ban_swearing",      BY_LOAD_CONFIG, YesNoGlobalVar::NO  );
NewOldGlobalVar      nuts_talk_style   ( "nuts_talk_style",   BY_LOAD_CONFIG, NewOldGlobalVar::NEW );
IntGlobalVar         forwarding        ( "forwarding",        EVERY_BOOT,     1   );
OnOffGlobalVar       atmos_on          ( "atmos_on",          BY_LOAD_CONFIG, OnOffGlobalVar::ON   );
TimeTGlobalVar       resetAnnounce     ( "rs_announce",       EVERY_BOOT,     0   );
IntGlobalVar         rs_countdown      ( "rs_countdown",      EVERY_BOOT,     0   );
IntGlobalVar         rs_which          ( "rs_which",          EVERY_BOOT,     -1  );
IntGlobalVar         num_recent_users  ( "num_recent_users",  EVERY_BOOT,     0   );
LimitedIntGlobalVar  max_users         ( "max_users",         BY_LOAD_CONFIG, 50,  1,  -1 );
LimitedIntGlobalVar  mesg_life         ( "mesg_life",         BY_LOAD_CONFIG, 1,   1,  -1 );
LimitedIntGlobalVar  min_private_users ( "min_private",       BY_LOAD_CONFIG, 2,   1,  -1 );
LimitedIntGlobalVar  heartbeat         ( "heartbeat",         BY_LOAD_CONFIG, 2,   1,  -1 );
LimitedIntGlobalVar  login_idle_time   ( "login_idle_time",   BY_LOAD_CONFIG, 180, 10, -1 );
LimitedIntGlobalVar  newbie_idle_time  ( "newbie_idle_time",  BY_LOAD_CONFIG, 150, 5,  -1 );
LimitedIntGlobalVar  user_idle_time    ( "user_idle_time",    BY_LOAD_CONFIG, 300, 10, -1 );
LimitedIntGlobalVar  max_clones        ( "max_clones",        BY_LOAD_CONFIG, 1,   0,  -1 );
LimitedIntGlobalVar  atmos_delay       ( "atmos_delay",       BY_LOAD_CONFIG, 30,  10, -1 );
LimitedIntGlobalVar  ping_delay        ( "ping_delay",        BY_LOAD_CONFIG, 15,  10, -1 );
LimitedIntGlobalVar  purgedays         ( "purgedays",         BY_LOAD_CONFIG, 30,  5,  -1 );
OnOffGlobalVar       color_on_def      ( "color_on_def",      BY_LOAD_CONFIG,OnOffGlobalVar::ON    );
OnOffGlobalVar       prompt_def        ( "prompt_def",        BY_LOAD_CONFIG,OnOffGlobalVar::ON    );
YesNoGlobalVar       password_echo     ( "password_echo",     BY_LOAD_CONFIG,YesNoGlobalVar::NO    );
YesNoGlobalVar       ignore_sigterm    ( "ignore_sigterm",    BY_LOAD_CONFIG,YesNoGlobalVar::NO    );
YesNoGlobalVar       time_out_afks     ( "time_out_afks",     BY_LOAD_CONFIG,YesNoGlobalVar::NO    );
YesNoGlobalVar       allow_caps_in_name( "allow_caps_in_name",BY_LOAD_CONFIG,YesNoGlobalVar::YES   );
YesNoGlobalVar       do_fork           ( "fork",              BY_LOAD_CONFIG,YesNoGlobalVar::YES   );
LevelGlobalVar       time_out_maxlevel ( "time_out_maxlevel", BY_LOAD_CONFIG, LEV_THR );
LevelGlobalVar       wizport_level     ( "wizport_level",     BY_LOAD_CONFIG, LEV_FOU );
LevelGlobalVar       gatecrash_level   ( "gatecrash_level",   BY_LOAD_CONFIG, LEV_FIV );
LevelGlobalVar       ignore_mp_level   ( "ignore_mp_level",   BY_LOAD_CONFIG, LEV_FOU );
DayTimeGlobalVar     mesg_check_time   ( "mesg_check_time",   BY_LOAD_CONFIG, 0,0 );
strArrGlobVar        logged_users      ( "logged_users",      EVERY_BOOT, LOG_LINES, "" );
StrGlobalVar         default_color     ( "default_color",     COLOR_LEN, BY_LOAD_CONFIG, "~FW");

int config_line;
int no_prompt;
int force_listen;
int curr_user_destructed;
UR_OBJECT curr_user;
int keepalive_interval;
int temp_user_count;
time_t boot_time;

StringsArray mood_list;

int login_cnt;

char *getLevelName(int which)
{
   char *level_name[]  = { "Outcast"     ,
                           "Calf"  ,
                           "Transient",
                           "Member"   ,
                           "Elder"    ,
                           "Shaman"   ,
                           "Leader"   ,
                           "Bot",
                           "*"        };
   which++; // -1 = Calf, 0 = Outcast, etc
   if(which < 0 || which >= (int)(sizeof(level_name)/sizeof(char*)) )
   {
      abort();
   }
   return(level_name[which]);
}

char *getLevelShortName(int which)
{
   char *level_short_name[] = { "OUTC",
                                "CALF",
                                "TRAN",
                                "MEMB",
                                "ELDR",
                                "SHAM",
                                "LEDR",
                                "BOT",  /* no spaces, screws up some code */
                                "*"     };
   which++; // -1 = Calf, 0 = Outcast, etc
   if(which < 0 || which >= (int)(sizeof(level_short_name)/sizeof(char*)) )
   {
      abort();
   }
   return(level_short_name[which]);
}

char *noyes1[]={ " NO","YES" };
char *noyes2[]={ "NO ","YES" };
char *offon[]={ "OFF","ON " };

/* local function prototypes */

int        get_charclient_line(UR_OBJECT user, pod_string &inpstr);
int        init_connections();
int        init_globals();
int        init_signals();
void       init_user(UR_OBJECT user);
int        reset_alarm();
void       sig_handler(int sig);
int        user_onlist_visible(UR_OBJECT user);
void       do_events();

void show_uptime(UR_OBJECT user)
{
   char bstr[40];
   time_t uptime;
       
   /* Get some values */      
   strcpy(bstr,ctime(&boot_time));
   uptime=(int)(time(0)-boot_time);
        
   write_userf(user,"~FTTalker booted : ~FG%s",bstr);
   write_userf(user,"~FTUptime        : ~FG%s\n",time2string(TRUE,uptime).c_str() );
}

void write_seperator_line(UR_OBJECT user,char *string)
{
   if(!string)
   {
      string = "";
   }
   write_user_crt(user,gen_seperator_line(string).c_str());

   return;
}

int secret_room_vis(UR_OBJECT looker,UR_OBJECT looked_at )
{
   if ( (looker->room!=looked_at->room) &&   
        (looker->level < LEV_THR)   &&
        (looker->level <= looked_at->level) )
   { 
      if(looked_at->room!=NULL) /* if not looking at a user in login fase (cause checking rooms secret status would do crash then) */
      {
         if(looked_at->room->secret) return 0; /* if that user in secret room (duh :) */
      }                                    /* then don't show him :) */
   }
   return 1;
}


int unlink_checked(const char *filename, char *caller)
{
   if(unlink(filename) == -1)
   {
      write_syslogf("Error in %s while unlinking file '%s' : '%s'.\n",TRUE,caller,filename,strerror(errno));
      return -1;
   }
   return 0;
}


int start_autorun(char *argv[])
{
   char *autorun_args[4];
   char autorun_cmd[80];
   char autorun_opt2[80];
   int pid;
   /* int status; */

   printf("-- BOOT : Starting autorun deamon\n");
   sprintf(autorun_cmd,"utils/autorun.ex");
   sprintf(autorun_opt2,"%d",getpid());

   pid = fork();
   if (pid == -1)
   {
     std::cerr << "          " << TALKER_NAME << ": Failed to fork autorun process, error: " << strerror( errno ) << ".\n";
     logStream << setLogfile( BOOTLOG ) << noTime << "BOOT FAILURE: Failed to fork autorun process, error: " << strerror(errno) << ".\n" << pod_send;
     return -1;
   }
   if(pid == 0)
   {
      autorun_args[0]=autorun_cmd;
      autorun_args[1]=argv[0];
      autorun_args[2]=autorun_opt2;
      autorun_args[3]=NULL;
      execv(autorun_cmd,autorun_args);
      logStream << setLogfile( SYSLOG ) << noTime << "BOOT FAILURE: Failed to execv autorun, error: " << strerror(errno) << ".\n" << pod_send;
      exit(133);
   }
   /* child process should fork right away, so wait for it to do that */
/*   do
   {
      if(waitpid(pid, &status, 0) == -1)
      {
         if (errno != EINTR) return -1;
      }
      else return 0;
   } while (1);*/
   return pid;
}
/*** This function calls all the setup routines and also contains the
	main program loop ***/

   char confile[40];
int main(int argc, char *argv[])
{
   char filename[80];
   int softbooted = FALSE;
   int autorunPid = -1;

   glob_argv=argv;
   
   if (argc<2) strcpy(confile,CONFIGFILE);
   else strcpy(confile,argv[1]);

#ifndef __CYGWIN__
   system("mv -f plugins_new/*.so plugins/");
#else
   system("mv -f plugins_new/*.dll plugins/");
#endif

   /* Startup */
   printf("\n-- BOOT : %s Version %s server\n\n",TALKER_NAME,RVERSION);
   init_signals();
   init_globals();
   /*try
   {*/
      StringLibrary::getInstance()->parseAllFiles();
   /*}
   catch( somerror e )
   {
      std::cout << "          " << TALKER_NAME << ": loading of glob strings failed\n";
      logStream << setLogfile( BOOTLOG ) << noTime << "BOOT FAILURE: Failed to load glob strings.\n" << pod_send;
      exit(113);
   }*/
   logStream << setLogfile( BOOTLOG ) << noTime << "\n-- SYSTEM   : SERVER BOOTING\n" << pod_send;
   check_messages(NULL,1);

   softbooted=softboot_restart();
   if(!softbooted) /* if not softbooting, do first init stuffs */
   {
      init_glob_firstboot_list();

      /* Run in background automatically. */
      if(do_fork.get())
      {
         switch(fork()) 
         {
            case -1: /* fork failure */
               std::cerr << "          " << TALKER_NAME << ": Failed to fork, error: " << strerror( errno ) << ".\n";
               logStream << setLogfile( BOOTLOG ) << noTime << "BOOT FAILURE: Failed to fork, error: " << strerror(errno) << ".\n" << pod_send;
               exit(11);
            case  0:
               std::cout << "-- BOOT : forked\n";
               break; /* child continues */
            default:
               sleep(2); /* wait a bit before returning the prompt */
               exit(0);  /* parent dies */
         }
      }
   }
   else write_systemf("\07~OLSYSTEM: ~FGWarm boot complete.\n");

   if(!softbooted) /* don't spawn it on softboot */
   {
      autorunPid = start_autorun(argv);
      if(autorunPid == -1)
      {
         exit(-13);
      }
   }

   if( socketInterface.initListenSockets() == -1)
   {
      if(!softbooted)
      {
         printf("-- BOOT : Shutting down autorun deamon\n");
         kill(autorunPid,SIGTERM);
      }
      exit(-13);
   }

   reset_alarm();
   
   /* this is only for testing purposes, don't uncomment it! 
      (speeds up the heartbeat to megafast ;) */
/*    {
      struct itimerval mytim;
      mytim.it_interval.tv_sec=0;
      mytim.it_interval.tv_usec=10;
      mytim.it_value.tv_sec=0;
      mytim.it_value.tv_usec=mytim.it_interval.tv_usec;
      setitimer(ITIMER_REAL,&mytim,NULL);
   } */

   std::cout << "\n-- BOOT : Booted with PID " << getpid() << " \n\n";
   logStream << setLogfile( BOOTLOG ) << noTime << "-- SYSTEM   : Booted successfully with PID " << getpid() << "\n" << pod_send;
   logStream << setLogfile( BOOTLOG ) << noTime << "-- SYSTEM   : " << long_date(1) << "\n\n" << pod_send;
   logStream << setLogfile( SYSLOG ) << noTime << "*** BOOT successfull PID " << getpid() << " " << long_date(1) << "\n\n" << pod_send;

   /* added call to load system macros. May, 1997 -- KnightShade */
   sprintf(filename,"%s/%s",DATAFILES,SYSMACROFILE);
   system_macrolist.loadMacros(filename);

   sprintf(filename,"%s/%s",DATAFILES,SYSACTIONFILE);
   system_actionlist.loadMacros(filename);

   checkConfig();
   /**** Main program loop. *****/
   setjmp(jmpvar); /* jump to here if we crash and crash_action = IGNORE */
   while(TRUE)
   {
      do_events(); /* check and execute timed events if neccesairy */
      socketInterface.handleRequests();
   }
}

int afk_check(UR_OBJECT user)
{
   if (user->afk)
   {
      if (user->afk==2)
      {
         if (!words.word_count)
         {
            if (user->command_mode) prompt(user);
            return 1;
         }
         if (strcmp((char *)crypt(words.word[0],"NU"),user->pass))
         {
            write_user(user,"Incorrect password.\n");
            prompt(user);
            return 1;
         }
         cls(user);
         write_user(user,"Session unlocked, you are no longer AFK.\n");
      }
      else write_user(user,"You are no longer AFK.\n");

      StrGlobalVar *afk_mesg    = (StrGlobalVar*)user->getVar("afk_mesg");
      afk_mesg->set("");

      pod_string name = get_visible_name(user);
      if (!user->cloaked) write_room_exceptf(user->room,user,"%s comes back from being AFK.\n", name.c_str() );
      if (user->afk==2)
      {
         user->afk=0;
         prompt(user);
         return 1;
      }
      user->afk=0;
   }
   return 0;
}

/* Is the user visible? --- Vaghn 1999 */
int user_onlist_visible(UR_OBJECT user)
{
   if (user->vis && !user->cloaked) return 1;
   return 0;
}

/* Return color when vis, white when not --- Crandonkphin 1999 */
char* get_visible_color(UR_OBJECT user)
{
   StrGlobalVar *color       = (StrGlobalVar*)user->getVar("Color");

   static char temp_color[COLOR_LEN+1];

   if(user->room == NULL)                                     strcpy(temp_color, color->get().c_str() );
   else if(user->vis && !user->cloaked && !user->room->inked) strcpy(temp_color, color->get().c_str() );
   else strcpy(temp_color, default_color.get().c_str() );
   return temp_color;
}


/* Return name or cloaked/invis alternative --- Crandonkphin 1999 */
pod_string get_visible_name(UR_OBJECT user)
{
   StringLibrary * stringLibrary = StringLibrary::getInstance();
   pod_string name;
   if(user->room->inked)
   {
      name  = stringLibrary->makeString("ink_name");   
   }
   else if(user->cloaked)
   {
      name  = stringLibrary->makeString("cloaked_name");
   }
   else if (!user->vis)
   {
      name  = stringLibrary->makeString("invisname");
   }
   else
   {
      name = user->name;
   }
   return name;
}

/* Return cloaked/invis/inked/no prechar --- Crandonkphin 1999 */
char get_visible_prechar(UR_OBJECT user)
{
   char prechar;
   
   if(user->room==NULL)prechar = 'x';
   else if(user->cloaked) prechar = '+';
   else if (!user->vis) prechar = '*';
   else prechar = ' ';
   return prechar;
}

/* Return sound or cloaked/invis alternative --- Crandonkphin 1999 */
void get_sound(UR_OBJECT user, const char *inpstr, char *type)
{
   StrGlobalVar *noise         = (StrGlobalVar*)user->getVar("Noise");

   if ( noise->get().length() != 0 ) strcpy( type, noise->get().c_str() );
   else 
   {
      switch(inpstr[strlen(inpstr)-1]) 
      {
         case '?': strcpy(type,"ask");  break;
         case '!': strcpy(type,"exclaim");  break;
         default : strcpy(type,"say");
      }
   }
}

pod_string get_visible_sound(UR_OBJECT user, pod_string inpstr)
{
   StringLibrary * stringLibrary = StringLibrary::getInstance();
   StrGlobalVar *noise         = (StrGlobalVar*)user->getVar("Noise");
   pod_string result;

   if(user->room->inked)                              result = stringLibrary->makeString("ink_noise");
   else if (user->cloaked)                            result = stringLibrary->makeString("cloaked_noise");
   else if (noise->get().length() != 0 && user->vis)  result = noise->get();
   else 
   {
      switch(inpstr[inpstr.size()-1]) 
      {
         case '?': 
            result="ask";
            break;
         case '!': 
            result="exclaim";
            break;
         default :
            result="say";
      }
   }

   return result;
}


int get_idle_mins(UR_OBJECT user)
{
   int no_input_time;
   int idle_time;
   
   if(user->level == LEV_BOT) return 0;

   no_input_time = (int)(time(0) - user->last_input)/60;
   idle_time = no_input_time - 5;
   if(idle_time < 0) idle_time = 0;
   
   return idle_time;
}

int get_idle_secs(UR_OBJECT user)
{
   int no_input_time;
   int idle_time;
   
   if(user->level == LEV_BOT) return 0;
   
   no_input_time = (int)(time(0) - user->last_input);
   idle_time = no_input_time - 5*60;
   if(idle_time < 0) idle_time = 0;
   
   return idle_time;
}

int is_idling(UR_OBJECT user)
{
   int no_input_time;
   
   if(user->level == LEV_BOT) return 0;
   
   no_input_time = (int)(time(0) - user->last_input)/60;
   if(no_input_time >= 5) return 1;
   
   return 0;
}

char* get_stat(UR_OBJECT u)
{
   static char idlestr[6];
   int idle=(int)(time(0) - u->last_input)/60;
   if (u->level == LEV_BOT)
   {
      strcpy(idlestr,"AWKE");
   }
   else if (u->inedit) strcpy(idlestr,"EDIT"); 
   else if (u->afk   ) strcpy(idlestr,"AFK ");
   else if (idle <  1) strcpy(idlestr,"ACTV");
   else if (idle <= 5) strcpy(idlestr,"AWKE");
   else if (idle >  5) strcpy(idlestr,"IDLE");
   
   return idlestr;
}

/*** Return level value based on level name ***/
int get_level(const char *name)
{
   int i;

   i=0;
   while(getLevelName(i)[0]!='*')
   {
      if (!strcmp(getLevelName(i),name)) return i;
      if (!strcmp(getLevelShortName(i),name)) return i;
      ++i;
   }
   return -1;
}

/************ INITIALISATION FUNCTIONS *************/

void user_port_init()
{
   static LimitedIntGlobalVar  mainport ( "mainport", BY_LOAD_CONFIG, -1,  -1,  65535 );
   static LimitedIntGlobalVar  wizport  ( "wizport",  BY_LOAD_CONFIG, -1,  -1,  65535 );

   addConfigVar(&mainport);
   addConfigVar(&wizport);
   
   socketInterface.addListenSocket( new ListenSocket( "U", new UserTelnetSockObjectCreator(), mainport.get() ) );
   socketInterface.addListenSocket( new ListenSocket( "W", new UserTelnetSockObjectCreator(), wizport.get() ) );
}

void moods_init()
{
   pod_string filename;
   filename += DATAFILES;
   filename += "/moods";

   if(dyn_arr_file_add(filename,&mood_list))
   {
      logStream << setLogfile( BOOTLOG ) << noTime << "BOOT FAILURE: moods_init() failed.\n" << pod_send;
      exit(134);
   }
   return;
}

/*** Initialise globals ***/
int init_globals()
{
   void (*init_call)() = 0;
   void *libnerf;
   keepalive_interval=60; /* DO NOT TOUCH!!! */
   no_prompt=0;
   user_first=NULL;
   user_last=NULL;
   room_first=NULL;
   room_last=NULL; /* This variable isn't used yet */
   time(&boot_time);
   do_events_now=0;
   temp_user_count=0;
   login_cnt=0;

   clear_words(&words);

#warning temporary fix!!! -BIG WARNING- (Fix by loading rooms later, seperate function ?, or creating later atleast) \
   otherwise only init the data_ptr later.
   revbuffs_init();
   load_and_parse_config(confile);

//   init_func_register();

   addConfigVar( &max_users );
   addConfigVar( &do_fork );
   addConfigVar( &color_on_def );
   addConfigVar( &mesg_life );
   addConfigVar( &min_private_users );
   addConfigVar( &heartbeat );
   addConfigVar( &login_idle_time );
   addConfigVar( &user_idle_time );
   addConfigVar( &max_clones );
   addConfigVar( &atmos_delay );
   addConfigVar( &newbie_idle_time );
   addConfigVar( &ping_delay );
   addConfigVar( &purgedays );
   addConfigVar( &password_echo );
   addConfigVar( &ignore_sigterm );
   addConfigVar( &ban_swearing );
   addConfigVar( &time_out_afks );
   addConfigVar( &allow_caps_in_name );
   addConfigVar( &system_logging );
   setLoggingOn( system_logging.get() != 0 );
   addConfigVar( &prompt_def );
   addConfigVar( &atmos_on );
   addConfigVar( &time_out_maxlevel );
   addConfigVar( &wizport_level );
   addConfigVar( &gatecrash_level );
   addConfigVar( &ignore_mp_level );
   addConfigVar( &minlogin_level );
   addConfigVar( globalSpecialRoomNames.getIdlersRoomName() );
   addConfigVar( globalSpecialRoomNames.getEntryRoomName() );
   addConfigVar( globalSpecialRoomNames.getMainRoomName() );
   addConfigVar( globalSpecialRoomNames.getJailRoomName() );
   addConfigVar( &crash_action );
   addConfigVar( &nuts_talk_style );
   addConfigVar( &mesg_check_time );
   addConfigVar( &default_color );

   cmd_init();
   glob_add_var_cust( &minlogin_level);
   glob_add_var_cust( &system_logging);
   glob_add_var_cust( &ban_swearing);
   glob_add_var_cust( &nuts_talk_style);
   glob_add_var_cust( &forwarding);
   glob_add_var_cust( &atmos_on);
   glob_add_var_cust( &rs_countdown);
   glob_add_var_cust( &resetAnnounce );
   glob_add_var_cust( &rs_which);
   glob_add_var_cust( &rsUserName);
   glob_add_var_cust( &num_recent_users);
   glob_add_var_cust( &logged_users);
   glob_add_var_cust( &total_logins);
   glob_add_var_cust( &firstBootTime);

   hist_init();
   init_glob_list();

   StringLibrary::getInstance()->addFile(GLB_STR_FILE ".xml" );
   
   user_var_add_cust( "desc",                            new StrObjectCreator(USER_DESC_LEN,   ""       ), USR_SAVE_ALWS );
   user_var_add_cust( "species",                         new StrObjectCreator(SPECIES_LEN,     ""       ), USR_SAVE_ALWS );
   user_var_add_cust( "enter_phrase",                    new StrObjectCreator(ENT_LVE_LEN,     ""       ), USR_SAVE_ALWS );
   user_var_add_cust( "leave_phrase",                    new StrObjectCreator(ENT_LVE_LEN,     ""       ), USR_SAVE_ALWS );
   user_var_add_cust( "in_phrase",                       new StrObjectCreator(PHRASE_LEN,      ""       ), USR_SAVE_ALWS );
   user_var_add_cust( "out_phrase",                      new StrObjectCreator(PHRASE_LEN,      ""       ), USR_SAVE_ALWS );
   user_var_add_cust( "afk_mesg",                        new StrObjectCreator(AFK_MESG_LEN,    ""       ), USR_SAVE_SOFT );
   user_var_add_cust( "mood",                            new StrObjectCreator(TOTAL_MOOD_LEN,  "?"      ), USR_SAVE_SOFT );
   user_var_add_cust( "mail_to",                         new StrObjectCreator(WORD_LEN,        ""       ), USR_SAVE_SOFT );
   user_var_add_cust( "page_file",                       new StrObjectCreator(251,             ""       ), USR_SAVE_SOFT );
   user_var_add_cust( "Squeeze_out_of_a_tight_slit_day", new StrObjectCreator(BDAY_LEN,        ""       ), USR_SAVE_ALWS );
   user_var_add_cust( "Age",                             new StrObjectCreator(AGE_LEN,         ""       ), USR_SAVE_ALWS );
   user_var_add_cust( "Email",                           new StrObjectCreator(EMAIL_LEN,       ""       ), USR_SAVE_ALWS );
   user_var_add_cust( "Url",                             new StrObjectCreator(URL_LEN,         ""       ), USR_SAVE_ALWS );
   user_var_add_cust( "Noise",                           new StrObjectCreator(NOISE_LEN,       ""       ), USR_SAVE_ALWS );
   user_var_add_cust( "PrefRoom",                        new StrObjectCreator(ROOM_NAME_LEN,   ""       ), USR_SAVE_ALWS );
   user_var_add_cust( "Icq",                             new StrObjectCreator(ICQ_LEN,         ""       ), USR_SAVE_ALWS );
   user_var_add_cust( "Gender",                          new StrObjectCreator(GEND_DESC_LEN,   ""       ), USR_SAVE_ALWS );
   user_var_add_cust( "VrfyCode",                        new StrObjectCreator(80,              "#NONE"  ), USR_SAVE_ALWS );
   user_var_add_cust( "FwdAddy",                         new StrObjectCreator(EMAIL_LEN,       "#UNSET" ), USR_SAVE_ALWS );
   user_var_add_cust( "GendChoice",                      new StrObjectCreator(GEND_CHOICE_LEN, ""       ), USR_SAVE_ALWS );
   user_var_add_cust( "Color",                           new StrObjectCreator(COLOR_LEN,       ""       ), USR_SAVE_ALWS );
   user_var_add_cust( "ColorOn",                         new IntObjectCreator(0),                          USR_SAVE_ALWS );
   user_var_add_cust( "timezone",                        new IntObjectCreator(0),                          USR_SAVE_ALWS );
   user_var_add_cust( "Width",                           new IntObjectCreator(80),                         USR_SAVE_SOFT );
   user_var_add_cust( "Height",                          new IntObjectCreator(24),                         USR_SAVE_SOFT );

   dns_socket_init();
   user_port_init();         /* user port */
   info_port_init_structs(); /* info port */
   mail_port_init_structs(); /* email port */

   moods_init();

   StringsVector::iterator iterator;
   for(iterator=pluginStringsVector.begin();iterator!=pluginStringsVector.end();iterator++)
   {
      pod_string filename("plugins/");
      filename += *iterator;
      std::cout << "-- BOOT : Loading plugin " << filename << std::endl;
      if( ( libnerf = dlopen(filename.c_str(), RTLD_NOW | RTLD_GLOBAL ) ) ) // could also use _LAZY
      {
         const char *test;
         init_call=(void(*)())dlsym(libnerf,"plugin_init");
         test = dlerror();
         if(test)
         {
            std::cout << test << std::endl;
            abort();
         }
         (*init_call)();
      }
      else
      {
         std::cerr << dlerror() << std::endl;
         abort();
      }
   }
   ignore_init();
   account_reg_init();

   load_dyn_arrs();

   return 0;
}

void (*signal_checked(int signum, void (*handler)(int)))(int)
/*void signal_checked(int signum,__sighandler_t handler)*/
{
   typedef void (*sighandler_t)(int);

   sighandler_t ret_value;

   ret_value = signal(signum,handler);
   if(ret_value == SIG_ERR)
   {
      write_syslog("Nooo!,error in init_signals()",FALSE);
      exit(0);
   }
   return ret_value;
}

int init_signals()
{
/*   signal_checked(SIGTERM,sig_handler);
   signal_checked(SIGSEGV,sig_handler);
   signal_checked(SIGBUS ,sig_handler);*/
   signal_checked(SIGILL ,SIG_IGN);
   signal_checked(SIGTRAP,SIG_IGN);
#ifndef __CYGWIN__
   signal_checked(SIGIOT ,SIG_IGN);
#endif
   signal_checked(SIGTSTP,SIG_IGN);
   signal_checked(SIGCONT,SIG_IGN);
   signal_checked(SIGHUP ,SIG_IGN);
   signal_checked(SIGINT ,SIG_IGN);
   signal_checked(SIGQUIT,SIG_IGN);
//   signal_checked(SIGABRT,SIG_IGN);
   signal_checked(SIGFPE ,SIG_IGN);
   signal_checked(SIGPIPE,SIG_IGN);
   signal_checked(SIGTTIN,SIG_IGN);
   signal_checked(SIGTTOU,SIG_IGN);
   return 0;
}

int close_checked(int fd)
{
   int val;

   if((val=close(fd))==-1)
   {
      write_syslogf("Error in close() \"%s\", while closing fd # : %i.\n",TRUE,strerror(errno),fd);
      return -1;
   }
   return val;
}
/************* WRITE FUNCTIONS ************/

/*** See if a user has access to a room. If room is fixed to private then
	it is considered a wizroom so grant permission to any user of WIZ and
	above for those. ***/
int has_room_access(UR_OBJECT user, RM_OBJECT rm)
{
if ((rm->access & ROOM_PRIVATE)
    && user->level<gatecrash_level.get()
    && user->invite_room!=rm
    && !((rm->access & ROOM_FIXED) && user->level >= LEV_THR)) return 0;
return 1;
}

#define TEMP_LEN 81

int look_users(UR_OBJECT user)
{
   UR_OBJECT u;
   RM_OBJECT rm;
   int users=0;
   char *ptr;

   rm=user->room;

   if(rm->inked) write_user(user,"~FTYou see : ~FM~OL~LII~RS ~FM~OL~LIN~RS ~FM~OL~LIK~RS ~FM~OL~LIE~RS ~FM~OL~LID~RS\n");
   else
   {
      for(u=user_first;u!=NULL;u=u->next) 
      {                
         if ( u->room!=rm ||
              u==user || 
              (!user_onlist_visible(u) && (u->level >= user->level)) )
         continue;

         StrGlobalVar *desc     = (StrGlobalVar*)u->getVar("desc");

         if (!users++) write_user(user,"~FTYou see :\n");
         if (u->afk) ptr="~OL~FR(AFK)\n"; 
         else ptr="\n";

         write_userf(user,"     ~FR%c~RS%s%s~RS %s %s~RS",get_visible_prechar(u),get_visible_color(u),u->name,desc->get().c_str(), ptr );
      }
      if (!users) write_user(user,"~FTYou see :  ~RSAbsolutely nobody\n");
      else write_user(user,"\n");
   }
   
   return 0;
}

/*** Display details of room ***/
int look(UR_OBJECT user)
{
   RM_OBJECT rm;
   char temp[TEMP_LEN];
   int i,exits;
   pod_string output;

   rm=user->room;
   write_seperator_line(user,NULL);
   write_user(user, user->room->desc.c_str() );
   write_user(user,"\n");

   look_users(user);

   exits=0;
   output = "~FTExits   :";
   for(i=0;i<MAX_LINKS;++i)
   {
      if (!rm->link[i]) break;
      if (!rm->link[i]->secret || user->level == LEV_FIV)
      {
         if (rm->link[i]->access & ROOM_PRIVATE) snprintf(temp,TEMP_LEN,"  ~FR%s",rm->link[i]->name);
         else
         {
            if (rm->link[i]->secret && user->level == LEV_FIV) snprintf(temp,TEMP_LEN,"  ~FM%s",rm->link[i]->name);
            else snprintf(temp,TEMP_LEN,"  ~FY%s",rm->link[i]->name);
         }
      }
      if (!rm->link[i]->secret || user->level == LEV_FIV)
      {
         output += temp;
      }
      ++exits;
   }
   if (!exits)
   {
      output = "~FTExits   :  ~FGThere are no exits.";
   }
   output += "\n";
   write_user(user,output.c_str());

   if (rm->access & ROOM_PRIVATE) write_userf(user,"~FTRoom    :  ~FR%-20s",rm->name);
   else write_userf(user,"~FTRoom    :  ~FG%-20s\n",rm->name);

   if (rm->topic[0])  write_userf(user,"~FTTopic   :  ~FY%s\n",rm->topic);
   else write_user(user,"~FTTopic   :  ~FGNo topic has been set yet.\n");

   output = "~FTAccess  :";
   switch(rm->access)
   {
      case ROOM_PUBLIC:        output += "  ~FGPublic~RS       ";       break;
      case ROOM_PRIVATE:       output += "  ~FRPrivate~RS      ";       break;
      case ROOM_FIXED_PUBLIC:  output += "  ~FRFixed~RS:~FGPublic~RS "; break;
      case ROOM_FIXED_PRIVATE: output += "  ~FRFixed~RS:~FRPrivate~RS"; break;
   }

   snprintf(temp,TEMP_LEN,"       ~FTMessages:  ~FG%d\n",rm->mesg_cnt);
   output += temp;
   write_user(user,output.c_str());

   write_seperator_line(user,NULL);
   return 0;
} 

/*** Set room access back to public if not enough users in room ***/
int reset_access(RM_OBJECT rm)
{
   UR_OBJECT u;
   int cnt=0;

   if (rm==NULL || rm->access!=ROOM_PRIVATE) return 0; 

   for(u=user_first;u!=NULL;u=u->next) if (u->room==rm) ++cnt;

   if (cnt<min_private_users.get()) 
   {
      write_room(rm,"Room access returned to ~FGPUBLIC.\n");
      rm->access=ROOM_PUBLIC;

      /* Reset any invites into the room & clear review buffer */
      for(u=user_first;u!=NULL;u=u->next) 
      {
         if (u->invite_room==rm) u->invite_room=NULL;
      }
      clear_revbuff(rm);
   }
   return 0;
}

int try_session_swap(UR_OBJECT user)
{
   UR_OBJECT u;

   for(u=user_first;u!=NULL;u=u->next)
   {
      if (user!=u && !is_clone(user) && !strcmp(user->name,u->name))
      {
         write_user(user,"\n\nYou are already connected - switching to old session.\n");
         write_syslogf("%s swapped sessions.\n",TRUE,user->name);

         u->socket->flag_as_closed();
         u->socket=user->socket;
         
         UserTelnetHandler* userTelnetHandler = (UserTelnetHandler*)( u->socket->getTelnetHandler() );
         userTelnetHandler->setUserPtr(u);
         
         destruct_user(user);

         StrGlobalVar *desc     = (StrGlobalVar*)u->getVar("desc");

         write_room_exceptf(u->room,u,"~OLSESSION SWAP:~RS %s %s\n",u->name, desc->get().c_str() );
         look(u);
         prompt(u);

         reset_clone_socks(user,u);
         return 0;
      }
   }
   return 1;
}

/*
   V could be after session swap, since current user object gets destroyed, also after anything not checking level
   - levelfilter, changes users level if he's found in the levelfilter file
   V very preferably up front, since a lot isn't needed for an already logged on user.
   - session swap, assigns the socket to the old user object, deletes new user object
      - tell rest of room
      - look
      - prompt
      - calls reset_clone_socks , which assigns the new sockets(and user object??) to existing clones
      - exit
   V only before anything using users's macros's (nothing thus)
   - load user macro's
   V group with rest of display functions ? then can be anywhere
   - send some initial screens
      - crt
      - motd-2
      - welcome
   V anywhere before room info is used, and after level info is clear
   - move to jail/initial/preferred room
   V does this use room info ? think so, after setting room then.
   - write some stuff to rest of talker (enter phrase, admin login info, etc.)
   V before this info is used(and after the old setting is used ! might want to use a seperat var for that)
   - set users last login time
   V group with other display funcs ?
   - level, look, accreq info, (autormail, maildeamon), newsdeamon, prompt
   V anywhere really.
   - write entry to syslog (think we might want to keep this solid too)
   V anywhere too (this is a nice thing for plugins, keep in middle).
   - do recent users logging
   V anywhere, could group with setting login to 0.
   - increase login counts
   V anywhere. (stays in this function, so solid at the end ?)
   - set users 'login' to 0 (not on login anymore)

   dude.. we might need to set need or messes with flags
   (NEEDS_USERLEVEL == 1>>0, CHANGES_USERLEVEL, NEEDS_USERROOM, CHANGES_USERROOM etc. , sort regarding that )
   also, need to clump together things that need to be in order (aka, text to user/room, first motd, then..)
   could make that user configurable, even via a menu in the talker itself ? (prints list of all plugins
   which registered for the list, let the admin pick a nice order, cheapman: use config file)
   note.., until login=0 the user doesn't see anything sent to all rooms..
*/
#warning finish this list, then make a generic object witha function call (dothis(user with this user)) and let plugins plug in theri stuff into a list of this type

int connect_user(UR_OBJECT user)
{
   int cnt;

   char temp[30];
   char filename[80];

   IntGlobalVar *accreq;

// set user level if name is in filter file //
   filter_level(user);

// swap to old user, exists right away if there is one //
   if(!try_session_swap(user))
   {
      return 0;
   }

// load macro's //
   sprintf(filename,"%s/%s.A",USERFILES,user->name);
   user->macrolist.loadMacros(filename);

// print motd to user //
   write_user(user,"\n");
   more(user,NULL,MOTD2);

// print welcome and last login time/site to user, needs these values (time changed on login) //
   if (user->last_site[0])
   {
      sprintf(temp,"%s",ctime(&user->last_login));
      temp[strlen(temp)-1]=0;
      write_userf(user,"Welcome %s...\n\n~OL~FBYou were last logged in on %s from %s.\n\n",user->name,temp,user->last_site);
   }
   else write_userf(user,"Welcome %s...\n\n",user->name);

// move to initial room - depends on level, changes room //
   accreq   = (IntGlobalVar*)user->getVar("accreq");

   if (user->level == LEV_MIN)                           user->room=get_room(globalSpecialRoomNames.getJailRoomName()->get().c_str());
   else if (accreq->get() != ACCREQ_GIVEN) user->room=get_room(globalSpecialRoomNames.getEntryRoomName()->get().c_str());
   else
   {
      StrGlobalVar *prefroom      = (StrGlobalVar*)user->getVar("PrefRoom");

      user->room=get_room( prefroom->get().c_str() );
      if (!user->room || !has_room_access(user,user->room))  user->room=get_room(globalSpecialRoomNames.getMainRoomName()->get().c_str());
   }

   StrGlobalVar *enter_phrase  = (StrGlobalVar*)user->getVar("enter_phrase");
   StrGlobalVar *desc     = (StrGlobalVar*)user->getVar("desc");

// print enter phrase to user's room (needs room)//
   if ( enter_phrase->get().length() > 0 ) write_roomf(user->room,"~OL~FY%s\n",enter_phrase->get().c_str() );

// write's login info for the admin's, and generic to all rooms, needs users level and room //

   if(user->tag) write_levelf(LEV_THR,1,NULL,"~LI?~RS ");
   write_roomf(NULL,"~OL[ SURFACING ]~RS %s %s ", user->name, desc->get().c_str() );
   if (user->level == LEV_MIN)  write_roomf(NULL,"~OL~FR<%s>~RS ",globalSpecialRoomNames.getJailRoomName()->get().c_str());
   else write_roomf(NULL,"~OL~FY<%s>~RS ",user->room->name);
   write_levelf(LEV_FOU,1,NULL,"~FT(%s:%d)",user->socket->getPeerSite().c_str(),user->socket->getPeerPort());
   write_room(NULL, "\n");

// reset last login time //
   user->last_login=time(0);

// show level info to user, needs user level //
   write_userf(user,"~FTYour level is:~RS~OL %s\n",getLevelName(user->level));

// show roominfo to user, might need level, definatly needs roominfo //
   look(user);

// show accreq related..
   if (accreq->get() == ACCREQ_NONE)
   {
      write_userf(user,"\n~OL~FWWelcome to ~FG%s~FW!!\n",TALKERNAME);
      write_user (user,"To request recidency and be auto-promoted, please use the '.accreq' command\n");
   }

// show maildeamon to user //
   if ((user->autoread==1)&&(has_unread_mail(user)))
   rmail(user);
   else if (has_unread_mail(user))
   write_user(user,"\07~OL~FR-- MAILDAEMON : ~LIYou have new mail!\n");
   else if (mail_count(user))
   {
      write_userf(user,"~OL~FT-- MAILDAEMON : You have %d old mail",mail_count(user));
      if (mail_count(user)==1) write_user(user,"~OL~FT message!\n");
      else write_user(user,"~OL~FT messages!\n");
   }

// show newsdeamon to user, needs level //
   if(user->level >= LEV_THR)
   {
      write_userf(user,"\07~OL~FT-- NEWSDAEMON : There are %d admnotes messages\n",news_count(ADMINBOARD));
   }

// shwo prompt to user //
   prompt(user);

// add a syslog entry //
   write_syslogf("%s logged in on port %d from %s:%d.\n",TRUE,user->name,user->socket->getLocalPort(),user->socket->getPeerSite().c_str(),user->socket->getPeerPort());

// log user login to .recent //
#warning certain candidate to become a plugin, but.. all the other things dont seem very wanting to become plugins, and a list with just one entry ?
   if (num_recent_users.get() > LOG_LINES)
   {
      abort(); // oops.
   }
   pod_stringstream outputStream;
   outputStream << "~FT" << std::setw(12) << user->name << "~RS logged in at: ~FG" << long_date(3);
   if (num_recent_users.get() == LOG_LINES)
   {
      for ( cnt = 0; cnt < (num_recent_users.get()-1); cnt++ )
      {
         logged_users.set( cnt, logged_users.get(cnt+1) );
      }
      logged_users.set( LOG_LINES-1, outputStream.str() );
   }
   else
   {
      logged_users.set( num_recent_users.get(), outputStream.str() );
      ++num_recent_users;
   }

// update counters //
   ++login_cnt;
   ++total_logins;

// set as not on login anymore, nontrivial! as long as this aint done write_room(NULL) will skip this user)
   user->login=0;

   return 0;
}

typedef std::vector<disconnect_handler_func*, pod_alloc< disconnect_handler_func* >::Type > DisconnectHandlersVector;
DisconnectHandlersVector disconnectHandlersVector;

void disconnect_handler_add( disconnect_handler_func *func )
{
   disconnectHandlersVector.push_back( func );
}

/*** Disconnect user from talker ***/
int disconnect_user(UR_OBJECT user)
{
   RM_OBJECT rm;
   char filename[80];

   rm=user->room;
   if (user->login)
   {
      user->socket->flag_as_closed();
      destruct_user(user);
      return 0;
   }

   // update last login etc.      
   user->last_login_len = time(0) - user->last_login;
   user->last_login     = time(0);
   strcpy( user->last_site, user->socket->getPeerSite().c_str() );
      
   // save
   save_user(user);
   
   sprintf(filename,"%s/%s.A",USERFILES,user->name);
   user->macrolist.saveMacros(filename);

   DisconnectHandlersVector::iterator disconnectHandler;
   for( disconnectHandler =  disconnectHandlersVector.begin();
        disconnectHandler != disconnectHandlersVector.end();
        disconnectHandler++ )
   {
     (**disconnectHandler)(user);
   }

   follow_kill(user,FALSE);
   write_syslogf("%s logged out.\n",TRUE,user->name);
   write_user(user,"\n~OL~FBYou are removed from this reality...\n\n");

   StrGlobalVar *leave_phrase = (StrGlobalVar*)user->getVar("leave_phrase");
   StrGlobalVar *desc         = (StrGlobalVar*)user->getVar("desc");

   if (user->cloaked || user->room->secret)
   {
/*    if (user->leave_phrase != NULL)
      {
         sprintf(text,"~OL~FY%s\n",user->leave_phrase);*/  /* added by crandonkphin, shows login intro */
/*       write_level(get_level(level_name[user->level]),1,text,NULL);*/ /* added by crandonkphin,shows intro */
/*    }  removed, no to room but only of level.. function*/

      write_levelf(user->level,1,user,"~OL[ SUBMERGING ]~RS %s %s\n",user->name,desc->get().c_str() );
   }
   else
   {
      /* added by crandonkphin, shows logout intro */
      if (leave_phrase->get().length() > 0) write_room_exceptf(user->room,user,"~OL~FY%s\n",leave_phrase->get().c_str() );
      write_room_exceptf(NULL,user,"~OL[ SUBMERGING ]~RS %s %s\n",user->name, desc->get().c_str() );
   }
   if (user->malloc_start!=NULL) xfree(user->malloc_start);
   destroy_user_clones(user,FALSE);

   user->socket->flag_as_closed();

   destruct_user(user);
   reset_access(rm);
   return 0;
}

/*** Delete a user ***/
int delete_user(UR_OBJECT user, int this_user)
{
   UR_OBJECT u;
   char name[USER_NAME_LEN+1];

   if (this_user)
   {
   /* User structure gets destructed in disconnect_user(), need to keep a
      copy of the name */
      strcpy(name,user->name);
      write_user(user,"\n~FR~LI~OLACCOUNT DELETED!\n");
      write_room_exceptf(user->room,user,"~OL~LI%s commits suicide!\n",user->name);
      write_syslogf("%s SUICIDED.\n",TRUE,name);
      disconnect_user(user);
      purge_user_files(name);
      return 0;
   }
   if (words.word_count<2)
   {
      write_user(user,"Usage: nuke <user>\n");
      return 0;
   }
   words.word[1][0]=toupper(words.word[1][0]);
   if (!strcmp(words.word[1],user->name))
   {
      write_user(user,"Heck, be glad I didn't just let you do that (use .suicide if you  -really- have to).\n");
      return 0;
   }
   if (get_user_exactmatch(words.word[1])!=NULL)
   {
      /* Safety measure just in case. Will have to .kill them first */
      write_user(user,"You cannot delete a user who is currently logged on.\n");
      return 0;
   }
   if( !(u=temp_user_spawn(user,words.word[1],"delete_user")) ) return 0;
   if (u->level>=user->level)
   {
      write_user(user,"You cannot delete a user of an equal or higher level than yourself.\n");
   }
   else
   {
      purge_user_files(words.word[1]);
      write_userf(user,"\07~FR~OL~LIUser %s deleted!\n",words.word[1]);
      write_syslogf("%s DELETED %s.\n",TRUE,user->name,words.word[1]);
   }
   temp_user_destroy(u);
   return 0;
}

/*** Reset some values at the end of editing ***/
int editor_done(UR_OBJECT user)
{
user->misc_op=MISC_NONE;
user->edit_op=0;
user->edit_line=0;
xfree(user->malloc_start);
user->malloc_start=NULL;
user->malloc_end=NULL;
user->ignall=user->ignall_store;
prompt(user);
return 0;
}

/*** Enter user profile ***/
int enter_profile(UR_OBJECT user, int done_editing)
{
   StringLibrary * stringLibrary = StringLibrary::getInstance();
FILE *fp;
char *c,filename[80];

if (!done_editing) {
	write_userf(user,"\n~BB*** Writing profile ***\n\n");
	user->misc_op=MISC_PROFILE;
	editor(user,NULL);
	return 0;
	}
sprintf(filename,"%s/%s.P",USERFILES,user->name);
if (!(fp=fopen(filename,"w"))) {
	write_userf(user,"%s: couldn't save your profile.\n",stringLibrary->makeString("syserror").c_str());
	write_syslogf("ERROR: Couldn't open file %s to write in enter_profile().\n",FALSE,filename);
	return 0;
	}
c=user->malloc_start;
while(c!=user->malloc_end) putc(*c++,fp);
fclose(fp);
write_user(user,"Profile stored.\n");
return 0;
}


/*** The editor used for writing profiles, mail and messages on the boards ***/
int editor(UR_OBJECT user, const char *inpstr)
{
   StringLibrary * stringLibrary = StringLibrary::getInstance();
   int cnt,line;
   char *edprompt="\n~FGSave~RS, ~FYredo~RS or ~FRabort~RS (s/r/a): ";
   char *ptr;

   user->inedit=1;
   if (user->edit_op)
   {
      switch(toupper(*inpstr))
      {
         case 'S':
            write_room_exceptf(user->room,user,"%s finishes composing some text.\n",get_visible_name(user).c_str());
            switch(user->misc_op)
            {
               case MISC_BOARD:   write_board(user,NULL,1);  break;
               case MISC_MAIL:    smail(user,NULL,1);  break;
               case MISC_PROFILE: enter_profile(user,1);  break;
            }
            editor_done(user);
            user->inedit=0;
            return 0;

         case 'R':
            user->edit_op=0;
            user->edit_line=1;
            user->charcnt=0;
            user->malloc_end=user->malloc_start;
            *user->malloc_start='\0';
            write_userf(user,"\nRedo message...\n\n%d>",user->edit_line);
            telnet_eor_send(user);
            return 0;

         case 'A':
            write_user(user,"\nMessage aborted.\n");
            write_room_exceptf(user->room,user,"%s gives up composing some text.\n",get_visible_name(user).c_str());
            editor_done(user);
            user->inedit=0;
            return 0;

         default:
            write_user(user,edprompt);
            telnet_eor_send(user);
            return 0;
      }
   }
/* Allocate memory if user has just started editing */
   if (user->malloc_start==NULL)
   {
      if ((user->malloc_start=(char *)xalloc(MAX_LINES*81,"editor"))==NULL)
      {
         write_userf(user,"%s: failed to allocate buffer memory.\n",stringLibrary->makeString("syserror").c_str());
         write_syslog("ERROR: Failed to allocate memory in editor().\n",FALSE);
         user->misc_op=MISC_NONE;
         prompt(user);
         return 0;
      }
      user->ignall_store=user->ignall;
      user->ignall=1; /* Dont want chat mucking up the edit screen */
      user->edit_line=1;
      user->charcnt=0;
      user->malloc_end=user->malloc_start;
      *user->malloc_start='\0';
      write_userf(user,"~FTMaximum of %d lines, end with a '.' on a line by itself.\n\n1>",MAX_LINES);
      telnet_eor_send(user);
      write_room_exceptf(user->room,user,"%s starts composing some text...\n",get_visible_name(user).c_str());
      return 0;
   }

/* Check for empty line */
   if (!words.word_count)
   {
      if (!user->charcnt)
      {
         write_userf(user,"%d>",user->edit_line);
         telnet_eor_send(user);
         return 0;
      }
      *user->malloc_end++='\n';
      if (user->edit_line==MAX_LINES)
      {
         goto END;
      }
      write_userf(user,"%d>",++user->edit_line);
      telnet_eor_send(user);
      user->charcnt=0;
      return 0;
   }

/* If nothing carried over and a dot is entered then end */
   if (!user->charcnt && !strcmp(inpstr,".")) goto END;

   line=user->edit_line;
   cnt=user->charcnt;

/* loop through input and store in allocated memory */
   while(*inpstr)
   {
      *user->malloc_end++=*inpstr++;
      if (++cnt==80) {  user->edit_line++;  cnt=0;  }
      if (user->edit_line>MAX_LINES || user->malloc_end - user->malloc_start>=MAX_LINES*81)
      {
         goto END;
      }
   }

   if (line!=user->edit_line)
   {
      ptr=(char *)(user->malloc_end-cnt);
      *user->malloc_end='\0';
      write_userf(user,"%d>%s",user->edit_line,ptr);
      telnet_eor_send(user);
      user->charcnt=cnt;
      return 0;
   }
   else
   {
      *user->malloc_end++='\n';
      user->charcnt=0;
   }

   if (user->edit_line!=MAX_LINES)
   {
      write_userf(user,"%d>",++user->edit_line);
      telnet_eor_send(user);
      return 0;
   }

/* User has finished his message , prompt for what to do now */
END:
   *user->malloc_end='\0';
   if (*user->malloc_start)
   {
      write_user(user,edprompt);
      telnet_eor_send(user);
      user->edit_op=1;
      return 0;
   }
   write_user(user,"\nNo text.\n");
   write_room_exceptf(user->room,user,"%s gives up composing some text.\n",get_visible_name(user).c_str());
   editor_done(user);
   user->inedit=0;
   return 0;
}

/*** Stuff that is neither speech nor a command is dealt with here ***/
int misc_ops(UR_OBJECT user, const char *inpstr)
{
   char filename[80];

   if(user->nextCommand)
   {
      user->nextCommand->exec(user,(char*)inpstr);
      return 1;
   }

   switch(user->misc_op)
   {
      case MISC_MORE: /* check at end of more part ('E to end, enter to continue') */
         misc_more(user,inpstr);
         return 1;

      case MISC_BOARD:   /* writing on board */
      case MISC_MAIL:    /* Writing mail */
      case MISC_PROFILE: /* doing profile */
         editor(user,inpstr);
         return 1;

      case MISC_SUICIDE: /* Suicide! */
         if (toupper(inpstr[0])=='Y')
         {
            delete_user(user,1);
         }
         else
         {
            user->misc_op=MISC_NONE;
            prompt(user);
         }
         return 1;

      case MISC_DEL_MAIL:
         if (toupper(inpstr[0])=='Y')
         {
            user->misc_op=MISC_NONE;
            write_user(user,"All mail deleted.\n");
            sprintf(filename,"%s/%s.M.xml",USERFILES,user->name);
            unlink_checked(filename,"misc_ops");
            prompt(user);
         }
         else
         {
            user->misc_op=MISC_NONE;
            prompt(user);
         }
         return 1;

      case MISC_WARM_BOOT:
         user->misc_op=MISC_NONE;
         if (toupper(inpstr[0])=='Y')
         {
            write_systemf("\07~OLSYSTEM: ~FR~LIWarm boot in progress.\n");
            softboot_shutdown(); /* function won't return :) */
            write_user(user,"warning, softboot failed!\n");
         }
         user->misc_op=MISC_NONE;
         prompt(user);
         return 1;
   }
   return 0;
}

/*** User prompt ***/
int prompt(UR_OBJECT user)
{
   struct tm *tm_struct;
   int hr,min;

   if (no_prompt) return 0;

   if (user->command_mode && !user->misc_op)
   {
     write_user(user,"~FTCOM> ");
     return 0;
   }

   if (!user->prompt || user->misc_op) return 0;

   tm_struct = get_current_localtime();

   hr=(int)(time(0)-user->last_login)/3600;
   min=((int)(time(0)-user->last_login)%3600)/60;

   if (user->cloaked)   write_userf(user,"~FR<%02d:%02d, %02d:%02d, %s>\n",tm_struct->tm_hour,tm_struct->tm_min,hr,min,user->name);
   else if (!user->vis) write_userf(user,"~FB<%02d:%02d, %02d:%02d, %s>\n",tm_struct->tm_hour,tm_struct->tm_min,hr,min,user->name);
   else                 write_userf(user,"~FT<%02d:%02d, %02d:%02d, %s>\n",tm_struct->tm_hour,tm_struct->tm_min,hr,min,user->name);
   return 0;
}

/*** Get room struct pointer from abbreviated name ***/
RM_OBJECT get_room(const char *name)
{
   RoomsVector::iterator roomNode;

   for(roomNode=roomsList.begin();roomNode != roomsList.end();roomNode++)
   {
      if (!strncmp((*roomNode)->name,name,strlen(name)))
      {
         return (*roomNode);
      }
   }
   return NULL;
}

/*** Switch between command and speech mode ***/
int toggle_mode(UR_OBJECT user)
{
   user->command_mode=!user->command_mode;
   write_userf(user,"Now in %s mode.\n",user->command_mode ? "COMMAND" : "SPEECH");
   return 0;
}

int afk_check_verbal(UR_OBJECT checker,UR_OBJECT target)
{
   StrGlobalVar *afk_mesg    = (StrGlobalVar*)target->getVar("afk_mesg");

   if (target->afk)
   {
      if ( afk_mesg->get().length() > 0 )
      {
         write_userf(checker,"%s is AFK, message is: %s\n",target->name, afk_mesg->get().c_str() );
      }
      else write_userf(checker,"%s is AFK at the moment.\n",target->name);
      return 1;
   }
   return 0;
}

void misc_cloak(UR_OBJECT user, char *inpstr)
{
   StrGlobalVar *leave_phrase = (StrGlobalVar*)user->getVar("leave_phrase");
   StrGlobalVar *desc         = (StrGlobalVar*)user->getVar("desc");

   if (toupper(inpstr[0])=='Y')
   {
      if (leave_phrase->get().length() > 0) write_roomf(user->room,"~OL~FY%s\n", leave_phrase->get().c_str() );
      write_roomf(NULL,"~OL[ SUBMERGING ]~RS %s %s\n",user->name,desc->get().c_str() );
   }
   if (!user->vis)
   {
      user->vis=1;
      write_user(user,"You are no longer invisible.\n");
   }

   user->cloaked=1;
      
   // store the 'cloaked' values.
   user->cloak_store_last_login     = time(0);
   user->cloak_store_last_login_len = time(0) - user->last_login;
   strcpy( user->cloak_store_last_site, user->socket->getPeerSite().c_str() );
   
   write_user(user,"You are now cloaked.\n");
}

void misc_decloak(UR_OBJECT user, const char *inpstr)
{
   StrGlobalVar *enter_phrase  = (StrGlobalVar*)user->getVar("enter_phrase");
   StrGlobalVar *desc          = (StrGlobalVar*)user->getVar("desc");
   
   if (toupper(inpstr[0])=='Y')
   {
      if ( enter_phrase->get().length() > 0 ) write_roomf(user->room, "~OL~FY%s\n",enter_phrase->get().c_str() );
      write_roomf(NULL, "~OL[ SURFACING ]~RS %s %s ",user->name,desc->get().c_str() );
      if (user->level == LEV_MIN) write_roomf(NULL,"~OL~FR<%s>~RS ",globalSpecialRoomNames.getJailRoomName()->get().c_str());
      else write_roomf(NULL,"~OL~FY<%s>~RS ",user->room->name);
      pod_stringstream outputStream;
      outputStream << "~FT(" << user->socket->getPeerSite() << ":" << user->socket->getPeerPort() << ")";
      write_level(LEV_FOU,1,(char*)outputStream.str().c_str(),NULL);
      write_room(NULL, "\n");
   }
   user->cloaked=0;

   // set cloak storage to something neutral.
   user->cloak_store_last_login     = 0;
   user->cloak_store_last_login_len = 0;
   strcpy( user->cloak_store_last_site, "" );

   write_user(user,"You are no longer cloaked.\n");
}

/* Cloak command, better than invis. User is invisible and user is not
   counted in who. Written by Vaghn 1998 */
int cloak(UR_OBJECT user, char *inpstr)
{
   if(user->nextCommand)
   {
      if (user->cloaked)
      {
         misc_decloak(user, inpstr);
      }
      else
      {
         misc_cloak(user, inpstr);
      }
      user->nextCommand=0;
      prompt(user);
   }
   else
   {
      if (user->cloaked) write_user(user,"Do you want to fake a login when you decloak? (y/n) ");
      else               write_user(user,"Do you want to fake a logout when you cloak? (y/n) ");
      telnet_eor_send(user);
      user->nextCommand=currentCommand;
      no_prompt=1;
   }
   return 0;
}

/** Broadcast an important message **/
int bcast(UR_OBJECT user, const char *inpstr)
{
   if (words.word_count<2)
   {
      write_user(user,"Usage: bcast <message>\n");
   }
   else
   {
      force_listen=1;
      write_systemf("\07\n~BR*** Broadcast message from %s ***\n%s\n\n",get_visible_name(user).c_str(),inpstr);
   }
   return 0;
}

/*** Switch prompt on and off ***/
int toggle_prompt(UR_OBJECT user)
{
   user->prompt = !user->prompt;
   write_userf(user,"Prompt %s.\n",user->prompt ? "~FGON" :"~FROFF");

   return 0;
}


/*** Set rooms to public or private ***/
int set_room_access(UR_OBJECT user)
{
   StringLibrary * stringLibrary = StringLibrary::getInstance();
   UR_OBJECT u;
   RM_OBJECT rm;
   pod_string name;
   int cnt;

   rm=user->room;
   if (words.word_count<2) rm=user->room;
   else
   {
      if (user->level<gatecrash_level.get())
      {
         write_user(user,"You are not a high enough level to use the room option.\n");
         return 0;
      }
      else if ((rm=get_room(words.word[1]))==NULL)
      {
         write_user_crt(user,stringLibrary->makeString("nosuchroom").c_str());
         return 0;
      }
   }
   if (rm->access>ROOM_PRIVATE)
   {
      if (rm==user->room) write_user(user,"This room's access is fixed.\n");
      else write_user(user,"That room's access is fixed.\n");
   }
   else if ( com_status == PUBCOM && rm->access==ROOM_PUBLIC)
   {
      if (rm==user->room) write_user(user,"This room is already public.\n");
      else                write_user(user,"That room is already public.\n");
   }
   else if ( com_status == PRIVCOM && rm->access==ROOM_PRIVATE)
   {
      if (rm==user->room) write_user(user,"This room is already private.\n");
      else                write_user(user,"That room is already private.\n");
   }
   else
   {
      name=get_visible_name(user);
      if ( com_status == PRIVCOM)
      {
         cnt=0;
         for(u=user_first;u!=NULL;u=u->next) if (u->room==rm) ++cnt;
         if (cnt<min_private_users.get() && user->level<ignore_mp_level.get())
         {
            write_userf(user,"You need at least %d users/clones in a room before it can be made private.\n",min_private_users.get());
         }
         else
         {
            write_user(user,"Room set to ~FRPRIVATE.\n");
            if (rm==user->room) write_room_exceptf(rm,user,"%s has set the room to ~FRPRIVATE.\n",name.c_str());
            else write_room(rm,"This room has been set to ~FRPRIVATE.\n");
            rm->access=ROOM_PRIVATE;
         }
      }
      else
      {
         write_user(user,"Room set to ~FGPUBLIC.\n");
         if (rm==user->room) write_room_exceptf(rm,user,"%s has set the room to ~FGPUBLIC.\n",name.c_str());
         else write_room(rm,"This room has been set to ~FGPUBLIC.\n");
         rm->access=ROOM_PUBLIC;

         for(u=user_first;u!=NULL;u=u->next)
         {
            if (u->invite_room==rm) u->invite_room=NULL;
         }
         clear_revbuff(rm);
      }
   }
   return 0;
}


/*** Set the room topic ***/
int set_topic(UR_OBJECT user, const char *inpstr)
{
   RM_OBJECT rm;
   pod_string name;

   rm=user->room;
   if (words.word_count<2)
   {
      if (!strlen(rm->topic)) write_user(user,"No topic has been set yet.\n");
      else                    write_userf(user,"The current topic is: %s\n",rm->topic);
   }
   else if (strlen(inpstr)>TOPIC_LEN) write_user(user,"Topic too long.\n");
   else if ((!strcmp(words.word[1],"-c"))&&(user->level>LEV_FOU))
   {
      rm->topic[0]='\0';
      write_userf(user,"You have cleared the topic for room: %s\n",rm->name);
   }
   else
   {
      write_userf(user,"Topic set to: %s\n",inpstr);
      name=get_visible_name(user);
      write_room_exceptf(rm,user,"%s has set the topic to: %s\n",name.c_str(),inpstr);
      strcpy(rm->topic,inpstr);
   }

   return 0;
}


/*************************************************************
*** delete lines from boards or mail or suggestions, etc
*************************************************************/
int* get_wipe_parameters(UR_OBJECT user, int max)
{
char* nextnum;
char* first_invalid = (char*)(10); /*not NULL , can be anything else)*/
int* parameter_list;
int from,to,cnt2,cnt1,temp;

   if(!(parameter_list = (int *)xalloc(max* sizeof(int),"wipe_param" ) ))
   {
      write_syslog("ERROR: Failed to allocate memory in get_wipe_parameters().\n",0);
      xfree(parameter_list);
      return NULL;
   }
   for(cnt1=0;cnt1<max;cnt1++)
   {
      parameter_list[cnt1]=0;
   }

   strtolower(words.word[1]);
   if (!strcmp(words.word[1],"all"))
   {
      for(cnt1=0;cnt1<max;cnt1++)
      {
      	parameter_list[cnt1]=-1;
      }
      return(parameter_list);
   }

   for(cnt1=1;cnt1<words.word_count;cnt1++)
   {
      /* get delete paramters */
      strtolower(words.word[cnt1]);

      from=strtol(words.word[cnt1],&first_invalid,10);
      if(words.word[cnt1] == first_invalid)
      {
         write_user(user,"From value should be a number.\n");
         xfree(parameter_list);
         return NULL;
      }
      if((*first_invalid != '-') && (*first_invalid))
      {
         write_user(user,"Eek!, no strange char's after the from value :).\n");
         xfree(parameter_list);
         return NULL;
      }
      if(from>max)
      {
         write_user(user,"From value should be smaller or equal to amount of msg's\n");
         xfree(parameter_list);
         return NULL;
      }
      if(from<=0)
      {
         write_user(user,"From value should be larger then zero\n");
         xfree(parameter_list);
         return NULL;
      }
      if((nextnum=strchr(words.word[cnt1],'-')))
      {
         to=strtol(nextnum+1,&first_invalid,10);
         if((nextnum+1) == first_invalid)
         {
            write_user(user,"To value should be a number.\n");
            xfree(parameter_list);
            return NULL;
         }
         if(*first_invalid)
         {
            write_user(user,"No char's after to value allowed.\n");
            xfree(parameter_list);
            return NULL;
         }
         if(to>max)
         {
            write_user(user,"To value should be smaller or equal to amount of msg's\n");
            xfree(parameter_list);
            return NULL;
         }
         if(to<=0)
         {
            write_user(user,"To value should be larger then zero\n");
            xfree(parameter_list);
            return NULL;
         }
      }
      else
      {
         to=from;
      }

      if (from>to)
      {
         temp = to;
         to = from;
         from = temp;
      }
      for(cnt2=(from-1);cnt2<to;cnt2++)
      {
        parameter_list[cnt2]=1;
      }
   }
   return(parameter_list);
}


/*** Show talker rooms ***/
/*
         if (!strncmp(ADMINBOARD,words.word[1],strlen(words.word[1])))
         {
            if(user->level>=THR)
            {
               words.word[1]=ADMINBOARD;
               flag=1;
            }
         }
*/

int rooms(UR_OBJECT user)
{
   RoomsVector::iterator roomNode;

   UR_OBJECT u;
   char access[9];
   int cnt;

   write_userf(user,"\n~OL~FB*** Rooms data ***\n\n~FT%-*s : Access  Users  Mesgs  Inked  Topic\n\n",ROOM_NAME_LEN,"Room name");
   for(roomNode=roomsList.begin();roomNode != roomsList.end();roomNode++)
   {
      if ((*roomNode)->access & ROOM_PRIVATE) strcpy(access," ~FRPRIV");
      else strcpy(access,"  ~FGPUB");
      if ((*roomNode)->access & ROOM_FIXED) access[0]='*';
      cnt=0;
      for(u=user_first;u!=NULL;u=u->next)
      {
         if (u->cloaked && !(u->level <= user->level) ) continue;
         if (!is_clone(u) && u->room==(*roomNode)) ++cnt;
      }
      if (!(*roomNode)->secret || user->level == LEV_FIV)
      {
         write_userf(user,"%-*s : %9s~RS    %3d    %3d    %3d  %s\n",ROOM_NAME_LEN,(*roomNode)->name,access,cnt,(*roomNode)->mesg_cnt,(*roomNode)->inked,(*roomNode)->topic);
      }
   }
   write_user(user,"\n");
   return 0;
}

/*** Change users password. Only THRes and above can change another users
	password and they do this by specifying the user at the end. When this is
	done the old password given can be anything, the wiz doesnt have to know it
	in advance. ***/
int change_pass(UR_OBJECT user)
{
   UR_OBJECT u;
   pod_string name;

   if (words.word_count<3)
   {
      if (user->level<LEV_FOU) write_user(user,"Usage: passwd <old password> <new password>\n");
      else                 write_user(user,"Usage: passwd <old password> <new password> [<user>]\n");
   }
   else if (strlen(words.word[2])<3)        write_user(user,"New password too short.\n");
   else if (strlen(words.word[2])>PASS_LEN) write_user(user,"New password too long.\n");
   else if (words.word_count==3)
   {
      if (strcmp((char *)crypt(words.word[1],"NU"),user->pass)) write_user(user,"Old password incorrect.\n");
      else if (!strcmp(words.word[1],words.word[2]))                  write_user(user,"Old and new passwords are the same.\n");
      else
      {
         strcpy(user->pass,(char *)crypt(words.word[2],"NU"));
         save_user(user);
         cls(user);
         write_user(user,"Password changed.\n");
      }
   }
   else if (user->level<LEV_FOU) write_user(user,"You are not a high enough level to use the user option.\n");
   else
   {
      words.word[3][0]=toupper(words.word[3][0]);
      if (!strcmp(words.word[3],user->name)) write_user(user,"You cannot change your own password using the <user> option.\n");
      else if ((u=get_user(words.word[3])))
      {
         if (u->level>=user->level) write_user(user,"You cannot change the password of a user of equal or higher level than yourself.\n");
         else
         {
            strcpy(u->pass,(char *)crypt(words.word[2],"NU"));
            cls(user);
            write_userf(user,"%s's password has been changed.\n",u->name);
            name=get_visible_name(user);
            write_userf(u,"~FR~OLYour password has been changed by %s!\n",name.c_str());
            write_syslogf("%s changed %s's password.\n",TRUE,user->name,u->name);
         }
      }
      else if( (u=temp_user_spawn(user,words.word[3],"change_pass")) )
      {
         if (u->level>=user->level)
         {
            write_user(user,"You cannot change the password of a user of equal or higher level than yourself.\n");
            temp_user_destroy(u);
         }
         else
         {
            strcpy(u->pass,(char *)crypt(words.word[2],"NU"));
            save_user(u);
            temp_user_destroy(u);
            cls(user);
            write_userf(user,"%s's password changed to \"%s\".\n",words.word[3],words.word[2]);
            write_syslogf("%s changed %s's password.\n",TRUE,user->name,u->name);
         }
      }
   }
   return 0;
}

/*** Kill a user ***/
int kill_user(UR_OBJECT user)
{
   StringLibrary * stringLibrary = StringLibrary::getInstance();
   UR_OBJECT victim;
   pod_string name;

   if (words.word_count<2)                      write_user(user,"Usage: kill <user>\n");
   else if (!(victim=get_user(words.word[1])))  write_user_crt(user,stringLibrary->makeString("notloggedon").c_str());
   else if (user==victim)                 write_user(user,"Trying to commit suicide this way is the sixth sign of madness.\n");
   else if (victim->level>=user->level)
   {
      write_user(user,"You cannot kill a user of equal or higher level than yourself.\n");
      write_userf(victim,"%s tried to kill you!\n",user->name);
   }
   else
   {
      write_syslogf("%s KILLED %s.\n",TRUE,user->name,victim->name);
      write_user(user,"~FM~OLYou Look upon your prey with despise.\n");
      name=get_visible_name(user);
      write_roomf(NULL,"~FM~OL%s noisily clacks %s teeth...\n",name.c_str(),get_gender(user,"his"));
      write_userf(victim,"~FM~OL%s's flukes slice through the air, and slaughters you!!!\n",name.c_str());
      write_roomf(NULL,"~FM~OL%s's fluke slices through the air, thwapping %s to death!\n",name.c_str(),victim->name);
      write_roomf(NULL,"~FM~OL%s swims back to the ocean cove whistling triumph over the evil %s\n",name.c_str(),victim->name);
      quit(victim);
   }

   return 0;
}

/*** Set user visible or invisible ***/
int visibility(UR_OBJECT user, int vis)
{
   if (vis == user->vis) write_userf(user,"You are already %s.\n", vis ? "visible" : "invisible" );
   else
   {
      if (vis)
      {
         write_user(user,"~FB~OLYou recite a melodic incantation and reappear.\n");
         write_room_exceptf(user->room,user,"~FB~OLYou hear a melodic incantation chanted and %s materialises!\n",user->name);
      }
      else
      {
         if (user->cloaked)
         {
            user->cloaked=0;
            write_user(user,"You are no longer cloaked.\n");
         }
         write_user(user,"~FB~OLYou recite a melodic incantation and fade out.\n");
         write_room_exceptf(user->room,user,"~FB~OL%s recites a melodic incantation and disappears!\n",get_visible_name(user).c_str());
      }
      user->vis=vis;
   }

   return 0;
}

/*** Site a user ***/
int site(UR_OBJECT user)
{
   UR_OBJECT u;

   if (words.word_count<2) write_user(user,"Usage: site <user>\n");
   else if ((u=get_user(words.word[1]))) write_userf(user,"%s is logged in from %s:%d.\n",u->name,u->socket->getPeerSite().c_str(),u->socket->getPeerPort());
   else if( (u=temp_user_spawn(user,words.word[1],"site")) )
   {
      write_userf(user,"%s was last logged in from %s.\n",words.word[1],u->last_site);
      temp_user_destroy(u);
   }
   return 0;
}


/*** Switch system logging on and off ***/
int logging(UR_OBJECT user)
{
   if (system_logging.get())
   {
      write_user(user,"System logging ~FROFF.\n");
      write_syslogf("%s switched system logging OFF.\n",TRUE,user->name);
      system_logging.set(0);
   }
   else
   {
      write_user(user,"System logging ~FGON.\n");
      write_syslogf("%s switched system logging ON.\n",TRUE,user->name);
      system_logging.set(1);
   }

   setLoggingOn( system_logging.get() != 0 );

   return 0;
}

/*** Set minlogin level ***/
int minlogin(UR_OBJECT user)
{
   UR_OBJECT u,next;
   char *usage="Usage: minlogin NONE/<user level>\n";
   char levstr[5];
   int lev,cnt;

   if (words.word_count<2) write_user(user,usage);
   else
   {
      if ((lev=get_level(words.word[1])) != -1) strcpy(levstr,getLevelName(lev));
      else
      {
         if (strcasecmp(words.word[1],"NONE"))
         {
            write_user(user,usage);
            return 0;
         }
         lev=-1;
         strcpy(levstr,"NONE");
      }

      if (lev>user->level)          write_user(user,"You cannot set minlogin to a higher level than your own.\n");
      else if (minlogin_level.get()==lev) write_user(user,"It is already set to that.\n");
      else
      {
         minlogin_level.set(lev);
         write_userf(user,"Minlogin level set to: ~OL%s.\n",levstr);
         write_room_exceptf(NULL,user,"%s has set the minlogin level to: ~OL%s.\n",get_visible_name(user).c_str(),levstr);
         write_syslogf("%s set the minlogin level to %s.\n",TRUE,user->name,levstr);
         cnt=0;
         for(u=user_first;u;u=next)
         {
            next=u->next;
            if (!u->login && !is_clone(u) && u->level<lev)
            {
               write_user(u,"\n~FY~OLYour level is now below the minlogin level, disconnecting you...\n");
               quit(u);
               ++cnt;
            }
         }
         write_userf(user,"Total of %d users were disconnected.\n",cnt);
      }
   }
   return 0;
}

/*** Show talker system parameters etc ***/
int system_details(UR_OBJECT user)
{
   RoomsVector::iterator roomNode;
   char minlogin[5];
   int rms;

   write_userf(user,"\n~OL~FB*** %s version %s - System status ***\n",TALKER_NAME,RVERSION);

   rms=0;
   for(roomNode=roomsList.begin();roomNode != roomsList.end();roomNode++)
   {
      ++rms;
   }

   if (minlogin_level.get() == -1) strcpy(minlogin,"NONE");
   else strcpy(minlogin,getLevelName(minlogin_level.get()));

   /* Show header parameters */
   write_userf(user,"~FTProcess ID    : ~FG%d\n",getpid());
   show_compiler_info(user);
   show_uptime(user);
   show_total_uptime(user);
   write_user(user,socketInterface.getPortsString().c_str());
   write_user(user,"\n\n");

   /* Show others */
   write_userf(user,"Max users              : %-12d ",max_users.get());
   write_userf(user,"Current num. of users  : %d\n",get_num_of_users());

   write_userf(user,"Max clones             : %-12d ",max_clones.get());
   write_userf(user,"Current num. of clones : %d\n",clones_count());

   write_userf(user,"Current minlogin level : %-12s ",minlogin);
   write_userf(user,"Login idle time out    : %d secs.\n",login_idle_time.get());

   write_userf(user,"User idle time out     : %-4d secs.   ",user_idle_time.get());
   write_userf(user,"Heartbeat              : %d\n",heartbeat.get());

   write_userf(user,"Wizport min login level: %-12s ",getLevelName(wizport_level.get()));
   write_userf(user,"Gatecrash level        : %s\n",getLevelName(gatecrash_level.get()));

   write_userf(user,"Time out maxlevel      : %-12s ",getLevelName(time_out_maxlevel.get()));
   write_userf(user,"Private room min count : %d\n",min_private_users.get());

   write_userf(user,"Message lifetime       : %-3d days     ",mesg_life.get());
   write_userf(user,"Message check time     : %02d:%02d\n",mesg_check_time.getHour(),mesg_check_time.getMin());

   write_userf(user,"Number of rooms        : %-3d          ",rms);
   write_userf(user,"Newbie idle time out   : %-4d secs.\n",newbie_idle_time.get());

   write_userf(user,"Ignoring sigterm       : %-12s ",noyes2[ignore_sigterm.get()]);
   write_userf(user,"Ping heartbeat         : %d\n",ping_delay.get());

   write_userf(user,"Atmospherics           : %-12s ",offon[atmos_on.get()]);
   write_userf(user,"Atmospherics heartbeat : %d\n",atmos_delay.get());

   write_userf(user,"Echoing passwords      : %-12s ",noyes2[password_echo.get()]);
   write_userf(user,"Swearing banned        : %s\n",noyes2[ban_swearing.get()]);

   write_userf(user,"Time out afks          : %-12s ",noyes2[time_out_afks.get()]);
   write_userf(user,"Allowing caps in name  : %s\n",noyes2[allow_caps_in_name.get()]);

   write_userf(user,"New user prompt default: %-12s ",offon[prompt_def.get()]);
   write_userf(user,"New user color default : %s\n",offon[color_on_def.get()]);

   write_userf(user,"Crash action           : %-12s ",crash_action_txt());
   write_userf(user,"System logging         : %s\n",offon[system_logging.get()]);

   write_userf(user,"Temp users (err. if !0): %-6d\n",temp_user_count);

   write_user(user,"\n");
   return 0;
}

/*** Free a hung socket ***/
int clearline(UR_OBJECT user)
{
   UR_OBJECT u;
   int sock;

   if (words.word_count<2 || !isNumber(words.word[1])) write_user(user,"Usage: clearline <line>\n");
   else
   {
      sock=atoi(words.word[1]);

      for(u=user_first;u!=NULL;u=u->next)
         if (!is_clone(u) && u->socket->getFileDescriptor() == sock) break;

      if(u==NULL)        write_user(user,"That line is not currently active.\n");
      else if(!u->login) write_user(user,"You cannot clear the line of a logged in user.\n");
      else
      {
         write_user(u,"\n\nThis line is being cleared.\n\n");
         disconnect_user(u);
         write_syslogf("%s cleared line %d.\n",TRUE,user->name,sock);
         write_userf(user,"Line %d cleared.\n",sock);
      }
   }

   return 0;
}

/*** Change whether a rooms access is fixed or not ***/
int change_room_fix(UR_OBJECT user, int fix)
{
   StringLibrary * stringLibrary = StringLibrary::getInstance();
   RM_OBJECT rm;
   pod_string name;

   if (words.word_count<2) rm=user->room;
   else if (!(rm=get_room(words.word[1])))
   {
      write_user_crt(user,stringLibrary->makeString("nosuchroom").c_str());
      return 0;
   }
   name = get_visible_name(user);
   if (fix != !(rm->access & ROOM_FIXED))
   {
      if (rm==user->room) write_userf(user,"This room's access is already %s.\n",fix ? "fixed" : "unfixed" );
      else write_userf(user,"That room's access is already %s.\n",fix ? "fixed" : "unfixed" );
   }
   else if (fix)
   {
      write_userf(user,"Access for room %s is now ~FRFIXED.\n",rm->name);
      if (user->room==rm) write_room_exceptf(rm,user,"%s has ~FRFIXED~RS access for this room.\n",name.c_str());
      else                write_roomf(rm,"This room's access has been ~FRFIXED.\n");
      write_syslogf("%s FIXED access to room %s.\n",TRUE,user->name,rm->name);
      rm->access|=ROOM_FIXED;
   }
   else
   {
      write_userf(user,"Access for room %s is now ~FGUNFIXED.\n",rm->name);
      if (user->room==rm) write_room_exceptf(rm,user,"%s has ~FGUNFIXED~RS access for this room.\n",name.c_str());
      else     write_roomf(rm,"This room's access has been ~FGUNFIXED.\n");
      write_syslogf("%s UNFIXED access to room %s.\n",TRUE,user->name,rm->name);
      rm->access &= ~ROOM_FIXED;
      reset_access(rm);
   }
   return 0;
}

int afk(UR_OBJECT user, const char *inpstr)
{
   int lock=FALSE;

   if (words.word_count>1 && !strcmp(words.word[1],"lock"))
   {
      lock=TRUE;
      inpstr=const_remove_first(inpstr);
   }
   if (strlen(inpstr)>AFK_MESG_LEN)
   {
      write_user(user,"AFK message too long.\n");
      return 0;
   }
   if(lock)
   {
      write_user(user,"You are now AFK with the session locked, enter your password to unlock it.\n");
      user->afk=2;
   }
   else
   {
      write_user(user,"You are now AFK, press <return> to reset.\n");
      user->afk=1;
   }
   if (inpstr[0])
   {
      StrGlobalVar *afk_mesg    = (StrGlobalVar*)user->getVar("afk_mesg");
      afk_mesg->set( inpstr );
      write_user(user,"AFK message set.\n");
      if (!user->cloaked)   write_room_exceptf(user->room, user, "%s goes AFK: %s\n",get_visible_name(user).c_str(), afk_mesg->get().c_str() );
   }
   else if (!user->cloaked) write_room_exceptf(user->room, user, "%s goes AFK...\n", get_visible_name(user).c_str());

   return 0;
}

/*** Toggle user color_on on and off ***/
int toggle_color_on(UR_OBJECT user)
{
   IntGlobalVar *color_on    = (IntGlobalVar*)user->getVar("ColorOn");

   if ( color_on->get() )
   {
      write_user(user,"Color  ~FROFF.\n");
      color_on->set(0);
   }
   else
   {
      color_on->set(1);
      write_user(user,"Color  ~FGON.\n");
   }
   return 0;
}

int suicide(UR_OBJECT user)
{
   if (words.word_count<2)
   {
      write_user(user,"Usage: suicide <your password>\n");
      return 0;
   }
   if (strcmp((char *)crypt(words.word[1],"NU"),user->pass))
   {
      write_user(user,"Password incorrect.\n");
      return 0;
   }
   write_user(user,"\n\07~FR~OL~LI*** WARNING - This will delete your account! ***\n\nAre you sure about this (y/n)? ");
   telnet_eor_send(user);
   user->misc_op=MISC_SUICIDE;
   no_prompt=1;
   return 0;
}


int talker_softboot_command(UR_OBJECT user)
{
   write_user(user,"\n\07~FG~OL~LI*** WARNING - This will warmboot the talker! ***\n\nAre you sure about this (y/n)? ");
   telnet_eor_send(user);
   user->misc_op=MISC_WARM_BOOT;
   no_prompt=1;
   return 0;
}

/*** login_time_out is the length of time someone can idle at login,
     user_idle_time is the length of time they can idle once logged in.
     Also ups users total login time. ***/
int check_idle_and_timeout()
{
   RM_OBJECT rm;
   UR_OBJECT user,next;
   int tm,idle_time,idle_time_disconnect;

/* Use while loop here instead of for loop for when user structure gets
   destructed, we may lose ->next link and crash the program */
   user=user_first;
   while(user)
   {
      next=user->next;

      if (is_clone(user))
      {
         user=next;
         continue;
      }

      user->total_login+=heartbeat.get();

      if(user->afk)             user->total_afk_time    += heartbeat.get();
      else if(!is_idling(user)) user->total_active_time += heartbeat.get();
      else                      user->total_idle_time   += heartbeat.get();

      if (user->level>time_out_maxlevel.get())
      {
         user=next;
         continue;
      }

      tm=get_idle_secs(user);

      if (user->login)
      {
         if( tm>=login_idle_time.get())
         {
            write_user(user,"\n\n*** Time out ***\n\n");
            quit(user);
         }
         user=next;
         continue;
      }
      if( user->afk && !time_out_afks.get())
      {
         user=next;
         continue;
      }

      if(user->level > LEV_ONE) idle_time = user_idle_time.get();
      else                      idle_time = newbie_idle_time.get();

      if(user->afk)         idle_time *= 2;

      idle_time_disconnect = idle_time;

      if( user->level != LEV_ZER ) idle_time_disconnect *= 2 ;

      switch(user->idle_status)
      {
         case IDLE_NONE:
            if( user->level <= LEV_ZER )
            {
               if ( tm >= idle_time_disconnect-60 )
               {
                  if(!user->afk) write_user(user,"\n\07~FY~OL~LI*** WARNING - Input within 1 minute or you will be disconnected. ***\n\n");
                  else write_user(user,"\n\07~FY~OL~LI*** WARNING - Spending all your time AFK isn't the best way to get to know your fellow PODmates. Return from afk in 60 seconds or you will be disconnected. ***\n\n");
                  user->idle_status=IDLE_DISCONNECT_WARNED;
               }
            }
            else
            {
               if( tm >= idle_time-60 )
               {
                  if(!user->afk) write_user(user,"\n\07~FY~OL~LI*** WARNING - Input within 1 minute or you will be moved to the idle room. ***\n\n");
                  user->idle_status=IDLE_ROOM_WARNED;
               }
            }
            break;
         case IDLE_ROOM_WARNED:
            if (tm < idle_time-60) user->idle_status=IDLE_NONE;
            else if (tm >= idle_time)
            {
               rm = get_room(globalSpecialRoomNames.getIdlersRoomName()->get().c_str());
               write_userf(user,"\07You feel two distinct beams of sonar cross your body as Glide and Tide press their rostrums gently against you. With several quick fluke kicks you are escorted to the %s\n",rm->name);
               write_room_exceptf(user->room,user,"An alerting click-train can be felt as Glide and Tide rocket effortlessly into the %s to escort %s to the %s.\n",user->room->name,get_visible_name(user).c_str(),rm->name);
               write_roomf(rm,"The silence is broken by two quick exhales as Glide and Tide escort %s in.\nGlide and Tide swim off in tandem formation.\n",get_visible_name(user).c_str());
               user->idle_before_room=user->room;
               move_user(user,rm,GO_MODE_SILENT);
               user->idle_status=IDLE_ROOM_IN_IT;
            }
            break;
         case  IDLE_ROOM_IN_IT:
            if (tm < idle_time && !strcmp(user->room->name,globalSpecialRoomNames.getIdlersRoomName()->get().c_str()))
            {
               user->idle_status=IDLE_NONE;
               move_user(user,user->idle_before_room,GO_MODE_GIANT_FLIPPER);
               user->idle_before_room=NULL;
            }
            else if( tm >= idle_time_disconnect-60 )
            {
               if(!user->afk) write_user(user,"\n\07~FY~OL~LI*** WARNING - Input within 1 minute or you will be disconnected. ***\n\n");
               else write_user(user,"\n\07~FY~OL~LI*** WARNING - Spending all your time AFK isn't the best way to get to know your fellow PODmates. Return from afk in 60 seconds or you will be disconnected. ***\n\n");
               user->idle_status=IDLE_DISCONNECT_WARNED;
            }
            break;
         case IDLE_DISCONNECT_WARNED:
            if (tm < (idle_time_disconnect-60))
            {
               user->idle_status=IDLE_NONE;
               if(user->idle_before_room!=NULL)
                     move_user(user,user->idle_before_room,GO_MODE_GIANT_FLIPPER);
               user->idle_before_room=NULL;
            }
            else if(tm >= idle_time_disconnect)
            {
               write_user(user,"\n\n\07~FR~OL~LI*** You have been timed out. ***\n\n");
               quit(user);
            }
            break;
         default:
            break;
            /* erm, file a complaint, we're not supposed to be here */
      }
      user=next;
   }
   return 0;
}

void atmospherics(void)
{
   RoomsVector::iterator roomNode;
   pod_string filename;
   XmlTextReader *reader;
   int line_num;
   int line_cnt;
   int count;
   pod_string line;
   bool inAtmosElement;
   bool inLine;
   bool hasRead;

   com_status = COM_ATMOS;

   for(roomNode=roomsList.begin();roomNode != roomsList.end();roomNode++)
   {
      reader         = 0;
      
      filename = ATMOSFILES;
      filename += "/";
      filename += (*roomNode)->name;
      filename += ".xml";
      
      inAtmosElement = false;
      inLine         = false;
      line           = "";      
      line_num       = 0;
      line_cnt       = 0;
      count          = 0;

      try
      {
         reader = new XmlTextReader( filename );   
      }
      catch (XmlError &e)
      {
         continue;
      }

      hasRead = reader->Read();
      while( hasRead )
      {
         int type = reader->NodeType();
         if( type == XML_READER_TYPE_ELEMENT || type == XML_READER_TYPE_END_ELEMENT )
         {
            pod_string name = reader->Name();

            if( type == XML_READER_TYPE_ELEMENT )
            {
               if( name == "atmospherics" )
               {
                  inAtmosElement = true;
                  try
                  {
                     count = stringToInt( reader->GetAttribute( "count" ) );
                  }
                  catch (NoSuchAttribute &e)
                  {
                     logStream << setLogfile( SYSLOG ) << noTime 
                              << "No 'count' attribute for element atmospherics found in file " << filename << ".\n" << pod_send;
                     continue;
                  }
                  line_num = random()%(long)count ; /* 0 .. 1-count */               
               }
               else if( name == "line" &&  inAtmosElement == true )
               {
                  inLine = true;
               }
            }
            else if( type == XML_READER_TYPE_END_ELEMENT )
            {
               if( name == "atmospherics" && inAtmosElement == true )
               {
                  inAtmosElement = false;
               }
               if( name == "line" && inLine == true )
               {
                  inLine = false;

                  if( line_cnt == line_num )
                  {
                     break;
                  }

                  line = "";
                  line_cnt++;
               }
            }

         }
         else if( type == XML_READER_TYPE_TEXT ||
                  type == XML_READER_TYPE_SIGNIFICANT_WHITESPACE )
         {
            if( inLine == true )
            {
               line += reader->Value();
            }
         }

         hasRead = reader->Read();
      }

      delete reader;
      
      write_roomf( *roomNode, "%s\n", line.c_str() );
   }
   com_status = COM_UNSET;
}

void timer_func(int unused)
{
   do_events_now++;
   reset_alarm();
}

void do_events()
{
   RoomsVector::iterator roomNode;
   static int atmos_cnt = 0;
   int c_pid;

   if(do_events_now == 0) return;
   if(do_events_now > 1)
   {
     write_syslogf("Missed %i heartbeats, busy server ?.\n",TRUE,do_events_now - 1);
   }
   do_events_now = 0;

   while( (c_pid = waitpid(-1,NULL,WNOHANG)) > 0 )
   {
      write_syslogf("Zombie # %i rests in piece now.\n",TRUE,c_pid);
   }

   check_reboot_shutdown();
   check_idle_and_timeout();
   hist_update();
   check_messages(NULL,0);

   atmos_cnt+=heartbeat.get();
   if(atmos_cnt>=atmos_delay.get())
   {
      if(atmos_on.get()) atmospherics();
      atmos_cnt=0;
   }

   for(roomNode=roomsList.begin();roomNode != roomsList.end();roomNode++)
   {
      if((*roomNode)->inked > 0)
      {
         (*roomNode)->inked -= heartbeat.get();
         if((*roomNode)->inked <= 0)
         {
            write_room((*roomNode),"A gentle current passes through the room and the ink dissipates returning visibility to normal.\n");
            (*roomNode)->inked = 0;
         }
      }
   }

}

int reset_alarm()
{
   signal(SIGALRM,timer_func); /* assigns a new signal handler to a certain signal */
   alarm(heartbeat.get());  /* schedule a SIGALRM signal to be send in heartbeat seconds */

   return 0;
}

/* User quits */
int quit(UR_OBJECT user)
{
   IntGlobalVar *accreq   = (IntGlobalVar*)user->getVar("accreq");

   if (accreq->get() == ACCREQ_NONE && user->level <= LEV_ZER)
   {
      write_user(user,"~OLMinimum requirements not completed.  Account will be ~FRdeleted.\n");
      delete_user(user,1);
   }
   else disconnect_user(user);
   return 0;
}

int recent(UR_OBJECT user)
{
   int cnt;
   write_user(user,"\n~OL~FT*** ~FWRecent Logins ~FT***\n\n");
   for ( cnt = 0; cnt < num_recent_users.get(); cnt++ )
   {
      write_user_crt( user, logged_users.get(cnt).c_str() );
   }
   write_user(user,"\n");
   return 0;
}

#ifndef __FreeBSD__
int is_d_file(const struct dirent *node)
#else
int is_d_file(struct dirent *node)
#endif
{
   static const pod_string userFileExtension = ".D.xml";
   
   pod_string name = node->d_name;
   
   if( name.size() >= userFileExtension.size() && 
       name.compare( name.size() - userFileExtension.size(), userFileExtension.size(), userFileExtension ) == 0 )
   {
      return TRUE;
   }
  
   return FALSE;
}

int timed(UR_OBJECT user)
{
   int count=0,cnt=0;
   struct dirent **dir_list;

   if( (count = scandir(USERFILES, &dir_list, is_d_file, alphasort)) == -1)
   {
      write_syslogf("Error in timed() while opening directory '%s' : '%s'.\n",TRUE,USERFILES,strerror(errno));
      return 0;
   }
   for(cnt=0;cnt<count;cnt++) free(dir_list[cnt]);
   free(dir_list);

   write_userf(user,"\n~OL~FB*** %s version %s - Time status ***\n\n",TALKER_NAME,RVERSION);
   write_userf(user,"~FTServer Time   : ~FY%s %s\n",long_date(2).c_str(),tzname[0]);
   show_uptime(user);
   show_total_uptime(user);
   write_userf(user,"~FTTotal Users   : ~FG%d Dwellers\n\n",count);
   return 0;
}

int room_ink(UR_OBJECT user)
{
   if(user->room->inked) write_user(user,"The room is inked already, unable to ink more.\n");
   else 
   {   
      write_roomf(user->room,"%s pulls a reluctant octopus out of a crack in the wall and shakes it wildly forcing it to fill the entire room with a black inky veil.\n",user->name);
      user->room->inked = 900;
   }
   return 0;
}

int room_filter(UR_OBJECT user)
{
   if(!user->room->inked) write_user(user,"The room is clear already.\n");
   else 
   {   
      write_room(user->room,"A gentle current passes through the room and the ink dissipates returning visibility to normal.\n");
      user->room->inked = 0;
   }
   return 0;
}

/*** End of File ***/

