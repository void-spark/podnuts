#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "general_headers.h"
#include "string_misc.h"
#include "tandem.h"
#include "curse.h"
#include "StringLibrary.h"
#include "move.h"

/*** Called by go() and move() ***/
int move_user(UR_OBJECT user, RM_OBJECT rm, int teleport)
{
   StringLibrary * stringLibrary = StringLibrary::getInstance();
   RM_OBJECT old_room;
   struct tandem_list followers;
   followers.follow_count=0;
   
   old_room=user->room;

   if ( teleport!=GO_MODE_GIANT_FLIPPER && 
        teleport!=GO_MODE_THROWN )              /* !forced */
   { 
      if(curse_is_bound(user)) return 0;
      else if( !has_room_access(user,rm) && 
                user->follow_mode != FOLLOW_MODE_FOLLOWING ) 
      {
         write_user(user,"That room is currently private, you cannot enter.\n");  
         return 0;
      }
   }

   if(teleport==GO_MODE_NORMAL_GO  || teleport==GO_MODE_MAGICAL_VORTEX) /* self initiated */
   {
      if (user->follow_mode == FOLLOW_MODE_FOLLOWING)
      {
         write_user(user,"You are unable to manouver on your own while tandem swimming. (type .break to stop).\n");               
         return 0;
      }
      tandem_get_followers(user ,&followers);
   }
   
   /* Reset invite room if in it */
   if (user->invite_room==rm) user->invite_room=NULL;
   
   if (!(user->cloaked || teleport==GO_MODE_SILENT) || followers.follow_count) 
   {
      if (teleport==GO_MODE_GIANT_FLIPPER) /* forced move */
      {
         follow_kill(user,FALSE);
      
         write_userf(user,"\n~FT~OLA giant flipper grabs you and pulls you into a magical blue vortex!\n");
         write_room_exceptf(old_room,user,"~FT~OLA giant flipper grabs %s who is pulled into a magical blue vortex!\n",get_visible_name(user).c_str());
         write_roomf(rm,"~FT~OL%s falls out of a magical blue vortex!\n",get_visible_name(user).c_str());
      }
      else if (teleport==GO_MODE_THROWN) /* forced move */
      {
         follow_kill(user,FALSE);
        
         write_userf(user,"\n~FT~OLYou have been ~FBTHROWN~FT out of the %s\n",old_room->name);
         write_room_exceptf(old_room,user,"~FT~OL%s is ~FBTHROWN~FT outta here!!\n",get_visible_name(user).c_str());
         write_roomf(rm,"~FT~OL%s has been ~FBTHROWN~FT in from the %s\n",get_visible_name(user).c_str(),old_room->name);
      }
      else if( multi_move_user_msg(user, rm, teleport,&followers) ) {/* erm, nothing :) */ }
      else if (!user->vis) 
      {
         write_roomf(rm,"%s\n",stringLibrary->makeString("invisenter").c_str());
         write_room_exceptf(user->room,user,"%s\n",stringLibrary->makeString("invisleave").c_str());
      }
      else if( teleport==GO_MODE_MAGICAL_VORTEX) 
      {
         write_room_exceptf(old_room,user,"~FT~OL%s whistle-clicks a spell and vanishes into a magical blue vortex!\n",user->name);
         write_roomf(rm,"~FT~OL%s appears from a magical blue vortex!\n",user->name);
      }
      else 
      {
         StrGlobalVar *in_phrase    = (StrGlobalVar*)user->getVar("in_phrase");
         StrGlobalVar *out_phrase   = (StrGlobalVar*)user->getVar("out_phrase");

         if (rm->secret) write_room_exceptf(user->room,user,"%s %s to a secret place.\n",user->name,out_phrase->get().c_str() );	
         else write_room_exceptf(user->room,user,"%s %s to the %s.\n",user->name,out_phrase->get().c_str(),rm->name);
         write_roomf(rm,"%s %s.\n",user->name,in_phrase->get().c_str());
      }
   }

   user->room=rm;
   look(user);
   multi_move_user_move(user,&followers);
   reset_access(old_room);

   return 0;
}


/*** Move to another room ***/
int go(UR_OBJECT user)
{
   StringLibrary * stringLibrary = StringLibrary::getInstance();
   RM_OBJECT rm;
   int i;

   if (words.word_count<2) 
   {
      rm=get_room(globalSpecialRoomNames.getMainRoomName()->get().c_str());
      if (rm==user->room) write_userf(user,"You are already in the %s!\n",rm->name);
      else move_user(user,rm,GO_MODE_NORMAL_GO);  
      return 0;
   }
   if (user->level == LEV_MIN)
   {
      write_userf(user,"~OLSorry, you are in ~FR%s~FW, you cannot move!\n",globalSpecialRoomNames.getJailRoomName()->get().c_str());
      return 0;
   }
   if ((rm=get_room(words.word[1]))==NULL) 
   {
      write_user_crt(user,stringLibrary->makeString("nosuchroom").c_str());
      return 0;
   }
   if (rm==user->room) 
   {
      write_userf(user,"You are already in the %s!\n",rm->name);
      return 0;
   }

   /* See if link from current room */
   for(i=0;i<MAX_LINKS;++i) 
   {
      if (user->room->link[i]==rm) 
      {
         move_user(user,rm,GO_MODE_NORMAL_GO);  return 0;
      }
   }
   if (user->level<LEV_FOU)
   {
      write_userf(user,"The %s is not adjoined to here.\n",rm->name);
      return 0;
   }
   move_user(user,rm,GO_MODE_MAGICAL_VORTEX);
   return 0;
}

/*** Wizard moves a user to another room ***/
int move(UR_OBJECT user)
{
   StringLibrary * stringLibrary = StringLibrary::getInstance();
   UR_OBJECT u;
   RM_OBJECT rm;
   pod_string name;

   if (words.word_count<2) 
   {
      write_user(user,"Usage: move <user> [<room>]\n");  
      return 0;
   }
   if (!(u=get_user(words.word[1]))) 
   {
      write_user_crt(user,stringLibrary->makeString("notloggedon").c_str());
      return 0;
   }
   if (words.word_count<3) rm=user->room;
   else 
   {
      if ((rm=get_room(words.word[2]))==NULL) 
      {
         write_user_crt(user,stringLibrary->makeString("nosuchroom").c_str());
         return 0;
      }
   }
   if (user==u) 
   {
      write_user(user,"Trying to move yourself this way is the fourth sign of madness.\n");  
      return 0;
   }
   if (u->level>=user->level) 
   {
      write_user(user,"You cannot move a user of equal or higher level than yourself.\n");
      return 0;
   }
   if (rm==u->room) 
   {
      write_userf(user,"%s is already in the %s.\n",u->name,rm->name);
      return 0;
   }
   if (!has_room_access(user,rm)) 
   {
      write_userf(user,"The %s is currently private, %s cannot be moved there.\n",rm->name,u->name);
      return 0;
   }
   write_user(user,"~FT~OLYou whistle-click an ancient spell...\n");
   name=get_visible_name(user);
   write_room_exceptf(user->room,user,"~FT~OL%s whistle-clicks an ancient spell...\n",name.c_str());
   move_user(u,rm,GO_MODE_GIANT_FLIPPER);
   prompt(u);
   return 0;
}

/* Throw a user to another room */
/* If thrower is a wiz, user can be thrown anywhere inside talker */
/* If not, can only be thrown to an ajoining room */
int throw_user(UR_OBJECT user)
{
   StringLibrary * stringLibrary = StringLibrary::getInstance();
   UR_OBJECT u;
   RM_OBJECT rm;
         
   if (words.word_count<2) 
   {
      write_user(user,"Usage: throw <user> [<room>]\n");  
      return 0;
   }
   if (!(u=get_user(words.word[1]))) 
   {
      write_user_crt(user,stringLibrary->makeString("notloggedon").c_str());
      return 0;
   }
   if (words.word_count<3) rm=user->room;
   else 
   {  
      if ((rm=get_room(words.word[2]))==NULL) 
      {
         write_user_crt(user,stringLibrary->makeString("nosuchroom").c_str());
         return 0;
      }
   }
   if (user==u) 
   {
      write_user(user,"Trying to throw yourself eh?  I don't think so... :P\n");
      return 0;
   }
   if (u->level>user->level) 
   {
      write_user(user,"You cannot throw a user of higher level than yourself.\n");
      return 0;
   }
   if ((u->room!=user->room) && (user->level<LEV_FIV))
   {
      write_user(user,"You cannot throw someone that is not in the room!\n");
      return 0;
   }
   if (rm==u->room) 
   {
      write_userf(user,"%s is already in the %s.\n",u->name,rm->name);
      return 0;
   }
   if (!has_room_access(user,rm)) 
   {
      write_userf(user,"The %s is currently private, %s cannot be thrown there.\n",rm->name,u->name);
      return 0;  
   }
   write_userf(user,"~FT~OLYou heft %s up into the air...\n",u->name);
   write_room_exceptf(user->room,user,"~FT~OL%s hefts %s up into the air...\n",get_visible_name(user).c_str(),get_visible_name(u).c_str());
   move_user(u,rm,GO_MODE_THROWN);
   prompt(u);
   return 0;
}

int bring(UR_OBJECT user)
{
   StringLibrary * stringLibrary = StringLibrary::getInstance();
   UR_OBJECT u;
   RM_OBJECT rm;

   if (user->level == LEV_MIN)
   {
      write_user(user,"You are jailed, you can't bring anyone in.\n"); 
      return 0;
   }
   if (words.word_count<2) 
   {
      write_user(user,"bring who?\n");  
      return 0;
   }
   rm=user->room;
   if (!(u=get_user(words.word[1]))) 
   {
      write_user_crt(user,stringLibrary->makeString("notloggedon").c_str());
      return 0;
   }
   if (u==user) 
   {
      write_user(user,"Trying to bring yourself where you already are makes no sense!\n");
      return 0;
   }
   if (u->room==rm) 
   {
      write_userf(user,"%s is already here!\n",u->name);
      return 0;
   }
   if (u->invite_room==rm) 
   {
      write_userf(user,"%s has already been asked into here.\n",u->name);
      return 0;
   }
   write_userf(user,"You ask %s to come in.\n",u->name);
   /* name=get_visible_name(user); */
   /* when bringing someone he'd better know who's inviting , so I changed to user-> name (crandonkhpin) */
   write_userf(u,"%s would like you to join %s in the %s.\nType .join %s to join %s.\n",user->name,get_gender(user,"him"),rm->name,user->name,get_gender(user,"him"));
   u->invite_room=rm;
   return 0;
}

int join(UR_OBJECT u)
{
   StringLibrary * stringLibrary = StringLibrary::getInstance();
   UR_OBJECT target;

   if (u->level == LEV_MIN)                         write_userf(u,"~OLYou are in ~FR%s~FW....you cannot join anyone.\n",globalSpecialRoomNames.getJailRoomName()->get().c_str());
   else if (words.word_count < 2)                   write_user(u, "~FRSyntax: join <user>\n");
   else if ((target=get_user(words.word[1]))==NULL) write_user_crt(u,stringLibrary->makeString("notloggedon").c_str());
   else if (target->name == u->name)          write_user(u,"~FRWhy are you trying to join yourself?\n");
   else if (target->room == u->room)          write_userf(u,"~OLYou are already in the same room as %s!\n",target->name);
   else 
   {
      if (u->level==LEV_ONE)
      {
         if (u->invite_room==NULL)  write_user(u,"You have not been invited anywhere.\n");
         else if (u->invite_room!=target->room) 
         {
            write_userf(u,"You have not been invited to %s's current room, sorry.\n",u->name);
            u->invite_room=NULL;
         }
         else move_user(u,target->room,GO_MODE_MAGICAL_VORTEX);
      }
      else move_user(u,target->room,GO_MODE_MAGICAL_VORTEX);
   }
   return 0;
}

int back(UR_OBJECT user)
{
   RM_OBJECT rm;

   if (user->level == LEV_MIN) write_user(user,"You are under arrest, you can not go anywhere!\n");
   else
   {
      rm=get_room(globalSpecialRoomNames.getMainRoomName()->get().c_str());
      if (user->room==rm) write_userf(user,"You are already there!\n");
      else
      {
         move_user(user,rm,GO_MODE_GIANT_FLIPPER);
         write_userf(user,"There's no place like home... there's no place like home...\n");
      }
   }
   return 0;
}

/*** Ask to be let into a private room ***/
int letmein(UR_OBJECT user)
{
   StringLibrary * stringLibrary = StringLibrary::getInstance();
   RM_OBJECT rm;
   int i;

   if (words.word_count<2) 
   {
      write_user(user,"Let you into where?\n");  
      return 0;
   }
   if ((rm=get_room(words.word[1]))==NULL) 
   {
      write_user_crt(user,stringLibrary->makeString("nosuchroom").c_str());
      return 0;
   }
   if (rm==user->room) 
   {
      write_userf(user,"You are already in the %s!\n",rm->name);
      return 0;
   }
   for(i=0;i<MAX_LINKS;++i) 
      if (user->room->link[i]==rm) goto GOT_IT;
   write_userf(user,"The %s is not adjoined to here.\n",rm->name);

   return 0;

   GOT_IT:
   if (!(rm->access & ROOM_PRIVATE)) 
   {
      write_userf(user,"The %s is currently public.\n",rm->name);
      return 0;
   }
   write_userf(user,"You shout asking to be let into the %s.\n",rm->name);
   write_room_exceptf(user->room,user,"%s shouts asking to be let into the %s.\n",user->name,rm->name);
   write_roomf(rm,"%s shouts asking to be let in.\n",user->name);
   return 0;
}

/*** Invite a user into a private room ***/
int invite(UR_OBJECT user)
{
   StringLibrary * stringLibrary = StringLibrary::getInstance();
   UR_OBJECT u;
   RM_OBJECT rm;
   /* char *name; */

   if (words.word_count<2) 
   {
      write_user(user,"Invite who?\n");  
      return 0;
   }
   rm=user->room;
   if (!(rm->access & ROOM_PRIVATE)) 
   {
      write_user(user,"This room is currently public.\n");
      return 0;
   }
   if (!(u=get_user(words.word[1]))) 
   {
      write_user_crt(user,stringLibrary->makeString("notloggedon").c_str());
      return 0;
   }
   if (u==user) 
   {
      write_user(user,"Inviting yourself to somewhere is the third sign of madness.\n");
      return 0;
   }
   if (u->room==rm) 
   {
      write_userf(user,"%s is already here!\n",u->name);
      return 0;
   }
   if (u->invite_room==rm) 
   {
      write_userf(user,"%s has already been invited into here.\n",u->name);
      return 0;
   }
   write_userf(user,"You invite %s in.\n",u->name);
   /* name=get_visible_name(user); */
   /* when inviting someone he'd better know who's inviting , so I changed to user-> name (crandonkhpin) */
   write_userf(u,"%s has invited you into the %s.\n",user->name,rm->name);
   u->invite_room=rm;
   return 0;
}

