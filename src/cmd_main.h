#ifndef CMD_MAIN_H
#define CMD_MAIN_H

typedef  int cmd_func_t(UR_OBJECT,char*);
typedef  int cmd_func_small_t(UR_OBJECT);

#include "help.h"
#include <set>
#include <map>

class Command
{
   protected:
      pod_string _name;
      int    _level;
      int    _cmd_grp;
      int    _id;
      char   _shortcut;
      int    _count;
      time_t _lastuse;
      long   _lastlength;
      cmd_func_t       *_func;
      cmd_func_small_t *_func_small;

   public:
      Command(pod_string name,int level,int cmd_grp, char shortcut, cmd_func_small_t *func);
      Command(pod_string name,int level,int cmd_grp, char shortcut, cmd_func_t *func);
      Command(pod_string name,int level,int cmd_grp,int id, char shortcut);
      void setLevel(int level);
      int getLevel();
      int getCommandGroup();
      char getShortcut();
      pod_string getName();
      int exec(UR_OBJECT user, char *inpstr);
      int getId();
      int getCount();
      time_t getLastUse();
      long getLastLength();
};

struct command_init
{
   char  *name;
   int    level;
   int    cmd_grp;
   int    id;
   char   shortcut;
};

typedef std::map<pod_string, Command*, std::less<pod_string>, pod_alloc< std::pair<pod_string, Command*> >::Type > CommandMap;
typedef std::set<Command*, std::less<Command*>, pod_alloc< Command* >::Type > CommandSet;
typedef std::map<char,   Command*, std::less<char>, pod_alloc< std::pair<char, Command*> >::Type > CommandShortcutsMap;
extern CommandMap commandMap;
extern CommandShortcutsMap commandShortcutsMap;

extern Command *currentCommand;

void cmd_init();
Command *cmd_add(char *name,int level,comgrpvals cmd_grp,cmd_func_t *func, char shortcut = 0);
Command *cmd_add(char *name,int level,comgrpvals cmd_grp,cmd_func_small_t *func, char shortcut = 0);
Command *get_cmd(pod_string name);

/* Values of commands , used in switch in exec_com()  */
/* No need to sort thisone anymore! whee! =)          */

#define COM_UNSET  -1
#define COM_ATMOS  -2 
#define COM_SYS    -3 

extern int com_status;
       
enum comvals {
QUIT,      LOOK,      MODE,      SAY,       SHOUT,
TELL,      EMOTE,     SEMOTE,    PEMOTE,    ECHO,      FORCE,       
GO,        IGNTOG,    PROMPT,    DESC,      VIEWMOTD2, PUBCOM,    
PRIVCOM,   LETMEIN,   INVITE,    FORWARDING,TOPIC,     MOVE,      
BCAST,     WHO,       PEOPLE,    FAQ,       HELP,      SHUTDOWN,  
READ,      WRITE,     JOIN,      BACK,      WIPE,      SEARCH,    
STATUS,    BRING,     VER,       RMAIL,     SMAIL,
DMAIL,     FROM,      LAST,      ENTPRO,    EXAMINE,   ROOMS,     
WIZZERLIST,PASSWD,    KILL,      ACTIONCMD, PROMOTE,   DEMOTE,    
LISTBANS,  BAN,       UNBAN,     SYSACTION, VIS,       INVIS,     
SITE,      WAKE,      WTELL,     WEMOTE,    MUZZLE,    UNMUZZLE,  
MAP,       LOGGING,   MINLOGIN,  DELOLD,    SYSTEM,    CHARECHO,  
CLEARLINE, FIX,       UNFIX,     SHOWCOM,   VIEWLOG,
CREATE,    DESTROY,   ARREST,    MYCLONES,  ALLCLONES, SWITCH,    
CSAY,      CHEAR,     RELEASE,   SWBAN,     AFK,       CLS,       
COLOR,     CURSE,     SUICIDE,   NUKE,      REBOOT,    ENCHANT,
RECOUNT,   SET,       RANKS,     MATCH,     MACROCMD,
SYSMACRO,  TIMED,     LIST,      THROW,     RECENT,    CBOT,
QBOT,      BOTACT,    COPYTO,    NOCOPIES,  VERIFY,    POP,
WANNABE,   STO,       WHISPER,   CLOAK,     CEMOTE,    WSTO,
ROUTE
};

int       exec_com(UR_OBJECT user, pod_string inpstr);
int       cmd_list_proc(UR_OBJECT user);
int       cmd_shortcutslist_proc(UR_OBJECT user);

#endif /* !CMD_MAIN_H */
