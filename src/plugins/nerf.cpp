#include "../levels.h"
#include "../socket_funcs.h"
#include "../GlobalVars.h"
#include "../Room.h"
#include "../wordfind.h"
#include "../user_objects.h"
#include "../speech_funcs.h"
#include "../help.h"
#include "../StringLibrary.h"
#include "../cmd_main.h"

#include "../podnuts.h"
#include "../gender.h"
#include "../string_misc.h"

#define NERFROOM   "kelp_forest"

#define userGetIntVar(user,name)   \
        ((IntGlobalVar*)user->getVar(name))

#define GET_NERF_WIN(user)       \
        ((IntGlobalVar*)user->getVar("nerf_win"))
#define GET_NERF_LOSE(user)       \
        ((IntGlobalVar*)user->getVar("nerf_lose"))
#define GET_NERF_LIFE(user)       \
        ((IntGlobalVar*)user->getVar("nerflife"))
#define GET_NERF_CHARGE(user)       \
        ((IntGlobalVar*)user->getVar("nerfcharge"))

int nerf  (UR_OBJECT user,char *inpstr);
int charge(UR_OBJECT user,char *inpstr);

extern "C" void plugin_init()
{
   StringLibrary::getInstance()->addFile("datafiles/nerf.xml");

   user_var_add_cust("nerf_win",   new IntObjectCreator(0), USR_SAVE_ALWS);
   user_var_add_cust("nerf_lose",  new IntObjectCreator(0), USR_SAVE_ALWS);
   user_var_add_cust("nerflife",   new IntObjectCreator(10),USR_SAVE_ALWS);
   user_var_add_cust("nerfcharge", new IntObjectCreator(0), USR_SAVE_ALWS);

   cmd_add("charge", LEV_ONE, FIG, &charge);
   cmd_add("nerf",   LEV_ONE, FIG, &nerf);
}

int nerf(UR_OBJECT user,char *inpstr)
{
   struct words_struct* words_ptr = &words;
   UR_OBJECT u;
   WriteUserStream *userStream = WriteUserStream::getInstance();
   WriteRoomStream *roomStream = WriteRoomStream::getInstance();
   StringLibrary * stringLibrary = StringLibrary::getInstance();

   if (words_ptr->word_count<2)
   {
      *userStream << addUser( user ) << stringLibrary->makeString("nerf_usage") << '\n' << pod_send;
   }
   else if (!strstr(user->room->name,NERFROOM))
   {
      *userStream << addUser( user ) << stringLibrary->makeString("nerf_bad_rm") << '\n' << pod_send;
   }
   else
   {
      if (!(u=get_user(words_ptr->word[1])))
      {
         *userStream << addUser( user ) << stringLibrary->makeString("notloggedon") << '\n' << pod_send;
      }
      else if (u==user)
      {
         *userStream << addUser( user ) << stringLibrary->makeString("nerf_self") << '\n' << pod_send;
      }
      else if (!(user->room==u->room))
      {
         *userStream << addUser( user ) << stringLibrary->makeString("nerf_no_tgt") << '\n' << pod_send;
      }
      else if (GET_NERF_CHARGE(user)->get() < 1)
      {
         *userStream << addUser( user ) << stringLibrary->makeString("nerf_no_ammo") << '\n' << pod_send;
      }
      else if (GET_NERF_CHARGE(user)->get() > 0)
      {
         GET_NERF_CHARGE(user)->decrease(1);
         
         stringLibrary->addTextTag( "name", user->name );
         stringLibrary->addTextTag( "target", u->name );
         
         if ((rand() % 10) >= 5)
         {
            GET_NERF_LIFE(u)->decrease(1);
            if( GET_NERF_LIFE(u)->get() > 0 )
            {
               *userStream << addUser( user ) << stringLibrary->makeString("nerf_hit") << '\n' << pod_send;
               *roomStream << setRoom( u->room ) << addExcept( user )
                           << stringLibrary->makeString("nerf_is_hit") << '\n' << pod_send;
            }
            else
            {
               *userStream << addUser( u ) << stringLibrary->makeString("nerf_die") << '\n' << pod_send;
               *roomStream << setRoom( user->room ) << addExcept( u )
                           << stringLibrary->makeString("nerf_is_die") << '\n' << pod_send;

               GET_NERF_WIN(user)->increase(1);
               GET_NERF_LOSE(u)->increase(1);
               GET_NERF_LIFE(u)->set(10);
               disconnect_user(u);
            }
         }
         else 
         {
            *userStream << addUser( user ) << stringLibrary->makeString("nerf_mis") << '\n' << pod_send;
            *roomStream << setRoom( u->room ) << addExcept( user )
                        << stringLibrary->makeString("nerf_is_mis") << '\n' << pod_send;
         }
      }
   }
   return 0;
}

/** charge **/
int charge(UR_OBJECT user,char *inpstr)
{
   WriteUserStream *userStream = WriteUserStream::getInstance();
   WriteRoomStream *roomStream = WriteRoomStream::getInstance();
   StringLibrary * stringLibrary = StringLibrary::getInstance();

   stringLibrary->addTextTag( "name", user->name );
   stringLibrary->addTextTag( "his", get_gender(user,"his") );
   stringLibrary->addTextTag( "nerfroom", NERFROOM );   
      
   if (!(strstr(user->room->name,NERFROOM)))
   {
      *userStream << addUser( user ) << stringLibrary->makeString("chrg_bad_rm") << '\n' << pod_send;
      return 0;
   }
   *userStream << addUser( user ) << stringLibrary->makeString("chrg_chrg") << '\n' << pod_send;
   *roomStream << setRoom( user->room ) << addExcept( user )
               << stringLibrary->makeString("chrg_chrg_rm") << '\n' << pod_send;

   GET_NERF_CHARGE(user)->set(5);
   return 0;
}
