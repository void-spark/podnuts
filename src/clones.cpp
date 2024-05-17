#include "general_headers.h"
#include "chitter.h"
#include "string_misc.h"
#include "StringLibrary.h"
#include "tandem.h"
#include "clones.h"

int is_clone(UR_OBJECT user)
{
   return (user->type==CLONE_TYPE);
}

int reset_clone_socks(UR_OBJECT old_user,UR_OBJECT new_user)
{
   UR_OBJECT u_loop;

   /* Reset the sockets on any clones */
   for(u_loop=user_first;u_loop!=NULL;u_loop=u_loop->next) 
   {
      if (is_clone(u_loop) && u_loop->owner==old_user)
      {
         u_loop->socket = new_user->socket;  
         u_loop->owner  = new_user;
      }
   }
   return 0;
}

int clones_count()
{
   int num_clones=0;
   UR_OBJECT u;
 
   for(u=user_first;u!=NULL;u=u->next) 
   {
      if (is_clone(u)) num_clones++;
   }
   return num_clones;
}

/*** Destroy all clones belonging to given user ***/
int destroy_user_clones(UR_OBJECT user,int silent)
{
   UR_OBJECT u;
   WriteRoomStream *roomStream = WriteRoomStream::getInstance();

   for(u=user_first;u!=NULL;u=u->next)
   {
      if (is_clone(u) && u->owner==user)
      {
         if(!silent)
         {
            *roomStream  << setRoom( u->room ) << "The clone of " << u->name << " shimmers and vanishes.\n" << pod_send;
         }
         destruct_user(u);
      }
   }
   return 0;
}

/*** Clone a user in another room ***/
int create_clone(UR_OBJECT user)
{
   StringLibrary * stringLibrary = StringLibrary::getInstance();
   UR_OBJECT new_clone,u_loop;
   RM_OBJECT rm;
   int clone_cnt=0;
   WriteRoomStream *roomStream = WriteRoomStream::getInstance();

   if (words.word_count<2) rm=user->room;
   else if (!(rm=get_room(words.word[1])))
   {
      write_user_crt(user,stringLibrary->makeString("nosuchroom").c_str());
      return 0;
   }

   if (!has_room_access(user,rm))
   {
      write_user(user,"That room is currently private, you cannot create a clone there.\n");
      return 0;
   }

   for(u_loop=user_first;u_loop!=NULL;u_loop=u_loop->next)
   {
      if (is_clone(u_loop) && u_loop->owner==user)
      {
         if (u_loop->room==rm)
         {
            write_userf(user,"You already have a clone in the %s.\n",rm->name);
            return 0;
         }
         if (++clone_cnt == max_clones.get())
         {
            write_user(user,"You already have the maximum number of clones allowed.\n");
            return 0;
         }
      }
   }
   if (!(new_clone=create_user()))
   {
      write_userf(user,"%s: Unable to create copy.\n",stringLibrary->makeString("syserror").c_str());
      write_syslog("ERROR: Unable to create user copy in clone().\n",0);
      return 0;
   }
   new_clone->type   = CLONE_TYPE;
   new_clone->room   = rm;
   new_clone->socket = user->socket;
   new_clone->owner  = user;
   strcpy(new_clone->name,user->name);

   StrGlobalVar *desc     = (StrGlobalVar*)new_clone->getVar("desc");
   desc->set("~BR(CLONE)");

   if (rm==user->room) write_user (user,"~FB~OLYou whisper a haunting spell and a clone is created here.\n");
   else                write_userf(user,"~FB~OLYou whisper a haunting spell and a clone is created in the %s.\n",rm->name);
   *roomStream  << setRoom( user->room ) << addExcept( user ) << "~FB~OL"            << get_visible_name(user) << " whispers a haunting spell...\n" << pod_send;
   *roomStream  << setRoom( rm )         << addExcept( user ) << "~FB~OLA clone of " << user->name             << " appears in a swirling magical mist!\n" << pod_send;

   return 0;
}

/*** Destroy user clone ***/
int destroy_clone(UR_OBJECT user)
{
   StringLibrary * stringLibrary = StringLibrary::getInstance();
   UR_OBJECT u_loop,target_user;
   RM_OBJECT rm;
   WriteRoomStream *roomStream = WriteRoomStream::getInstance();

   if (words.word_count<2) rm=user->room;
   else if (!(rm=get_room(words.word[1])))
   {
      write_user_crt(user,stringLibrary->makeString("nosuchroom").c_str());
      return 0;
   }

   if (words.word_count>2)
   {
      if (!(target_user=get_user(words.word[2])))
      {
         write_user_crt(user,stringLibrary->makeString("notloggedon").c_str());
         return 0;
      }
      if (target_user->level>=user->level)
      {
         write_user(user,"You cannot destroy the clone of a user of an equal or higher level.\n");
         return 0;
      }
   }
   else target_user=user;

   for(u_loop=user_first;u_loop!=NULL;u_loop=u_loop->next)
   {
      if (is_clone(u_loop) && u_loop->room==rm && u_loop->owner==target_user)
      {
         destruct_user(u_loop);
         reset_access(rm);
         write_user(user,"~FM~OLYou whisper a sharp spell and the clone is destroyed.\n");
         *roomStream  << setRoom( user->room ) << addExcept( user ) << "~FM~OL"            << get_visible_name(user) << " whispers a sharp spell...\n" << pod_send;
         *roomStream  << setRoom( rm ) << "~FM~OLThe clone of " << target_user->name << " shimmers and vanishes.\n" << pod_send;
         if (target_user!=user) write_userf(target_user,"~OLSYSTEM: ~FR%s has destroyed your clone in the %s.\n",user->name,rm->name);

	 return 0;
      }
   }
   if (target_user==user) write_userf(user,"You do not have a clone in the %s.\n",rm->name);
   else                   write_userf(user,"%s does not have a clone the %s.\n",target_user->name,rm->name);

   return 0;
}

/*** Show users own clones ***/
int myclones(UR_OBJECT user)
{
   UR_OBJECT u_loop;
   int cnt=0;

   for(u_loop=user_first;u_loop!=NULL;u_loop=u_loop->next) 
   {
      if (!is_clone(u_loop) || u_loop->owner != user) continue;
      if (++cnt==1) write_user(user,"\n~BB*** Rooms you have clones in ***\n\n");
      write_userf(user,"  %s\n",u_loop->room->name);
   }
   if (!cnt) write_user (user,"You have no clones.\n");
   else      write_userf(user,"\nTotal of %d clones.\n\n",cnt);

   return 0;
}

/*** Show all clones on the system ***/
int allclones(UR_OBJECT user)
{
   UR_OBJECT u_loop;
   int cnt=0;

   for(u_loop=user_first;u_loop!=NULL;u_loop=u_loop->next) 
   {
      if (!is_clone(u_loop)) continue;
      if (++cnt==1) write_userf(user,"\n~BB*** Current clones %s ***\n\n",long_date(1).c_str());
      write_userf(user,"%-15s : %s\n",u_loop->name,u_loop->room->name);
   }
   if (!cnt) write_user(user,"There are no clones on the system.\n");
   else write_userf(user,"\nTotal of %d clones.\n\n",cnt);

   return 0;
}

/*** User swaps places with his own clone. All we do is swap the rooms the
	objects are in. ***/
int clone_switch(UR_OBJECT user)
{
   StringLibrary * stringLibrary = StringLibrary::getInstance();
   UR_OBJECT u_loop;
   RM_OBJECT rm;
   WriteRoomStream *roomStream = WriteRoomStream::getInstance();

   if (words.word_count<2)
   {
      write_user(user,"Usage: switch <room clone is in>\n");
      return 0;
   }
   if (!(rm=get_room(words.word[1])))
   {
      write_user_crt(user,stringLibrary->makeString("nosuchroom").c_str());
      return 0;
   }
   follow_kill(user,FALSE);

   for(u_loop=user_first;u_loop!=NULL;u_loop=u_loop->next)
   {
      if (is_clone(u_loop) && u_loop->room==rm && u_loop->owner==user)
      {
         write_user(user,"\n~FB~OLYou experience a strange sensation...\n");
         u_loop->room=user->room;
         user->room=rm;
         *roomStream  << setRoom( user->room ) << addExcept( user ) << "The clone of " << u_loop->name << " comes alive!\n" << pod_send;
         *roomStream  << setRoom( u_loop->room ) << addExcept( u_loop ) << u_loop->name << " turns into a clone!\n" << pod_send;
         look(user);
         return 0;
      }
   }
   write_user(user,"You do not have a clone in that room.\n");
   return 0;
}

/*** Make a clone speak ***/
int clone_say(UR_OBJECT user, char *inpstr)
{
   StringLibrary * stringLibrary = StringLibrary::getInstance();
   RM_OBJECT rm;
   UR_OBJECT u_loop;

   if (words.word_count<3)            write_user(user,"Usage: csay <room clone is in> <message>\n");
   else if (!(rm=get_room(words.word[1]))) write_user_crt(user,stringLibrary->makeString("nosuchroom").c_str());
   else 
   {
      for(u_loop=user_first;u_loop!=NULL;u_loop=u_loop->next) 
      {
         if (is_clone(u_loop) && u_loop->room==rm && u_loop->owner==user) 
         {
            say_gen(u_loop,remove_first(inpstr));
            return 0;
         }
      }
      write_user(user,"You do not have a clone in that room.\n");
   }
   return 0;
}

/* Make a clone emote something */
/* Added by Vaghn 1998 */
int clone_emote(UR_OBJECT user, char *inpstr)
{
   StringLibrary * stringLibrary = StringLibrary::getInstance();
   RM_OBJECT rm;
   UR_OBJECT u_loop;

   if (words.word_count<3)            write_user(user,"Usage: cemote <room clone is in> <message>\n");
   else if (!(rm=get_room(words.word[1]))) write_user_crt(user,stringLibrary->makeString("nosuchroom").c_str());
   else
   {
      for(u_loop=user_first;u_loop!=NULL;u_loop=u_loop->next) 
      {
         if (is_clone(u_loop) && u_loop->room==rm && u_loop->owner==user) 
         {
           emote(u_loop,remove_first(inpstr));
           return 0;
         }
      }
      write_user(user,"You do not have a clone in that room.\n");
   }
   return 0;
}

/*** Set what a clone will hear, either all speach , just bad language
	or nothing. ***/
int clone_hear(UR_OBJECT user)
{
   StringLibrary * stringLibrary = StringLibrary::getInstance();
   RM_OBJECT rm;
   UR_OBJECT u_loop;

   if ( ( words.word_count<3 ) || 
          ( strcmp(words.word[2],"all")     && 
            strcmp(words.word[2],"swears")  && 
            strcmp(words.word[2],"nothing")    ) ) 
   {
      write_user(user,"Usage: chear <room clone is in> all/swears/nothing\n");
      return 0;
   }
   if ((rm=get_room(words.word[1]))==NULL) 
   {
      write_user_crt(user,stringLibrary->makeString("nosuchroom").c_str());
      return 0;
   }
   for(u_loop=user_first;u_loop!=NULL;u_loop=u_loop->next) 
   {
      if (is_clone(u_loop) && u_loop->room==rm && u_loop->owner==user) break;
   }
   if (u_loop==NULL) 
   {
      write_user(user,"You do not have a clone in that room.\n");
      return 0;
   }
   if (!strcmp(words.word[2],"all")) 
   {
      u_loop->clone_hear=CLONE_HEAR_ALL;
      write_user(user,"Clone will now hear everything.\n");
      return 0;
   }
   if (!strcmp(words.word[2],"swears")) 
   {
      u_loop->clone_hear=CLONE_HEAR_SWEARS;
      write_user(user,"Clone will now only hear swearing.\n");
      return 0;
   }
   u_loop->clone_hear=CLONE_HEAR_NOTHING;
   write_user(user,"Clone will now hear nothing.\n");

   return 0;
}

