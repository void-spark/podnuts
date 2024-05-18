#ifndef USER_OBJECTS_H
#define USER_OBJECTS_H

typedef struct user_struct* UR_OBJECT;

extern UR_OBJECT user_first,user_last;

#include <sys/time.h>
#include <unistd.h>
#include "macro.h"
#include "cmd_main.h"
#include "rfc854.h"
#include "TelnetSender.h"
#include "TelnetHandler.h"
#include "QMethod.h"
#include "TelnetSocket.h"
#include "CRTLineBuffer.h"

#define USER_NAME_LEN  12
#define PASS_LEN       20 /* the 1st 8 chars will be used by crypt() though */
#define INPSTR_OLD_LEN 30

class user_struct
{
   public:
      BasicVar **data_ptr;
      char name[USER_NAME_LEN+1];

   /* things saved by load/save user */
      char pass[PASS_LEN+6];
      char last_site[MAX_SITE_LEN+1];
      int no_delold;
      int help_mode;
      MacroListGlobalVar macrolist;
      time_t last_login,last_login_len,total_login;      
      time_t cloak_store_last_login;
      time_t cloak_store_last_login_len;
      char   cloak_store_last_site[MAX_SITE_LEN+1];      
      time_t total_active_time,total_idle_time,total_afk_time,total_active_first;
      int mail_verified,autofwd;
      int level;
      int autoread;
      int hide_url;
      int hide_email;
      int cursed;
      char curse_name[80];
      int prompt;
      int muzzled;
      int tag;
      int command_mode;
      int jailed;

   /* things not saved by load/save user (SAVE_NOT_CURRENT)*/
      TelnetSocket *socket;
      int       afk;
      char      boardwrite[30];
      int       charcnt;
      int       cloaked;
      int       clone_hear;
      int       edit_op;
      int       edit_line;
      int       filepos;
      int       follow_mode;
      UR_OBJECT follow_partner;
      int       ignall;
      int       ignall_store;
      int       inedit;
      char      inpstr_old[INPSTR_OLD_LEN+1];
      RM_OBJECT invite_room;
      RM_OBJECT idle_before_room;
      time_t    last_input;
      char      *malloc_start;
      char      *malloc_end;
      int       misc_op;
      Command *nextCommand;
      UR_OBJECT owner;
      RM_OBJECT room;
      int       type;
      int       vis;
      int       idle_status;
   /* login users only */
      int       attempts;
      int       login;
   /* always boot generated */
      struct    user_struct *prev,*next;
   /* can be ignored without too much loss (I hope ^_^) */
      CRTLineBuffer  buffer;
      int       remoteEchoOn;

      void toXMLSoft( xmlTextWriterPtr ptr );
      void toXMLPerm( xmlTextWriterPtr ptr);
      void fromXMLSoft( XmlTextReader *reader );
      void fromXMLPerm( XmlTextReader *reader );
      bool load();
      void save();
      BasicVar* getVar( pod_string name );
};

int        get_num_of_users();
int        get_num_of_unhidden_users();
int        get_num_of_unhidden_wizs();
int        get_num_of_logins();
int        user_exists(UR_OBJECT user);
UR_OBJECT  temp_user_spawn(UR_OBJECT user,char* name,char* caller);
int        temp_user_destroy(UR_OBJECT user);
UR_OBJECT  create_user();
UR_OBJECT  create_user_adv(int link);
int        destruct_user(UR_OBJECT user);
int        purge_user_files(char* u_name);
void       new_init_user(UR_OBJECT user);

static const int GUA_SILENT      = 0;
static const int GUA_SHOW_ERRORS = 1;

static const int GUA_ROOM        = 0;
static const int GUA_TALKER      = 1;

static const int GUA_ALL         = 0;
static const int GUA_NOT_CLOAKED = 1;

UR_OBJECT get_user_advanced( UR_OBJECT user, pod_string name, int verbosity, int context, int who );
UR_OBJECT get_user_and_check( UR_OBJECT user, pod_string name );
UR_OBJECT get_user( pod_string name );

UR_OBJECT get_user_exactmatch(char* name);

enum
{  USR_SAVE_NEVR,
   USR_SAVE_SOFT,
   USR_SAVE_ALWS  };

int user_vars_proc(UR_OBJECT user);
int user_var_add_cust( pod_string name,ObjectCreator *var, int save_type );

#endif /* !USER_OBJECTS_H */
