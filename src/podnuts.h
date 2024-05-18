/*****************************************************************************

            ______   _______   _____                   __         
           |   __ \ |       | |     \    .-----.--.--.|  |_.-----.
           |    __/ |   -   | |  --  |   |     |  |  ||   _|__ --|
           |___|    |_______| |_____/____|__|__|_____||____|_____|
                                   |______|                              

    Based on rNUTS 3.0.2 by Engi & Slugger which is a heavily modified
           form of NUTS version 3.3.3 (C)Neil Robertson 1996
       Neil Robertson                Email    : neil@ogham.demon.co.uk

                  POD_nuts modifications by Vaghn & Dolfin
                 Extra coding help courtesy of Crandonkphin
                               VERSION 1.15

**************************** Header file POD_nuts **************************/

#ifndef PODNUTS_H
#define PODNUTS_H

#include <stdio.h>
#include "Room.h"
#include "pod_string.h"
#include "review.h"

#define ADMINBOARD    "admnotes"
#define NEWSBOARD     "news"
#define UPGRADESBOARD "upgrades"

#define NO_OVERR(user,u) (user->level < LEV_FIV || u->level > user->level)


enum { MISC_NONE = 0,
       MISC_MORE,
       MISC_BOARD,
       MISC_MAIL,
       MISC_PROFILE,
       MISC_SUICIDE,
       MISC_DEL_MAIL,
       MISC_WARM_BOOT   };

#define VERIFYCODE   "TR-684C."   /* followed by a random number */

#define LOG_STRING_MAX 80
#define LOG_LINES      10
#define FILE_NAME_LEN  80

#define MAX_MORE_LINES 23
#define OUT_BUFF_SIZE  1000
#define WORD_LEN       40
#define MAX_LINES      15

#define NOISE_LEN      15
#define COLOR_LEN      9 
//#define USER_DESC_LEN  58
#define USER_DESC_LEN  56
#define GEND_DESC_LEN   8
#define GEND_CHOICE_LEN 6
#define SPECIES_LEN    24
#define AGE_LEN        3
#define BDAY_LEN       5
#define MOOD_LEN       4
#define TOTAL_MOOD_LEN (4+6*3) /* 6 colorcodes allowed , else, kapoof! */

#define URL_LEN        45
#define EMAIL_LEN      45
#define ICQ_LEN        10
#define AFK_MESG_LEN   60
#define PHRASE_LEN     69
#define ENT_LVE_LEN    PHRASE_LEN /*by crandonkphin, enter/ leave phrase max length */
#define SERV_NAME_LEN  80
#define SITE_NAME_LEN  80
#define VERIFY_LEN     20
#define DNL            12

enum {
        ROOM_PUBLIC        = 0,
        ROOM_PRIVATE       = 1,
        ROOM_FIXED         = 2,
        ROOM_FIXED_PUBLIC  = 2,
        ROOM_FIXED_PRIVATE = 3
};

#define USER_TYPE          0
#define CLONE_TYPE         1

#define CLONE_HEAR_NOTHING 0
#define CLONE_HEAR_SWEARS  1
#define CLONE_HEAR_ALL     2

#define IDLE_NONE                0
#define IDLE_ROOM_WARNED         1
#define IDLE_ROOM_IN_IT          2
#define IDLE_DISCONNECT_WARNED   3

struct review_struct;

enum boolean {FALSE, TRUE};

// Some globals that need to be defined

extern char **glob_argv;

extern int do_events_now;

#include "GlobalVars.h"

class SpecialRoomNames
{
   protected:
      StrGlobalVar idlersRoomName;
      StrGlobalVar entryRoomName;
      StrGlobalVar mainRoomName;
      StrGlobalVar jailRoomName;

   public:
      SpecialRoomNames() : idlersRoomName("idlers_room",ROOM_NAME_LEN,0,""),
                           entryRoomName("entry_room",ROOM_NAME_LEN,0,""),
                           mainRoomName("main_room",ROOM_NAME_LEN,0,""),
                           jailRoomName("jail_room",ROOM_NAME_LEN,0,"")
      {
      };
      
      StrGlobalVar *getIdlersRoomName()
      {
         return &idlersRoomName;
      }

      StrGlobalVar *getEntryRoomName()
      {
         return &entryRoomName;
      }
      StrGlobalVar *getMainRoomName()
      {
         return &mainRoomName;
      }

      StrGlobalVar *getJailRoomName()
      {
         return &jailRoomName;
      }

};


extern class SpecialRoomNames globalSpecialRoomNames;


extern LimitedIntGlobalVar heartbeat;
extern TimeTGlobalVar      resetAnnounce;
extern IntGlobalVar        rs_countdown;
extern IntGlobalVar        rs_which;
extern OnOffGlobalVar      color_on_def;
extern OnOffGlobalVar      system_logging;
extern OnOffGlobalVar      prompt_def;
extern YesNoGlobalVar      ignore_sigterm;
extern YesNoGlobalVar      ban_swearing;
extern YesNoGlobalVar      password_echo;
extern LimitedIntGlobalVar purgedays; 
extern LimitedIntGlobalVar max_clones;
extern LimitedIntGlobalVar max_users;
extern IntGlobalVar forwarding;
extern LevelGlobalVar wizport_level;
extern LevelGlobalVar gatecrash_level;
extern LevelOrNoneGlobalVar minlogin_level;
extern DayTimeGlobalVar mesg_check_time;
extern NewOldGlobalVar nuts_talk_style;
extern OnOffGlobalVar atmos_on;
extern YesNoGlobalVar allow_caps_in_name;
extern StrGlobalVar default_color;
extern int config_line;
extern int no_prompt;
extern int force_listen;
extern int curr_user_destructed;
extern UR_OBJECT curr_user;
extern int keepalive_interval;
extern int temp_user_count;
extern time_t boot_time;

#include <vector>
typedef std::vector<pod_string, pod_alloc< pod_string >::Type > StringsArray;

extern StringsArray mood_list;

extern int login_cnt;

/* extern char *sys_errlist[]; */

/* function prototypes */
int        afk(UR_OBJECT user, const char *inpstr);
int        afk_check(UR_OBJECT user);
int        afk_check_verbal(UR_OBJECT checker,UR_OBJECT target);
int        allclones(UR_OBJECT user);
int        bcast(UR_OBJECT user, const char *inpstr);
int        change_pass(UR_OBJECT user);
int        change_room_fix(UR_OBJECT user, int fix);
int        clearline(UR_OBJECT user);
int        cloak(UR_OBJECT user, char *inpstr);
int        clone_emote(UR_OBJECT user, const char *inpstr);
int        clone_hear(UR_OBJECT user);
int        clone_say(UR_OBJECT user, const char *inpstr);
int        clone_switch(UR_OBJECT user);
int        connect_user(UR_OBJECT user);
int        create_clone(UR_OBJECT user);
int        delete_user(UR_OBJECT user, int this_user);
int        destroy_clone(UR_OBJECT user);
int        destroy_user_clones(UR_OBJECT user,int silent);
int        disconnect_user(UR_OBJECT user);
int        editor(UR_OBJECT user, const char *inpstr);
int        enter_profile(UR_OBJECT user, int done_editing);
int        get_idle_mins(UR_OBJECT user);
int        get_level(const char *name);
RM_OBJECT  get_room(const char *name);
void       get_sound(UR_OBJECT user, const char *inpstr, char *type);
char*      get_stat(UR_OBJECT u);
char*      get_visible_color(UR_OBJECT user);
pod_string get_visible_name(UR_OBJECT user);
char       get_visible_prechar(UR_OBJECT user);
pod_string get_visible_sound(UR_OBJECT user, pod_string inpstr);
int*       get_wipe_parameters(UR_OBJECT user, int max) ;
char*      getLevelName(int which);
char*      getLevelShortName(int which);
int        has_room_access(UR_OBJECT user, RM_OBJECT rm);
int        kill_user(UR_OBJECT user);
int        logging(UR_OBJECT user);
int        look(UR_OBJECT user);
int        minlogin(UR_OBJECT user);
int        misc_ops(UR_OBJECT user, const char *inpstr);
int        myclones(UR_OBJECT user);
int        prompt(UR_OBJECT user);
int        quit(UR_OBJECT user);
int        reboot_com(UR_OBJECT user);
int        recent(UR_OBJECT user);
int        reset_access(RM_OBJECT rm);
int        room_filter(UR_OBJECT user);
int        room_ink(UR_OBJECT user);
int        rooms(UR_OBJECT user);
int        secret_room_vis(UR_OBJECT looker,UR_OBJECT looked_at );
int        set_room_access(UR_OBJECT user);
int        set_topic(UR_OBJECT user, const char *inpstr);
int        shutdown_com(UR_OBJECT user);
int        site(UR_OBJECT user);
int        suicide(UR_OBJECT user);
int        system_details(UR_OBJECT user);
int        talker_softboot_command(UR_OBJECT user);
int        timed(UR_OBJECT user);
int        toggle_color_on(UR_OBJECT user);
int        toggle_mode(UR_OBJECT user);
int        toggle_prompt(UR_OBJECT user);
int        unlink_checked(const char *filename, char *caller);
int        visibility(UR_OBJECT user, int vis);
void       write_seperator_line(UR_OBJECT user,char *string);


typedef  int disconnect_handler_func(UR_OBJECT);
void disconnect_handler_add( disconnect_handler_func *func );

#include <dirent.h>
#ifndef __FreeBSD__
int is_d_file(const struct dirent *node);
#else
int is_d_file(struct dirent *node);
#endif

/* end of function declarations */
#endif /* !PODNUTS_H */
