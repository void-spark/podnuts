#include "../levels.h"
#include "../socket_funcs.h"
#include "../GlobalVars.h"
#include "../Room.h"
#include "../wordfind.h"
#include "../user_objects.h"
#include "../speech_funcs.h"
#include "../help.h"
#include "../cmd_main.h"
#include "../ignore.h"

#include "../podnuts.h"
#include "../string_misc.h"

int        ecco(UR_OBJECT user, char *inpstr);
int        eecco(UR_OBJECT user, char *inpstr);
int        ecco_end(UR_OBJECT user);
int        ecco_kill(UR_OBJECT user,int silent);
int        ecco_ping(UR_OBJECT user);
int        ecco_disconnect(UR_OBJECT user)
{
   return ecco_kill(user,FALSE);
}

extern "C" void plugin_init()
{
   user_var_add_cust( "ecco_partner",  new UserObjectCreator(), USR_SAVE_SOFT );
   user_var_add_cust( "ecco_accepted", new IntObjectCreator(0), USR_SAVE_SOFT );

   cmd_add("ping",      LEV_TWO, SPCH, &ecco_ping);
   cmd_add("ecco",      LEV_TWO, SPCH, &ecco);
   cmd_add("eecco",     LEV_TWO, SPCH, &eecco);
   cmd_add("end",       LEV_TWO, SPCH, &ecco_end);

   disconnect_handler_add( &ecco_disconnect );
}

/*** Open an 'ecco' link with another user, by crandonkphin ***/
int ecco_ping(UR_OBJECT user)
{
   UR_OBJECT u;
   pod_string name;
   struct words_struct* words_ptr = &words;

   UserGlobalVar *user_ecco_partner  = (UserGlobalVar*)user->getVar("ecco_partner");
   IntGlobalVar  *user_ecco_accepted = (IntGlobalVar*)user->getVar("ecco_accepted");

   if ( user_ecco_partner->get() )  write_userf(user,"You are already eccoing with %s.\n",user_ecco_partner->get()->name);
   else if (user->level == LEV_MIN)   write_userf(user,"~OLYou are in ~FR%s~FW....noone can hear your ping.\n",globalSpecialRoomNames.getJailRoomName()->get().c_str());
   else if (words_ptr->word_count<2)   write_user(user,"Ping who ?\n");
   else if(!(u=get_user_and_check(user,words_ptr->word[1])));
   else if (u==user)        write_user(user,"You can't ping yourself silly! :)\n");
   else if (u->level < LEV_TWO) write_user(user,"That user is of too low level to reply to your ping.\n");
   else if (afk_check_verbal(user,u));
   else if (u->ignall && NO_OVERR(user,u))
   {
      if (u->malloc_start!=NULL) write_userf(user,"%s is using the editor at the moment.\n",u->name);
      else write_userf(user,"%s is ignoring everyone at the moment.\n",u->name);
   }
   else if (GET_IGNORE_TELL(u)->get() && NO_OVERR(user,u)) write_userf(user,"%s is ignoring tells at the moment.\n",u->name);
   else
   {
      UserGlobalVar *u_ecco_partner     = (UserGlobalVar*)u->getVar("ecco_partner");
      IntGlobalVar *u_ecco_accepted     = (IntGlobalVar*)u->getVar("ecco_accepted");
      name=get_visible_name(user);
      if ( u_ecco_partner->get() )
      {
         if(u_ecco_accepted->get())
         {
            write_userf(u,"%s has tried to ping you.\n",name.c_str());
            write_userf(user,"%s is already eccoing somephin else.\n",u->name);
            return 0;
         }
         else
         {
            write_user(u_ecco_partner->get(),"Your ping has been reflected by other crosssquawk, try again later.\n");
            UserGlobalVar *u_ecco_partner_partner     = (UserGlobalVar*)u_ecco_partner->get()->getVar("ecco_partner");
            IntGlobalVar *u_ecco_partner_accepted     = (IntGlobalVar*)u_ecco_partner->get()->getVar("ecco_accepted");
            u_ecco_partner_partner->set(0);
            u_ecco_partner_accepted->set(0);
         }
      }

      pod_stringstream outputStream;
      outputStream << "~OL" << get_visible_color(user) << "You ping " << u->name << "~RS\n";
      write_user(user,(char*)outputStream.str().c_str() );
      record_tell(user, (char*)outputStream.str().c_str() );

      outputStream.str("");
      outputStream << "~OL" << get_visible_color(user) << "A ping from " << name << " passes through your body {type .ecco (message) to reply}~RS\n";
      write_user(u,(char*)outputStream.str().c_str() );
      record_tell(u, (char*)outputStream.str().c_str() );

      u_ecco_partner->set(user);
      user_ecco_partner->set(u);
      user_ecco_accepted->set(1);
   }
   return 0;
}

/*** Close an 'ecco' link with another user, by crandonkphin ***/
int ecco_end(UR_OBJECT user)
{
   UserGlobalVar *user_ecco_partner  = (UserGlobalVar*)user->getVar("ecco_partner");

   if ( !user_ecco_partner->get() ) write_userf(user,"You are not eccoing with anyone.\n");
   else ecco_kill(user,FALSE);

   return 0;
}

/*** Kill any 'ecco' link with another user, by crandonkphin ***/
int ecco_kill(UR_OBJECT user,int silent)
{
   UR_OBJECT u;
   pod_string name;
   pod_string ecco_to_name;

   UserGlobalVar *user_ecco_partner  = (UserGlobalVar*)user->getVar("ecco_partner");
   IntGlobalVar *user_ecco_accepted  = (IntGlobalVar*)user->getVar("ecco_accepted");

   if (!user_ecco_partner->get()) return 0;

   u = user_ecco_partner->get();
   UserGlobalVar *u_ecco_partner     = (UserGlobalVar*)u->getVar("ecco_partner");
   IntGlobalVar *u_ecco_accepted     = (IntGlobalVar*)u->getVar("ecco_accepted");

   name=get_visible_name(user);
   ecco_to_name=get_visible_name(u);

   if(!silent)
   {
      pod_stringstream outputStream;
      if(u_ecco_accepted->get())
      {
         outputStream << "~OL" << get_visible_color(user) << "You stop eccoing with " << ecco_to_name << "~RS\n";
         write_userf(user, (char*)outputStream.str().c_str() );
         record_tell(user, (char*)outputStream.str().c_str() );
         write_userf(u,"~OL%s%s Stops eccoing with you~RS\n",get_visible_color(user),name.c_str());
      }
      else
      {
         outputStream << "~OL" << get_visible_color(user) << "You decide you don't want to ecco with " << ecco_to_name << " afterall~RS\n";
         write_userf(user, (char*)outputStream.str().c_str() );
         record_tell(user, (char*)outputStream.str().c_str() );
         write_userf(u,"~OL%s%s decides he doesn't want to ecco with you afterall~RS\n",get_visible_color(user),name.c_str());
      }
   }
   u_ecco_partner->set(0);
   user_ecco_partner->set(0);
   u_ecco_accepted->set(0);
   user_ecco_accepted->set(0);

   return 0;
}

/*** Tell 'ecco linked' user something (crandonkphin) ***/
int ecco(UR_OBJECT user, char *inpstr)
{
   pod_string type;
   UR_OBJECT u;
   struct words_struct* words_ptr = &words;

   UserGlobalVar *user_ecco_partner  = (UserGlobalVar*)user->getVar("ecco_partner");
   IntGlobalVar *user_ecco_accepted  = (IntGlobalVar*)user->getVar("ecco_accepted");

   if (!user_ecco_partner->get())                    write_userf(user,"You are not eccoing with anyphin.\n");
   else
   {
      IntGlobalVar *user_ecco_partner_accepted    = (IntGlobalVar*)user_ecco_partner->get()->getVar("ecco_accepted");
      if( !user_ecco_partner_accepted->get() ) write_userf(user,"%s has not accepted your ping yet.\n",user_ecco_partner->get()->name);
      else
      {
         u = user_ecco_partner->get();
         if (user->level == LEV_MIN) write_userf(user,"~OLYou are in ~FR%s~FW....noone can here your words.\n",globalSpecialRoomNames.getJailRoomName()->get().c_str());
         else if (words_ptr->word_count<2) write_user(user,"Ecco what?\n");
         else if (!afk_check_verbal(user,u))
         {
            if (u->ignall && NO_OVERR(user,u))
            {
               if (u->malloc_start!=NULL) write_userf(user,"%s is using the editor at the moment.\n",u->name);
               else                       write_userf(user,"%s is ignoring everyone at the moment.\n",u->name);
            }
            else if (GET_IGNORE_TELL(u)->get() && NO_OVERR(user,u))
            {
               write_userf(user,"%s is ignoring tells at the moment.\n",u->name);
            }
            else
            {
               type = get_visible_sound(user, inpstr);

               pod_stringstream outputStream;
               outputStream << "~OL" << get_visible_color(user) << "[Ecco] You " << type << " to " << get_visible_name(u) << ": " << inpstr << " ~RS\n";
               write_user(user,(char*)outputStream.str().c_str() );
               record_tell(user, (char*)outputStream.str().c_str() );

               outputStream.str("");
               outputStream << "~OL" << get_visible_color(user) << "[Ecco] " << get_visible_name(user) << " " << type << "s: " << inpstr << " ~RS\n";
               write_user(u,(char*)outputStream.str().c_str() );
               record_tell(u, (char*)outputStream.str().c_str() );

               if(!user_ecco_accepted->get()) user_ecco_accepted->set(1);
            }
         }
      }
   }
   return 0;
}

/*** Pemote 'ecco linked' user something (crandonkphin) ***/
int eecco(UR_OBJECT user, char *inpstr)
{
   UR_OBJECT u;
   struct words_struct* words_ptr = &words;

   UserGlobalVar *user_ecco_partner  = (UserGlobalVar*)user->getVar("ecco_partner");
   IntGlobalVar *user_ecco_accepted  = (IntGlobalVar*)user->getVar("ecco_accepted");

   if (!user_ecco_partner->get())                    write_userf(user,"You are not eccoing with anyphin.\n");
   else
   {
      IntGlobalVar *user_ecco_partner_accepted    = (IntGlobalVar*)user_ecco_partner->get()->getVar("ecco_accepted");
      if(!user_ecco_partner_accepted->get()) write_userf(user,"%s has not accepted your ping yet.\n",user_ecco_partner->get()->name);
      else
      {
         u = user_ecco_partner->get();
         if (user->level == LEV_MIN)  write_userf(user,"~OLYou are in ~FR%s~FW....noone can here your words.\n",globalSpecialRoomNames.getJailRoomName()->get().c_str());
         else if (words_ptr->word_count<2)      write_user(user,"Ecco what?\n");
         else if (!afk_check_verbal(user,u))
         {
            if (u->ignall && NO_OVERR(user,u))
            {
               if (u->malloc_start!=NULL) write_userf(user,"%s is using the editor at the moment.\n",u->name);
               else                       write_userf(user,"%s is ignoring everyone at the moment.\n",u->name);
            }
            else if (GET_IGNORE_TELL(u)->get() && NO_OVERR(user,u))
            {
               write_userf(user,"%s is ignoring tells at the moment.\n",u->name);
            }
            else
            {
               pod_stringstream outputStream;
               outputStream << "~OL" << get_visible_color(user) << "[Ecco] To " << get_visible_name(u) << ": " << get_visible_name(user) << " " << inpstr << "\n";
               write_user(user,(char*)outputStream.str().c_str() );
               record_tell(user, (char*)outputStream.str().c_str() );

               outputStream.str("");
               outputStream << "~OL" << get_visible_color(user) << "[Ecco] " << get_visible_name(user) << " " << inpstr << "\n";
               write_user(u,(char*)outputStream.str().c_str() );
               record_tell(u, (char*)outputStream.str().c_str() );

               if(!user_ecco_accepted->get())
               {
                  user_ecco_accepted->set(1);
               }
            }
         }
      }
   }

   return 0;
}


