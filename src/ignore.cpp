#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "general_headers.h"
#include "file_io.h"
#include "StringLibrary.h"
#include "string_misc.h"
#include "cmd_main.h"
#include "xalloc.h"
#include "ignore.h"

#warning instead of vars, each set to true or fals(ignore or not), a list of the ignored vals \
       then let commands register themself under a certain val

CommandSet picCmdsSet;

#define userGetIntVar(user,name)   \
        ((IntGlobalVar*)user->getVar(name))

/*** Switch ignoring all/shout/tells on and off ***/
int toggle_ignore_help(UR_OBJECT user)
{
   write_userf(user,"Usage: ignore <all/tell/shout/pictures/atmos/system%s>\n",(user->level >= LEV_THR) ? "/wtell" : "");
   write_userf(user," all      = %s\n",user->ignall                  ? "on" : "off");
   write_userf(user," tell     = %s\n",GET_IGNORE_TELL(user)->get()  ? "on" : "off");
   write_userf(user," shout    = %s\n",GET_IGNORE_SHOUT(user)->get() ? "on" : "off");
   write_userf(user," pictures = %s\n",GET_IGNORE_PIC(user)->get()   ? "on" : "off");
   write_userf(user," atmos    = %s\n",GET_IGNORE_ATMOS(user)->get() ? "on" : "off");
   write_userf(user," system   = %s\n",GET_IGNORE_SYS(user)->get()   ? "on" : "off");
if(user->level >= LEV_THR)
   write_userf(user," wtell    = %s\n",GET_IGNORE_WIZ(user)->get()   ? "on" : "off");
   
   return 0;
}

void ignore_init()
{
   StringLibrary::getInstance()->addFile("datafiles/ignore_strs.xml");
   
   user_var_add_cust("ignpic",   new IntObjectCreator(0),USR_SAVE_SOFT);
   user_var_add_cust("ignatmos", new IntObjectCreator(0),USR_SAVE_SOFT);
   user_var_add_cust("ignwiz",   new IntObjectCreator(0),USR_SAVE_SOFT);
   user_var_add_cust("ignsys",   new IntObjectCreator(0),USR_SAVE_SOFT);
   user_var_add_cust("ignshout", new IntObjectCreator(0),USR_SAVE_SOFT);
   user_var_add_cust("igntell",  new IntObjectCreator(0),USR_SAVE_SOFT);

   picCmdsSet.insert(get_cmd("picture"));
   picCmdsSet.insert(get_cmd("banner"));
}

int ign_tog(UR_OBJECT user, char *ign_var, char *ignore_what, char *ign_str, char *unign_str)
{
   StringLibrary * stringLibrary = StringLibrary::getInstance();
   userGetIntVar(user,ign_var)->set( !userGetIntVar(user,ign_var)->get() );
   write_userf(user,"You are %s ignoring %s.\n",userGetIntVar(user,ign_var)->get() ? "now" : "no longer",ignore_what);
   write_room_exceptf(user->room,user,"%s %s\n",user->name,stringLibrary->makeString(userGetIntVar(user,ign_var)->get() ? ign_str : unign_str).c_str());

   return 0;
}

int toggle_ignore(UR_OBJECT user)
{   
   StringLibrary * stringLibrary = StringLibrary::getInstance();
   
   if (words.word_count != 2) toggle_ignore_help(user);
   else if(!strcmp(words.word[1],"atmos"))
      ign_tog(user,"ignatmos", "atmospherics",                  "ign_atmos", "unign_atmos" );
   else if(!strcmp(words.word[1],"pictures"))
      ign_tog(user,"ignpic",   "pictures and private pictures", "ign_pics",  "unign_pics"  );
   else if(!strcmp(words.word[1],"system"))
      ign_tog(user,"ignsys",   "system msgs",                   "ign_sys",   "unign_sys"   );
   else if(!strcmp(words.word[1],"shout"))
      ign_tog(user,"ignshout", "shouts and shout emotes",       "ign_shout", "unign_shout" );
   else if(!strcmp(words.word[1],"tell"))
      ign_tog(user,"igntell",  "tells and private emotes",      "ign_tell",  "unign_tell"  );
   else if(!strcmp(words.word[1],"wtell") && (user->level >= LEV_THR))
   {
      GET_IGNORE_WIZ(user)->set( !GET_IGNORE_WIZ(user)->get() );
      write_userf(user,"You are %s ignoring wizard tells.\n",GET_IGNORE_WIZ(user)->get() ? "now" : "no longer");
      write_levelf(LEV_THR,1,user,"~OL~FW[wsys ]~RS %s %s\n",user->name,stringLibrary->makeString(GET_IGNORE_WIZ(user)->get() ? "ign_wtell" : "unign_wtell").c_str());
/*      write_room_exceptf(user->room,user,"%s %s\n",user->name,stringLibrary->makeString(user_get_int(user,"ignwiz") ? "ign_wtell" : "unign_wtell"));*/
   }
   else if(!strcmp(words.word[1],"all"))
   {
      user->ignall = !user->ignall;
      if(user->ignall) write_user(user,"You are now ignoring everyone.\n");
      else             write_user(user,"You will now hear everyone again.\n");
      write_room_exceptf(user->room,user,"%s %s\n",user->name,stringLibrary->makeString(user->ignall ? "ign_all" : "unign_all").c_str());
   }
   else toggle_ignore_help(user);
   
   return 0;
}

char* ignore_stat(UR_OBJECT user)
{
   static char igntext[10];
   if ( user->ignall || 
          ( GET_IGNORE_SHOUT(user)->get() &&
            GET_IGNORE_TELL(user)->get()  ) ) strcpy(igntext,"ALL");
   else if( GET_IGNORE_SHOUT(user)->get() )   strcpy(igntext,"SHOUTS");
   else if( GET_IGNORE_TELL(user)->get()  )   strcpy(igntext,"TELLS");
   else                                       strcpy(igntext,"NO");

   return igntext;
}

