#include <ctype.h>
#include "general_headers.h"
#include "string_misc.h"
#include "clones.h"
#include "curse.h"
#include "StringLibrary.h"
#include "ignore.h"
#include "chitter.h"

int say_gen(UR_OBJECT user,  const char *inpstr)
{
   StringLibrary * stringLibrary = StringLibrary::getInstance();
   pod_string cursed;
   int old;
   pod_string sound;
   pod_string name;

   if(nuts_talk_style.get() == 1) old=FALSE;
   else                   old=TRUE;

   if (user->muzzled)                           write_user(user,"You are muzzled, you cannot speak.\n");
   else if (words.word_count<2 && user->command_mode) write_user(user,"Say what?\n");
   else
   {
      cursed = curse_rewrite(user,inpstr);
      name = get_visible_name(user);
      if(old)
      {
         sound = get_visible_sound( user, cursed );      
      }

      if(is_clone(user))
      {
         pod_stringstream outputStream;
         if(old)
         {
            outputStream << "Clone of " << user->name << " " << sound << "s: " << cursed << "\n";
         }
         else
         {
            outputStream << "[Clone of " << user->name << "] " << cursed << "\n";
         }
         write_room(user->room, (char*)outputStream.str().c_str() );
         record(user->room, (char*)outputStream.str().c_str() );
      }
      else if (ban_swearing.get() && contains_swearing(cursed)) write_user_crt(user,stringLibrary->makeString("noswearing").c_str());
      else
      {
         pod_stringstream outputStream;
         if(old)
         {
            write_userf(user,"%sYou %s: %s\n",get_visible_color(user),sound.c_str(),cursed.c_str());
            outputStream << get_visible_color(user) << name << " " << sound << "s: " << cursed << "\n";
         }
         else
         {
            write_userf(user,"%s[%s] %s\n",get_visible_color(user),user->name,cursed.c_str());
            outputStream << get_visible_color(user) << "[" << name << "] " << cursed << "\n";
         }
         write_room_except(user->room,(char*)outputStream.str().c_str(),user );
         record(user->room, (char*)outputStream.str().c_str() );
      }
   }

   return 0;
}

/*** Say something to a specific user. (public) ***/
/* Written by Vaghn 1998 */
int say_to(UR_OBJECT user, const char *inpstr)
{
   UR_OBJECT u;
   pod_string type;
   pod_string cursed;

   if (words.word_count<3) 
   {
      write_user(user,"Say what to whom?\n");
      return 0;
   }
   
   u = get_user_advanced( user, words.word[1], GUA_SHOW_ERRORS, GUA_ROOM, GUA_NOT_CLOAKED );
   
   if( u == 0 ) 
   {
      return 0;
   }   
   
   if (u==user)
   {
      write_user(user,"Are you trying to talk to yourself again?!\n");
   }
   else if (!afk_check_verbal(user,u))
   {
      type = get_visible_sound(user, inpstr);

      if (is_clone(user))
      {
         pod_stringstream outputStream;
         outputStream << "Clone of " << user->name << " " << type << " (to " << u->name << "): " << inpstr << "\n";
         write_room(user->room,(char*)outputStream.str().c_str() );
         record(user->room, (char*)outputStream.str().c_str() );
      }
      else
      {
         StrGlobalVar *color = (StrGlobalVar*)u->getVar("Color");
         inpstr=const_remove_first(inpstr);
         cursed = curse_rewrite(user,inpstr);
         write_userf(user,"%sYou %s (to ~RS%s%s~RS%s): %s\n",get_visible_color(user),type.c_str(),color->get().c_str(),u->name,get_visible_color(user),cursed.c_str());

         pod_stringstream outputStream;
         outputStream << get_visible_color(user) << get_visible_name(user) << " " << type << "s (to ~RS" << color->get() << u->name << "~RS" << get_visible_color(user) << "): " << cursed << "\n";
         write_room_except(user->room,(char*)outputStream.str().c_str(),user );
         record(user->room, (char*)outputStream.str().c_str() );
      }
   }
   
   return 0;
}

int wake(UR_OBJECT user, const char *inpstr)
{
   StringLibrary * stringLibrary = StringLibrary::getInstance();
   UR_OBJECT u;

   if (words.word_count<3)                write_user(user,"Usage: wake <user> <message>\n");
   else if (!(u=get_user(words.word[1]))) write_user_crt(user,stringLibrary->makeString("notloggedon").c_str());
   else if (u==user)                write_user(user,"Trying to wake yourself up is the eighth sign of madness.\n");
   else if (u->afk)                 write_user(user,"You cannot wake someone who is AFK.\n");
   else
   {
      inpstr=const_remove_first(inpstr);
      write_userf(u,"\07\n~BR*** ~OL~LIWAKE UP!!!~RS~BR ***\nFrom %s: %s\n\n",get_visible_name(user).c_str(),inpstr);
      write_userf(user,"Wake up call sent to %s.\n",u->name);
   }

   return 0;
}


/*** Shout something ***/
int shout(UR_OBJECT user, const char *inpstr)
{
   StringLibrary * stringLibrary = StringLibrary::getInstance();
   pod_string cursed;
   if (user->level == LEV_MIN)               write_userf(user,"~OLYou are in ~FR%s~FW...noone can hear you.\n",globalSpecialRoomNames.getJailRoomName()->get().c_str());
   else if ((words.word_count<2)||(!strcmp(inpstr,""))) write_user(user,"Shout what?\n");
   else if (ban_swearing.get() && contains_swearing(inpstr)) write_user_crt(user,stringLibrary->makeString("noswearing").c_str());
   else
   {
      cursed = curse_rewrite(user,inpstr);
      write_userf(user,"~OL%sYou shout: %s\n",get_visible_color(user),cursed.c_str());

      pod_stringstream outputStream;
      outputStream << "~OL" << get_visible_color(user) << get_visible_name(user) << " shouts: " << cursed << "\n";
      write_room_except(NULL,(char*)outputStream.str().c_str(),user );
      record_shout((char*)outputStream.str().c_str() );
   }
   return 0;
}

/*** Tell another user something ***/
int tell(UR_OBJECT user, const char *inpstr)
{
   pod_string cursed;
   UR_OBJECT u;
   pod_string type;

   if (user->level == LEV_MIN)                          write_userf(user,"~OLYou are in ~FR%s~FW....noone can here your words.\n",globalSpecialRoomNames.getJailRoomName()->get().c_str());
   else if ((words.word_count<3)||(!strcmp(words.word[2],""))) write_user(user,"Tell who what?\n");  
   else if (!(u=get_user_and_check(user,words.word[1]))) ;
   else if (u==user)                               write_user(user,"Talking to yourself is the first sign of madness.\n");
   else if (afk_check_verbal(user,u));
   else if (u->ignall && NO_OVERR(user,u)) 
   {
      if (u->malloc_start!=NULL) write_userf(user,"%s is using the editor at the moment.\n",u->name);
      else                       write_userf(user,"%s is ignoring everyone at the moment.\n",u->name);
   }
   else if ( GET_IGNORE_TELL(u)->get() && NO_OVERR(user,u) ) write_userf(user,"%s is ignoring tells at the moment.\n",u->name);
   else
   {
      type = get_visible_sound(user, inpstr);

      inpstr=const_remove_first(inpstr);
      cursed = curse_rewrite(user,inpstr);
   
      pod_stringstream outputStream;
      outputStream << "~OL" << get_visible_color(user) << ">> You " << type << " to " << u->name << ": " << cursed << " ~RS\n";
      write_user(user,(char*)outputStream.str().c_str() );
      record_tell(user,(char*)outputStream.str().c_str() );

      outputStream.str("");
      outputStream << "~OL" << get_visible_color(user) << ">> " << get_visible_name(user) << " " << type << "s to you: " << cursed << " ~RS\n";
      write_user(u,(char*)outputStream.str().c_str() );
      record_tell(u,(char*)outputStream.str().c_str() );
   }
   return 0;
}

/* Whisper to someone in the same room. */
/* Sort of a cross between STO and TELL. (private) */
/* Written by Vaghn 1998 */
int whisper(UR_OBJECT user, const char *inpstr)
{
   pod_string cursed;
   UR_OBJECT u;
   pod_string name;

   WriteRoomStream *roomStream = WriteRoomStream::getInstance();

   if (words.word_count<3) 
   {
      write_user(user,"Whisper what to who?\n");
      return 0;
   }
      
   u = get_user_advanced( user, words.word[1], GUA_SHOW_ERRORS, GUA_ROOM, GUA_NOT_CLOAKED );
   
   if( u == 0 ) 
   {
      return 0;
   }   
   
   if (u==user) write_user(user,"Why are you trying to whisper to yourself?\n");
   else if (afk_check_verbal(user,u));
   else if (u->ignall && NO_OVERR(user,u))
   {
      if (u->malloc_start!=NULL) write_userf(user,"%s is using the editor at the moment.\n",u->name);
      else                       write_userf(user,"%s is ignoring everyone at the moment.\n",u->name);
   }
   else if ( GET_IGNORE_TELL(u)->get() && NO_OVERR(user,u)) write_userf(user,"%s is ignoring tells and whispers at the moment.\n",u->name);
   else
   {
      inpstr=const_remove_first(inpstr);
      name = get_visible_name(user);

      cursed = curse_rewrite(user,inpstr);


      pod_stringstream outputStream;
      outputStream << "~OLYou whisper to " << u->name << ": " << cursed << "\n";
      write_user(user,(char*)outputStream.str().c_str() );
      record_tell(user,(char*)outputStream.str().c_str() );

      outputStream.str("");
      outputStream << "~OL" << name << " whispers to you: " << cursed << "\n";
      write_user(u,(char*)outputStream.str().c_str() );
      record_tell(u,(char*)outputStream.str().c_str() );

      *roomStream  << setRoom( user->room ) << addExcept( user ) << addExcept( u )
                  << "~OL" << name << " whispers something to " << u->name << "\n"
                  << pod_send ;
   }
   return 0;
}

/*** The function to allow WIZARDS to speak to all the other WIZARDS ***/
int wtell(UR_OBJECT user, const char *inpstr)
{
   char type[15];

   if (user->level == LEV_MIN)                          write_userf(user,"~OLYou are in ~FR%s~FW....noone can here your words.\n",globalSpecialRoomNames.getJailRoomName()->get().c_str());
   else if ((words.word_count<2)||(!strcmp(words.word[1],""))) write_user(user,"What do you want to say?\n");
   else
   {
      StrGlobalVar *color = (StrGlobalVar*)user->getVar("Color");
      get_sound(user, inpstr, type);

      pod_stringstream outputStream;
      outputStream << "~OL~FW[wtell ]~RS " << color->get() << user->name << " " << type << "s: " << inpstr << "\n";
      write_level(LEV_THR,1,(char*)outputStream.str().c_str(), user );
      wrecord((char*)outputStream.str().c_str() );

      write_userf(user,"~OL~FW[wtell ]~RS %sYou %s: %s\n",color->get().c_str(),type,inpstr);
   }
   return 0;
}

/*** This function allows WIZARDS to emote something over the WIZARD channel ***/
/* This function was written by Vaghn to replace the wizshout function which was
   causing too many system crashes (1999) */
int wemote(UR_OBJECT user, const char *inpstr)
{
   StringLibrary * stringLibrary = StringLibrary::getInstance();
   char append[10];

   if (user->level == LEV_MIN) write_userf(user,"~OLYou are in ~FR%s~FW....noone can here your words.\n",globalSpecialRoomNames.getJailRoomName()->get().c_str());
   else if ((words.word_count<2 && inpstr[1]<33)||(!strcmp(inpstr, ""))) write_user(user,"Emote what?\n");
   else if (ban_swearing.get() && contains_swearing(inpstr)) write_user_crt(user,stringLibrary->makeString("noswearing").c_str());
   else
   {
      StrGlobalVar *color = (StrGlobalVar*)user->getVar("Color");
      append[0]='\0';
      if ( (!strcmp(words.word[1],",")) || (!strcmp(words.word[1],"'s")) || (!strcmp(words.word[1],"'ll")))
      {
         inpstr=const_remove_first(inpstr);
         strcpy(append,words.word[1]);
      }
      pod_stringstream outputStream;
      outputStream << "~OL~FW[wemote]~RS " << color->get() << user->name << append << " " << inpstr << "\n";
      write_level(LEV_THR,1,(char*)outputStream.str().c_str(), NULL );
      wrecord((char*)outputStream.str().c_str() );
   }
   return 0;
}

/*** Say something to a specific user. (WIZARD) ***/
/* Written by Vaghn 1999. */
int wsto(UR_OBJECT user, const char *inpstr)
{
   UR_OBJECT u;
   char type[15];

   if (user->level == LEV_MIN)       write_userf(user,"~OLYou are in ~FR%s~FW....noone can here your words.\n",globalSpecialRoomNames.getJailRoomName()->get().c_str());
   else if (words.word_count<3)  write_user(user,"Say what to whom?\n");
   else if ((u=get_user_and_check(user,words.word[1])))
   {
      if (u->level < LEV_THR) write_userf(user, "That user is not at wizard level. They can not hear you!\n");
      else if (u==user)        write_userf(user,"Imagine a being of your level trying to talk to %sself! Hmph! :)\n",get_gender(user,"him"));
      else if (!afk_check_verbal(user,u))
      {
         if (u->ignall && NO_OVERR(user,u)) 
         {
            if (u->malloc_start!=NULL) write_userf(user,"%s is using the editor at the moment.\n",u->name);
            else write_userf(user,"%s is ignoring everyone at the moment.\n",u->name);
         }
         else if( GET_IGNORE_WIZ(u)->get() ) write_userf(user,"%s is ignoring wiztalk at the moment.\n",u->name);
         else
         {
            get_sound(user, inpstr, type);
            inpstr=const_remove_first(inpstr);
            StrGlobalVar *u_color = (StrGlobalVar*)u->getVar("Color");
            StrGlobalVar *user_color = (StrGlobalVar*)user->getVar("Color");

            pod_stringstream outputStream;
            outputStream << "~OL~FW[wsto  ]~RS " << user_color->get() << user->name << " " << type << "s (to ~RS" << u_color->get() << u->name << "~RS" << user_color->get() << "): " << inpstr << "\n";
            write_level(LEV_THR,1,(char*)outputStream.str().c_str(), user );
            wrecord((char*)outputStream.str().c_str() );

            write_userf(user,"~OL~FW[wsto  ]~RS %sYou %s (to ~RS%s%s~RS%s): %s\n",user_color->get().c_str(),type,u_color->get().c_str(),u->name,user_color->get().c_str(),inpstr);
         }
      }
   }
   return 0;
}

/*** Emote something ***/
int emote(UR_OBJECT user, const char *inpstr)
{
   StringLibrary * stringLibrary = StringLibrary::getInstance();
   char append[10];
   pod_string cursed;

   if (user->level == LEV_MIN) write_userf(user,"~OLYou are in ~FR%s~FW....noone can here your words.\n",globalSpecialRoomNames.getJailRoomName()->get().c_str());
   else if ((words.word_count<2 && inpstr[1]<33)||(!strcmp(inpstr, ""))) write_user(user,"Emote what?\n");  
   else if (ban_swearing.get() && contains_swearing(inpstr)) write_user_crt(user,stringLibrary->makeString("noswearing").c_str());
   else if (is_clone(user)) 
   {
      cursed = curse_rewrite(user,inpstr);

      pod_stringstream outputStream;
      outputStream << "Clone of " << user->name << " " << cursed << "\n";
      write_room(user->room,(char*)outputStream.str().c_str() );
      record( user->room, (char*)outputStream.str().c_str() );
   }
   else
   {
      cursed = curse_rewrite(user,inpstr);

      {
         append[0]='\0';
         if ( (!strcmp(words.word[1],",")) || (!strcmp(words.word[1],"'s")) || (!strcmp(words.word[1],"'ll")))
         {
            cursed=remove_first(cursed);
            strcpy(append,words.word[1]);
         }
         pod_stringstream outputStream;
         outputStream << get_visible_color(user) << get_visible_name(user) << append << " " << cursed << "\n";
         write_room(user->room,(char*)outputStream.str().c_str() );
         record( user->room, (char*)outputStream.str().c_str() );

      }
   }
   return 0;
}

/*** Do a shout emote ***/
int semote(UR_OBJECT user, const char *inpstr)
{
   pod_string cursed;
   char append[10];

   if (user->level == LEV_MIN) write_userf(user,"~OLYou are in ~FR%s~FW....noone can here your words.\n",globalSpecialRoomNames.getJailRoomName()->get().c_str());
   else if ((words.word_count<2 && inpstr[1]<33)||(!strcmp(inpstr, ""))) write_user(user,"Shout emote what?\n");
   else
   {
         cursed = curse_rewrite(user,inpstr);
         append[0]='\0';
         if ( (!strcmp(words.word[1],",")) || (!strcmp(words.word[1],"'s")) || (!strcmp(words.word[1],"'ll")) )
         {
            cursed=remove_first(cursed);
            strcpy(append,words.word[1]);
    }
      pod_stringstream outputStream;
      outputStream << "~OL" << get_visible_color(user) << "!!~RS" << get_visible_color(user) << " " << get_visible_name(user) << append << " " << cursed << "\n";
      write_room(NULL,(char*)outputStream.str().c_str() );
      record_shout( (char*)outputStream.str().c_str() );
   }
   return 0;
}

/*** Do a private emote ***/
int pemote(UR_OBJECT user, const char *inpstr)
{
   char append[10];
   UR_OBJECT u;

   if (user->level == LEV_MIN)  write_userf(user,"~OLYou are in ~FR%s~FW...noone can hear you.\n",globalSpecialRoomNames.getJailRoomName()->get().c_str());
   else if (words.word_count<3)  write_user (user,"Private emote what?\n");
   else
   {
      words.word[1][0]=toupper(words.word[1][0]);
      if((u=get_user_and_check(user,words.word[1])))
      {
         if (!strcmp(u->name,user->name)) write_user(user,"Pemoting to yourself is the second sign of madness.\n");
         else if (afk_check_verbal(user,u));
         else if (u->ignall && NO_OVERR(user,u))
         {
            if (u->malloc_start!=NULL) write_userf(user,"%s is using the editor at the moment.\n",u->name);
            else write_userf(user,"%s is ignoring everyone at the moment.\n",u->name);
         }
         else if ( GET_IGNORE_TELL(u)->get() && NO_OVERR(user,u) )  write_userf(user,"%s is ignoring private emotes at the moment.\n",u->name);
         else
         {
            inpstr=const_remove_first(inpstr);
            append[0]='\0';
            if ( (!strcmp(words.word[2],",")) || (!strcmp(words.word[2],"'s")) || (!strcmp(words.word[2],"'ll")) )
            {
               inpstr=const_remove_first(inpstr);
               strcpy(append,words.word[2]);
       }

         pod_stringstream outputStream;
         outputStream << "~OL" << get_visible_color(user) << ">> To " << u->name << ": " << get_visible_name(user) << append << " " << inpstr << " ~RS\n";
         write_user(user, (char*)outputStream.str().c_str() );
         record_tell(user, (char*)outputStream.str().c_str() );

         outputStream.str("");
         outputStream << "~OL" << get_visible_color(user) << ">> " << get_visible_name(user) << append << " " << inpstr << " ~RS\n";
         write_user(u, (char*)outputStream.str().c_str() );
         record_tell(u, (char*)outputStream.str().c_str() );

         }
      }
   }
   return 0;
}

/*** Echo something to screen ***/
int echo(UR_OBJECT user, const char *inpstr)
{
   UR_OBJECT u;

   if (user->level == LEV_MIN)   write_userf(user,"~OLYou are in ~FR%s~FW...noone can hear you.\n",globalSpecialRoomNames.getJailRoomName()->get().c_str());
   else if (words.word_count<2)   write_user (user,"Echo what?\n");
   else if (!strcmp(words.word[1],"")) write_user (user,"Echo what?\n");
   else
   {
      for(u=user_first;u!=NULL;u=u->next) 
      {
         if (  u->login
            || (u->room!=user->room && user->room!=NULL)
            || (u->ignall && !force_listen)
            || u==user) continue;
         
         if (is_clone(u)) 
         {
            if ( (u->clone_hear==CLONE_HEAR_NOTHING) || 
                 (u->owner->ignall) ||
                 (user->room!=u->room) ||
                 ( (u->clone_hear==CLONE_HEAR_SWEARS) && 
                   !contains_swearing(inpstr) )          ) continue;
            write_userf(u->owner,"~FT[ %s ]:~RS %s",u->room->name,inpstr);
         }
         else
         {
            if (u->level>=user->level)
            {
               write_userf(u,"~OL~FR(~FW%s~OL~FR)~RS %s\n",user->name,inpstr);
            }
            else write_user_crt(u,inpstr);
         }
      }

      pod_stringstream outputStream;
      outputStream << inpstr << "\n";
      write_user(user, (char*)outputStream.str().c_str() );
      record(user->room, (char*)outputStream.str().c_str() );
   }
   return 0;
}

/*** Show something to user ***/
int show(UR_OBJECT user, const char *inpstr)
{
   StringLibrary * stringLibrary = StringLibrary::getInstance();
   UR_OBJECT u;

   if (words.word_count<3)           write_user(user,"Show who what?\n");
   else if (!(u=get_user(words.word[1]))) write_user_crt(user,stringLibrary->makeString("notloggedon").c_str());
   else if (u==user)                write_user(user,"Trying to show yourself is the eighth sign of madness.\n");
   else
   {
      inpstr=const_remove_first(inpstr);
      write_userf(u,"~OL~FB-->Type:~RS %s\n",inpstr);
      write_userf(user,"~OL~FB-->You Show: %s <to:%s>\n",inpstr,u->name);
   }
   return 0;
}

