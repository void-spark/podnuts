#include <sstream>
#include <map>
#include <set>
#include <string.h>
#include "general_headers.h"
#include "help.h"
#include "string_misc.h"
#include "move.h"
#include "xalloc.h"
#include "set.h"
#include "who.h"
#include "examine_stat.h"
#include "boards.h"
#include "smail.h"
#include "MailCopyToVar.h"
#include "GenericMessage.h"
#include "MailMessage.h"
#include "MailForward.h"
#include "time_utils.h"
#include "banning.h"
#include "color.h"
#include "macro.h"
#include "bot.h"
#include "tandem.h"
#include "telnet.h"
#include "curse.h"
#include "chitter.h"
#include "system.h"
#include "review.h"
#include "version.h"
#include "clones.h"
#include "dyn_str.h"
#include "admin.h"
#include "more.h"
#include "StringLibraryCommands.h"
#include "ignore.h"
#include "cmd_main.h"

Command *currentCommand;

Command::Command(pod_string name,int level,int cmd_grp, char shortcut, cmd_func_t *func)
{
   _name       = name;
   _level      = level;
   _cmd_grp    = cmd_grp;
   _id         = -1;
   _shortcut   = shortcut;
   _func       = func;
   _func_small = 0;
   _count=0;
   _lastuse=0;
   _lastlength=0;
}

Command::Command(pod_string name,int level,int cmd_grp, char shortcut, cmd_func_small_t *func)
{
   _name       = name;
   _level      = level;
   _cmd_grp    = cmd_grp;
   _id         = -1;
   _shortcut   = shortcut;
   _func       = 0;
   _func_small = func;
   _count=0;
   _lastuse=0;
   _lastlength=0;
}

Command::Command(pod_string name,int level,int cmd_grp,int id, char shortcut)
{
   _name       = name;
   _level      = level;
   _cmd_grp    = cmd_grp;
   _id         = id;
   _shortcut   = shortcut;
   _func       = 0;
   _func_small = 0;
   _count=0;
   _lastuse=0;
   _lastlength=0;
}

void Command::setLevel(int level)
{
   _level = level;
}

int Command::getLevel()
{
   return _level;
}

int Command::getCommandGroup()
{
   return _cmd_grp;
}

pod_string Command::getName()
{
   return _name;
}

int Command::getId()
{
   return _id;
}

int Command::getCount()
{
   return _count;
}

time_t Command::getLastUse()
{
   return _lastuse;
}

long Command::getLastLength()
{
   return _lastlength;
}

char Command::getShortcut()
{
   return _shortcut;
}

int Command::exec(UR_OBJECT user, char *inpstr)
{
   char filename[80];
   struct timeval current_time;
   struct timeval after_time;

#warning make this global or something, and addable, or loaded from a file \
 do the same for (level == LEV_MIN) (F2-LEV_MIN) too, when arrested, btw.! since youre already void of \
 quite some commands at this level (like .shout) is it still needed to have anythign at all for \
 those ??. (yes, cause we can lower or raise command levels dynamicly)

   typedef std::set< pod_string, std::less<pod_string>, pod_alloc< pod_string >::Type > StringsSet;
   StringsSet denyWhenMuzzledCmds;
   denyWhenMuzzledCmds.insert("bcast");
   denyWhenMuzzledCmds.insert("emote");
   denyWhenMuzzledCmds.insert("shemote");
   denyWhenMuzzledCmds.insert("pemote");
   denyWhenMuzzledCmds.insert("echo");
   denyWhenMuzzledCmds.insert("show");
   denyWhenMuzzledCmds.insert("sto");
   denyWhenMuzzledCmds.insert("wake");
   denyWhenMuzzledCmds.insert("shout");
   denyWhenMuzzledCmds.insert("tell");
   denyWhenMuzzledCmds.insert("whisper");
   denyWhenMuzzledCmds.insert("wtell");
   denyWhenMuzzledCmds.insert("wemote");
   denyWhenMuzzledCmds.insert("wsto");
   denyWhenMuzzledCmds.insert("ping");
   denyWhenMuzzledCmds.insert("ecco");
   denyWhenMuzzledCmds.insert("eecco");
   denyWhenMuzzledCmds.insert("csay");
   denyWhenMuzzledCmds.insert("cemote");
   denyWhenMuzzledCmds.insert("write");
   denyWhenMuzzledCmds.insert("smail");
   denyWhenMuzzledCmds.insert("tandem");
   denyWhenMuzzledCmds.insert("picture");
   denyWhenMuzzledCmds.insert("ppicture");

   if( user->muzzled && denyWhenMuzzledCmds.find( _name ) != denyWhenMuzzledCmds.end() )
   {
      write_userf( user, "You are muzzled, you cannot use %s.\n", _name.c_str() );
      return 0;
   }

   gettimeofday(&(current_time),NULL);

   _count++;
   _lastuse = time(0);
   if(_func)
   {
      _func(user,inpstr);
   }
   else if(_func_small)
   {
      _func_small(user);
   }
   else
   {
      com_status = _id;
      /* Main switch */
      switch(_id)
      {
         case QUIT	: quit(user);  			  break;
         case LOOK	: look(user);         break;
         case MODE	: toggle_mode(user);  break;
         case SAY	  :
            if (words.word_count<2)
            {
               write_user(user,"Say what?\n");
               return 0;
            }
            else
            {
               say_gen(user,inpstr);
            }
            break;
         case WHISPER	: whisper(user,inpstr);		break;
         case STO	: say_to(user,inpstr);		break;
         case SHOUT 	: shout(user,inpstr);  		break;
         case TELL  	: tell(user,inpstr);   		break;
         case EMOTE 	: emote(user,inpstr);  		break;
         case SEMOTE	: semote(user,inpstr); 		break;
         case PEMOTE	: pemote(user,inpstr); 		break;
         case ECHO  	: echo(user,inpstr);   		break;
         case GO    	: go(user);  			break;
         case IGNTOG	: toggle_ignore(user);  	break;
         case PROMPT	: toggle_prompt(user);  	break;
         case DESC  	: set_desc(user,inpstr);  	break;
         case CLOAK	: cloak(user,inpstr);			break;
         case PUBCOM 	:
         case PRIVCOM	: set_room_access(user);  	break;
         case LETMEIN	: letmein(user);  		break;
         case INVITE 	: invite(user);   		break;
         case TOPIC  	: set_topic(user,inpstr);  	break;
         case MOVE   	: move(user);  			break;
         case BCAST  	: bcast(user,inpstr);  		break;
         case WHO    	: who(user,0);  		break;
         case CEMOTE	: clone_emote(user,inpstr);	break;
         case PEOPLE 	: who(user,1);  		break;
         case HELP   	: help(user);  			break;
         case SHUTDOWN	: shutdown_com(user); 		break;
         case WIZZERLIST: wizon(user);			break;
         case VIEWMOTD2:
                sprintf(filename,"%s",MOTD2);
                switch(more(user,NULL,filename)) {
                        case 0: write_user(user,"There is no news.\n"); break;
                        case 1: user->misc_op=MISC_MORE;
                        }
                break;
         case READ  	: read_board(user);  		break;
         case WRITE 	: write_board(user,inpstr,0);  	break;
         case WIPE  	: wipe_board(user,inpstr);  	break;
         case SEARCH	: search_boards(user);  	break;
         case BACK	: back(user);			break;
         case STATUS	: ustatus(user);  		break;
         case VER       : show_version(user);           break;
         case RMAIL   	: rmail(user);  		break;
         case SMAIL   	: smail(user,inpstr,0);  	break;
         case DMAIL   	: dmail(user);  		break;
         case FROM    	: mail_from(user);  		break;
         case ENTPRO  	: enter_profile(user,0);  	break;
         case EXAMINE 	: examine(user);  		break;
         case ROOMS    	: rooms(user);  		break;
         case PASSWD  	: change_pass(user);  		break;
         case KILL    	: kill_user(user);  		break;
         case PROMOTE 	: promote(user);  		break;
         case DEMOTE  	: demote(user);  		break;
         case LISTBANS	: listbans(user);  		break;
         case BAN     	: ban(user,inpstr); 		break;
         case UNBAN   	: unban(user,inpstr);  		break;
         case VIS     	: visibility(user,1);  		break;
         case INVIS   	: visibility(user,0);  		break;
         case SITE    	: site(user);  			break;
         case WAKE    	: wake(user,inpstr);  		break;
         case WTELL   	: wtell(user,inpstr);	  	break;
         case WEMOTE  	: wemote(user,inpstr);  	break;
         case WSTO	: wsto(user,inpstr);		break;
         case MUZZLE  	: muzzle(user);  		break;
         case UNMUZZLE	: unmuzzle(user);  		break;
         case MAP:
		sprintf(filename,"%s/%s",DATAFILES,MAPFILE);
		switch(more(user,NULL,filename)) {
		         case 0: write_user(user,"There is no map.\n");  break;
		         case 1: user->misc_op=MISC_MORE;
			}
		break;
         case LOGGING  	: logging(user); 		break;
         case MINLOGIN 	: minlogin(user);  		break;
         case SYSTEM   	: system_details(user);  	break;
         case CLEARLINE	: clearline(user);  		break;
         case FIX      	: change_room_fix(user,1);  	break;
         case UNFIX    	: change_room_fix(user,0);  	break;
         case CREATE   	: create_clone(user);  		break;
         case DESTROY  	: destroy_clone(user);  	break;
         case MYCLONES 	: myclones(user);  		break;
         case ALLCLONES	: allclones(user);  		break;
         case SWITCH	: clone_switch(user);  		break;
         case CSAY  	: clone_say(user,inpstr);  	break;
         case CHEAR 	: clone_hear(user);  		break;
         case SWBAN 	: swban(user);  		break;
         case AFK   	: afk(user,inpstr);  		break;
         case CLS   	: cls(user);  			break;
         case COLOR  	: toggle_color_on(user);  	break;
         case SUICIDE 	: suicide(user);  		break;
         case NUKE     	: delete_user(user,0);  	break;
         case REBOOT  	: reboot_com(user);  		break;
         case RECOUNT 	: check_messages(user,2);  	break;
         case POP	: pop_level(user,inpstr);	break;
         case VERIFY 	: verify_email(user);  		break;
         case FORWARDING:
           switch(forwarding.get()) {
              case 0: write_user(user,"You have turned ~FGon~RS smail auto-forwarding.\n");
                      forwarding.set(1);
                      logStream << setLogfile( SYSLOG ) << user->name << " turned ON mail forwarding.\n" << pod_send;
                      break;
              case 1: write_user(user,"You have turned ~FRoff~RS smail auto-forwarding.\n");
                      forwarding.set(0);
                      logStream << setLogfile( SYSLOG ) << user->name << " turned OFF mail forwarding.\n" << pod_send;
                      break;
              }
              break;
         case COPYTO	: copies_to(user,inpstr);	break;
         case NOCOPIES	: nocopies(user,inpstr);	break;
         case FORCE	: force(user,inpstr);		break;
         case LIST	: list_users(user);		break;
         case CURSE 	: curse(user);			break; 	
         case ENCHANT	: remove_curse(user);		break;
         case LAST   	: last_login(user);		break;
         case ARREST 	: jail_user(user,inpstr);	break;
         case RELEASE	: unjail_user(user,inpstr);	break;
         case JOIN   	: join(user);			break;
         case BRING  	: bring(user);			break;
         case SHOWCOM 	: show(user,inpstr);		break;
         case TIMED 	: timed(user);			break;
         case SET 	: opts_set(user,inpstr);    		break;
         case FAQ 	: ranks_faq(user,1);		break;
         case RANKS 	: ranks_faq(user,0);		break;
         case DELOLD 	: delold_users(user);		break;
         case MACROCMD	: user->macrolist.cmd(inpstr,user,0); break;
         case ACTIONCMD	: list_action_macros(user);	break;
         case ROUTE   : user->room->PathTo(user);break;
         case SYSMACRO:
         {
            system_macrolist.cmd(inpstr,user,0);
            pod_string filename = DATAFILES;
            filename += "/";
            filename += SYSMACROFILE;
            system_macrolist.saveMacros((char*)filename.c_str());
            break;
         }
         case SYSACTION:
         {
            system_actionlist.cmd(inpstr,user,1);
            pod_string filename = DATAFILES;
            filename += "/";
            filename += SYSACTIONFILE;
            system_actionlist.saveMacros((char*)filename.c_str());
            break;
         }
         case THROW 	: throw_user(user);  		break;
         case RECENT 	: recent(user);  		break;
         case CBOT	: start_bot(user);		break;
         case QBOT	: stop_bot(user);		break;
         case BOTACT	: bot_act(user,inpstr);		break;
         default	: write_user(user,"Command not executed in exec_com().\n");
      }	/* !switch */
   }
   gettimeofday(&(after_time),NULL);
   _lastlength=timeval_diff_usec(&after_time,&current_time);
   return 0;
}

int com_status;

CommandMap commandMap;
CommandShortcutsMap commandShortcutsMap;

/* our commands list, my it's a bigone! :) */
struct command_init command_list[] =
                    { { "quit",      LEV_MIN, GEN,   QUIT,       '\0' },
                      { "look",      LEV_ONE, ROOM,  LOOK,       '\0' },
                      { "mode",      LEV_FIV, GEN,   MODE,       '\0' },
                      { "say",       LEV_MIN, SPCH,  SAY,        '\'' },
                      { "shout",     LEV_TWO, SPCH,  SHOUT,      '!'  },
                      { "tell",      LEV_TWO, SPCH,  TELL,       ','  },
                      { "emote",     LEV_ZER, SPCH,  EMOTE     , ';'  },
                      { "shemote",   LEV_TWO, SPCH,  SEMOTE    , '#'  },
                      { "pemote",    LEV_TWO, SPCH,  PEMOTE    , '/'  },
                      { "echo",      LEV_THR, SPCH,  ECHO      , '-'  },
                      { "force",     LEV_FIV, ADM,   FORCE     , '\0' },
                      { "go",        LEV_TWO, ROOM,  GO        , '>'  },
                      { "prompt",    LEV_ZER, GEN,   PROMPT    , '\0' },
                      { "desc",      LEV_ZER, INF,   DESC      , '\0' },
                      { "motd",      LEV_ZER, GEN,   VIEWMOTD2 , '\0' },
                      { "public",    LEV_TWO, ROOM,  PUBCOM    , '\0' },
                      { "private",   LEV_TWO, ROOM,  PRIVCOM   , '\0' },
                      { "letmein",   LEV_TWO, ROOM,  LETMEIN   , '\0' },
                      { "invite",    LEV_TWO, ROOM,  INVITE    , '\0' },
                      { "forwarding",LEV_THR, MSG,   FORWARDING, '\0' },
                      { "topic",     LEV_TWO, GEN,   TOPIC     , '\0' },
                      { "move",      LEV_THR, ROOM,  MOVE      , '\0' },
                      { "bcast",     LEV_FOU, SPCH,  BCAST     , '\0' },
                      { "who",       LEV_ZER, INF,   WHO       , '@'  },
                      { "people",    LEV_THR, INF,   PEOPLE    , '\0' },
                      { "faq",	     LEV_ONE, INF,   FAQ       , '\0' },
                      { "help",      LEV_ZER, INF,   HELP      , '?'  },
                      { "shutdown",  LEV_FOU, SYS,   SHUTDOWN  , '\0' },
                      { "read",      LEV_ONE, MSG,   READ,       '\0' },
                      { "write",     LEV_TWO, MSG,   WRITE     , '\0' },	
                      { "join",      LEV_TWO, ROOM,  JOIN      , '\0' },
                      { "back",      LEV_ONE, ROOM,  BACK      , '\0' },	
                      { "wipe",      LEV_THR, MSG,   WIPE      , '\0' },	
                      { "search",    LEV_TWO, MSG,   SEARCH    , '\0' },	
                      { "status",    LEV_ONE, INF,   STATUS    , '\0' },	
                      { "bring",     LEV_TWO, ROOM,  BRING     , '\0' },	
                      { "version",   LEV_ONE, INF,   VER       , '\0' },	
                      { "rmail",     LEV_MIN, MSG,   RMAIL     , '\0' },	
                      { "smail",     LEV_TWO, MSG,   SMAIL     , '\0' },	
                      { "dmail",     LEV_TWO, MSG,   DMAIL     , '\0' },	
                      { "from",      LEV_TWO, MSG,   FROM      , '\0' },	
                      { "last",      LEV_ONE, INF,   LAST      , '\0' },	
                      { "entpro",    LEV_ONE, INF,   ENTPRO    , '\0' },	
                      { "examine",   LEV_ONE, INF,   EXAMINE   , '&'  },	
                      { "rooms",     LEV_TWO, INF,   ROOMS     , '\0' },	
                      { "wizlist",   LEV_ZER, INF,   WIZZERLIST, '\0' },
                      { "passwd",    LEV_ZER, GEN,   PASSWD    , '\0' },	
                      { "kill",      LEV_FOU, ADM,   KILL      , '\0' },	
                      { "actions",   LEV_TWO, INF,   ACTIONCMD , '\0' },	
                      { "promote",   LEV_FOU, ADM,   PROMOTE   , '\0' },	
                      { "demote",    LEV_FOU, ADM,   DEMOTE    , '\0' },	
                      { "listbans",  LEV_THR, ADM,   LISTBANS  , '\0' },
                      { "ban",       LEV_FIV, ADM,   BAN       , '\0' },	
                      { "unban",     LEV_FIV, ADM,   UNBAN     , '\0' },	
                      { "sysaction", LEV_FIV, GEN,   SYSACTION , '\0' },	
                      { "vis",       LEV_TWO, FUN,   VIS       , '\0' },	
                      { "invis",     LEV_TWO, FUN,   INVIS     , '\0' },	
                      { "site",      LEV_THR, INF,   SITE      , '\0' },	
                      { "wake",      LEV_TWO, SPCH,  WAKE      , '\0' },	
                      { "wtell",     LEV_THR, SPCH,  WTELL     , '\0' },	
                      { "wemote",    LEV_THR, SPCH,  WEMOTE    , '\0' },
                      { "muzzle",    LEV_THR, ADM,   MUZZLE    , '\0' },
                      { "unmuzzle",  LEV_THR, ADM,   UNMUZZLE  , '\0' },
                      { "map",       LEV_TWO, INF,   MAP       , '\0' },
                      { "logging",   LEV_FIV, SYS,   LOGGING   , '\0' },	
                      { "minlogin",  LEV_FIV, ADM,   MINLOGIN  , '\0' },	
                      { "delold",    LEV_FIV, ADM,   DELOLD    , '\0' },	
                      { "system",    LEV_FOU, INF,   SYSTEM    , '\0' },	
                      { "clearline", LEV_FIV, ADM,   CLEARLINE , '\0' },	
                      { "fix",       LEV_FIV, ADM,   FIX       , '\0' },	
                      { "unfix",     LEV_FIV, ADM,   UNFIX     , '\0' },	
                      { "show",      LEV_THR, FUN,   SHOWCOM   , '\0' },	
                      { "clone",     LEV_THR, CLO,   CREATE    , '\0' },	
                      { "destroy",   LEV_THR, CLO,   DESTROY   , '\0' },	
                      { "arrest",    LEV_THR, ADM,   ARREST    , '\0' },	
                      { "myclones",  LEV_THR, CLO,   MYCLONES  , '\0' },	
                      { "allclones", LEV_TWO, CLO,   ALLCLONES , '\0' },	
                      { "switch",    LEV_THR, CLO,   SWITCH    , '\0' },	
                      { "csay",      LEV_THR, CLO,   CSAY      , '\0' },	
                      { "chear",     LEV_THR, CLO,   CHEAR     , '\0' },	
                      { "release",   LEV_THR, ADM,   RELEASE   , '\0' },	
                      { "swban",     LEV_FIV, ADM,   SWBAN     , '\0' },	
                      { "afk",       LEV_ONE, GEN,   AFK       , '\0' },	
                      { "cls",       LEV_MIN, GEN,   CLS       , '\0' },	
                      { "color",     LEV_ONE, GEN,   COLOR     , '\0' },	
                      { "curse",     LEV_THR, ADM,   CURSE     , '\0' },	
                      { "ignore",    LEV_TWO, SPCH,  IGNTOG    , '\0' },
                      { "suicide",   LEV_MIN, GEN,   SUICIDE   , '\0' },	
                      { "nuke",      LEV_FIV, ADM,   NUKE      , '\0' },	
                      { "reboot",    LEV_FOU, SYS,   REBOOT    , '\0' },	
                      { "enchant",   LEV_THR, ADM,   ENCHANT   , '\0' },	
                      { "recount",   LEV_FIV, SYS,   RECOUNT   , '\0' },	
                      { "set",       LEV_ZER, INF,   SET       , '\0' },	
                      { "ranks",     LEV_ZER, INF,   RANKS     , '\0' },	
                      { "macro",     LEV_ONE, GEN,   MACROCMD  , '\0' },	
                      { "sysmacro",  LEV_FIV, GEN,   SYSMACRO  , '\0' },	
                      { "time",      LEV_ONE, INF,   TIMED     , '\0' },	
                      { "ulist",     LEV_THR, ADM,   LIST      , '\0' },	
                      { "throw",     LEV_THR, ROOM,  THROW     , '\0' },	
                      { "route",     LEV_TWO, ROOM,  ROUTE     , '\0' },	
                      { "recent",    LEV_TWO, INF,   RECENT    , '\0' },	
                      { "cbot",      LEV_FOU, BOTY,  CBOT      , '\0' },	
                      { "qbot",      LEV_FOU, BOTY,  QBOT      , '\0' },
                      { "bact",      LEV_FIV, BOTY,  BOTACT    , '\0' },
                      { "copyto",    LEV_TWO, MSG,   COPYTO    , '\0' },	
                      { "nocopy",    LEV_TWO, MSG,   NOCOPIES,   '\0' },	
                      { "verify",    LEV_TWO, MSG,   VERIFY    , '\0' },	
                      { "pop",       LEV_FIV, ADM,   POP       , '\0' },	
                      { "sto",       LEV_ONE, SPCH,  STO       , '\0' },	
                      { "whisper",   LEV_TWO, SPCH,  WHISPER   , '\0' },	
                      { "cloak",     LEV_THR, ADM,   CLOAK     , '\0' },	
                      { "cemote",    LEV_THR, CLO,   CEMOTE    , '\0' },
                      { "wsto",      LEV_THR, SPCH,  WSTO      , '\0' },	
                      { "*"       ,  0,   0,     0         , '\0' } };

struct str_grp
{
   char *str;
   int   id;
} str_grps[] = {  { "MSG",  MSG   },
                  { "ROOM", ROOM  },
                  { "GEN",  GEN   },
                  { "SPCH", SPCH  },
                  { "SYS",  SYS   },
                  { "INF",  INF   },
                  { "ADM",  ADM   },
                  { "FUN",  FUN   },
                  { "CLO",  CLO   },
                  { "BOTY", BOTY  },
                  { "CAR",  CAR   },
                  { "FIG",  FIG   },
                  { "BOA",  BOA   },
                  { "*",    0     } };

int change_com_level(UR_OBJECT user,char *inpstr);

int cmd_list_gen(UR_OBJECT user,char *inpstr)
{
   CommandMap::iterator iterator;
   Command* command;
   FILE *fp;
   int cnt;

   if(!(fp=fopen("logfiles/cmd_list.default","w")))
   {
      write_syslogf("Failed to open file in cmd_list_gen().\n",TRUE);
      return 0;
   }

   for(iterator=commandMap.begin();iterator!=commandMap.end();iterator++)
   {
      command = iterator->second;
      for(cnt=0;str_grps[cnt].str[0] != '*';cnt++)
      {
         if(str_grps[cnt].id == command->getCommandGroup()) break;
      }
      if( command->getShortcut() ) fprintf(fp,"%-12s %-4s %-4s %c\n",command->getName().c_str(),getLevelShortName(command->getLevel()),str_grps[cnt].str,command->getShortcut());
      else                         fprintf(fp,"%-12s %-4s %-4s\n",   command->getName().c_str(),getLevelShortName(command->getLevel()),str_grps[cnt].str);
   }

   fclose(fp);

   return 0;
}

int cmd_list_proc(UR_OBJECT user)
{
   CommandMap::iterator iterator;
   Command* command;
   int cnt;

   write_seperator_line(user," commandlist ");

   write_userf(user,"~FTKEY              : NAME         LVL  GRP  SHC USE LAST_RUNTIME SECS_AGO\n");
   for(iterator=commandMap.begin();iterator!=commandMap.end();iterator++)
   {
      command = iterator->second;
      for(cnt=0;str_grps[cnt].str[0] != '*';cnt++)
      {
         if(str_grps[cnt].id == command->getCommandGroup()) break;
      }

      if( command->getShortcut() )
      {
         write_userf(user,"~FT%-12s~RS     : %-12s %-4s %-4s %c   %d   %4ld:%03ld:%03ld %10d\n",
                        iterator->first.c_str(),
                        command->getName().c_str(),
                        getLevelShortName(command->getLevel()),
                        str_grps[cnt].str,
                        command->getShortcut(),
                        command->getCount(),
                        command->getLastLength()/1000000,
                        (command->getLastLength()%1000000)/1000,
                        command->getLastLength()%1000,
                        command->getLastUse() ? (int)(time(0) - command->getLastUse()) : 0 );
      }
      else
      {
         write_userf(user,"~FT%-12s~RS     : %-12s %-4s %-4s     %d   %4ld:%03ld:%03ld %10d\n",
                        iterator->first.c_str(),
                        command->getName().c_str(),
                        getLevelShortName(command->getLevel()),
                        str_grps[cnt].str,
                        command->getCount(),
                        command->getLastLength()/1000000,
                        (command->getLastLength()%1000000)/1000,
                        command->getLastLength()%1000,
                        command->getLastUse() ? (int)(time(0) - command->getLastUse()) : 0 );
      }
   }
   write_userf(user,"Total: %i\n",commandMap.size());
   write_seperator_line(user,NULL);
   write_user(user,"\n");

   return 0;
}

int cmd_shortcutslist_proc(UR_OBJECT user)
{
   CommandShortcutsMap::iterator iterator;
   Command* command;
   int cnt;

   write_seperator_line(user," shortcutslist ");
   write_userf(user,"~FTKEY     : NAME         LVL  GRP  SHORTCUT\n");

   for(iterator=commandShortcutsMap.begin();iterator!=commandShortcutsMap.end();iterator++)
   {
      command = iterator->second;
      for(cnt=0;str_grps[cnt].str[0] != '*';cnt++)
      {
         if(str_grps[cnt].id == command->getCommandGroup()) break;
      }
      write_userf(user,"~FT'%c'~RS     : %-12s %-4s %-4s %c\n",iterator->first,command->getName().c_str(),getLevelShortName(command->getLevel()),str_grps[cnt].str,command->getShortcut());
   }
   write_seperator_line(user,NULL);
   write_user(user,"\n");

   return 0;
}


int cmd_add(Command *command)
{
   if (!command)
   {
      write_syslog("In function cmd_add: null pointer given!\n",TRUE);
      return 1;
   }

   commandMap[command->getName()] = command;
   if(command->getShortcut())
   {
      commandShortcutsMap[command->getShortcut()] = command;
   }
   return 0;
}

Command *cmd_add(char *name,int level,comgrpvals cmd_grp,cmd_func_small_t *func, char shortcut)
{
   Command *newCommand = new Command(name,level,cmd_grp,shortcut,func);

   if (!newCommand)
   {
      write_syslog("In function cmd_add: Out of memory!\n",TRUE);
      abort();
   }
   cmd_add(newCommand);
   return newCommand;
}

Command *cmd_add(char *name,int level,comgrpvals cmd_grp,cmd_func_t *func, char shortcut)
{
   Command *newCommand = new Command(name,level,cmd_grp,shortcut,func);

   if (!newCommand)
   {
      write_syslog("In function cmd_add: Out of memory!\n",TRUE);
      abort();
   }
   cmd_add(newCommand);
   return newCommand;
}


void cmd_init()
{
   int i;
   Command *newCommand;

   for(i=0;command_list[i].name[0]!='*';i++)
   {
      newCommand = new Command(command_list[i].name,
                               command_list[i].level,
                               command_list[i].cmd_grp,
                               command_list[i].id,
                               command_list[i].shortcut);

      if (!newCommand)
      {
         write_syslog("In function cmd_init: Out of memory!\n",TRUE);
         abort();
      }

      cmd_add(newCommand);
   }
   cmd_add("cmdlist",   LEV_THR, ADM,  &cmd_list_gen);

   cmd_add("credits",   LEV_ONE, INF,  &help_credits);
   cmd_add("tandem",    LEV_TWO, ROOM, &follow_ask);
   cmd_add("accept",    LEV_ONE, ROOM, &follow_accept);
   cmd_add("break",     LEV_ONE, ROOM, &follow_break);
   cmd_add("nodelold",  LEV_FIV, ADM,  &no_delold);
   cmd_add("changelev", LEV_FIV, ADM,  &change_com_level);
   cmd_add("ulog",      LEV_THR, ADM,  &log_user_com);
   cmd_add("tag",       LEV_THR, ADM,  &tag_user);
   cmd_add("untag",     LEV_THR, ADM,  &untag_user);
   cmd_add("ink",       LEV_TWO, FUN,  &room_ink);
   cmd_add("filter",    LEV_TWO, FUN,  &room_filter);
   cmd_add("procstat",  LEV_FOU, INF,  &proc_status);
   cmd_add("softboot",  LEV_FOU, SYS,  &talker_softboot_command);
   cmd_add("reload",    LEV_FOU, SYS,  &reloadStringLibrary);
   cmd_add("rl_arrs",   LEV_FOU, SYS,  &dyn_arrs_reload);
}

Command* get_cmd(pod_string name)
{
   CommandMap::iterator iterator = commandMap.find(name);

   if(iterator == commandMap.end())
   {
      return 0;
   }

   return iterator->second;
}

Command* get_cmd_fuzzy(pod_string name)
{
   CommandMap::iterator upper_bound,previous;

   upper_bound=commandMap.upper_bound(name);
   if( upper_bound != commandMap.begin() )
   {
      previous = upper_bound;
      previous--;
      if(!previous->first.compare(name))
      {
         return previous->second;
      }
   }
   if( upper_bound == commandMap.end() )
   {
      return 0;
   }
   if(!upper_bound->first.compare(0,name.size(),name))
   {
      return upper_bound->second;
   }
   return 0;
}

/*** Deal with user input ***/
int exec_com( UR_OBJECT user, pod_string input )
{
   Command *command=0;
   CommandShortcutsMap::iterator shortcutsIterator;
   char *comword=NULL;
   bool found_short=false;

   if( strlen(words.word[0]) == 1 )
   {
      shortcutsIterator = commandShortcutsMap.find(words.word[0][0]);
      if( shortcutsIterator != commandShortcutsMap.end() )
      {
         pod_string::size_type pos;
         command = shortcutsIterator->second;
         words.word[0]=(char*)command->getName().c_str();
         
         pos = 1;
         while(input[pos] < 33 && input[pos] )
         {
            pos++;
         }
         input.erase(0,pos);
         
         found_short = true;
      }
   }

   if ( !(user->command_mode || words.word[0][0] == '.' || found_short))
   {
      say_gen( user, input.c_str() );
      return 0;
   }

   if( !found_short )
   {
      input = remove_first( input );
   }

   if (words.word[0][0]=='.') comword = words.word[0] + 1;
   else                       comword = words.word[0];

   if (!comword[0]) 
   {
      write_user(user,"Unknown command.\n");  
      return 0;
   }

   com_status = COM_UNSET;

   if(!strncmp(comword,"quit",strlen(comword)))
   {
      command = get_cmd("quit");
   }
   else if( !(command = get_cmd_fuzzy(comword)) || command->getLevel() > user->level)
   {
      write_user(user,"Unknown command.\n");
      return 0;
   }
   currentCommand=command;
   # warning evil cast, away from const!
   command->exec( user, (char*)input.c_str() );
   currentCommand=0;

   return 0;
}


int change_com_level(UR_OBJECT user,char *inpstr)
{
   int oldlev,lev;
   char *comword=0;
   Command *command=0;
   WriteRoomStream *roomStream = WriteRoomStream::getInstance();

   if (words.word_count < 3) write_user(user,"Usage: changelev <command> <level>\n");
   else
   {
      comword=words.word[1];

      if( !(command = get_cmd_fuzzy(comword)) )
      {
         write_user(user,"Unknown command given as parameter.\n");
      }
      else
      {
         pod_string commandString = words.word[2];
         strToUpper(commandString);
         lev=get_level(commandString.c_str());
         if(lev == -1 || lev == LEV_BOT) write_user(user,"Unknown level identifier.\n");
         else
         {
            oldlev=command->getLevel();
            command->setLevel(lev);

            pod_ostringstream log_stringstream;
            log_stringstream << "~OL" << user->name << " ~RSchanges the command~OL " << command->getName()
                           << "~RS from level " << getLevelName(oldlev) << " to level: ~FR~OL" << getLevelName(lev) << "\n";

            pod_string log_string = log_stringstream.str();

            *roomStream << setRoom( NULL ) << log_string << pod_send;
            logStream  << setLogfile( SYSLOG ) << color_com_strip(log_string) << pod_send;
         }
      }
   }
   return 0;
}

