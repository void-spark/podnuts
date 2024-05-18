#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "general_headers.h"
#include "string_misc.h"
#include "move.h"
#include "tandem.h"

int tandem_get_followers(UR_OBJECT user ,struct tandem_list *followers)
{
   int i;
   UR_OBJECT u;

   followers->follow_count=0;
   
   for(i=0; i < MAX_TANDEM_FOLLOWERS; i++ )
   {
      followers->tandem_partner[i]=NULL;
   }
   for(u=user_first;u!=NULL;u=u->next) /* is somephin following me ? */
   {
      if( u->follow_partner == user && u->follow_mode == FOLLOW_MODE_FOLLOWING)
      {
         followers->tandem_partner[followers->follow_count]=u;
         followers->follow_count++;
      }
   }
   return 0;
}

/** Ask another user if you can follow **/
int follow_ask(UR_OBJECT user, char *inpstr)
{
   pod_string name;
   UR_OBJECT u;
   int follow_count=0;

   for(u=user_first;u!=NULL;u=u->next) 
   {
      if( u->follow_partner == user )
      {
         follow_count++;
      }
   }

   if (user->follow_partner) 
   {
      write_userf(user,"You are already swimming in tandem with %s.\n",user->follow_partner->name);  
      return 0;
   }
   if (user->level == LEV_MIN)
   {
      write_userf(user,"~OLYou are in ~FR%s~FW....you can't follow anyphin.\n",globalSpecialRoomNames.getJailRoomName()->get().c_str());
      return 0;
   }
   if (words.word_count<2) 
   {
      write_user(user,"Swim with who ?\n");  
      return 0;
   }
   
   u = get_user_advanced( user, words.word[1], GUA_SHOW_ERRORS, GUA_ROOM, GUA_NOT_CLOAKED );
   
   if( u == 0 ) 
   {
      return 0;
   }   
   
   name=get_visible_name(user);
   if (u==user) 
   {
      write_user(user,"You can't swim with yourself sillyphin! :)\n");
      return 0;
   }

   if(follow_count>=MAX_TANDEM_FOLLOWERS)
   {
      write_userf(user,"You can't have more then %i phins tandemswimming with you\n",MAX_TANDEM_FOLLOWERS);  
      return 0;
   }
   if(u->follow_mode==FOLLOW_MODE_ASKING)
   {
      write_userf(u,"%s has tried to ask to tandemswim with you.\n",name.c_str());
      write_userf(user,"%s is already being asked to swim with somephin else, try later.\n",u->name);  
      return 0;
   }
   if(u->follow_mode==FOLLOW_MODE_FOLLOWING)
   {
       write_userf(u,"%s has tried to ask to tandem swim with you.\n",name.c_str());
       write_userf(user,"%s is already tandem swimming with somephin else, try later.\n",u->name);  
       return 0;
   }
   if (afk_check_verbal(user,u)) return 0;
   if (u->ignall && NO_OVERR(user,u)) 
   {
      if (u->malloc_start!=NULL) write_userf(user,"%s is using the editor at the moment.\n",u->name);
      else write_userf(user,"%s is ignoring everyone at the moment.\n",u->name);
      return 0;
   }

   StrGlobalVar *color = (StrGlobalVar*)user->getVar("Color");

   inpstr=remove_first(inpstr);

   pod_stringstream outputStream;
   outputStream << "~OL" << color->get() << "You offer to swim with " << u->name << "\n";
   write_user(user, (char*)outputStream.str().c_str() );
   record_tell(user, (char*)outputStream.str().c_str() );
   outputStream.str("");
   outputStream << "~OL" << get_visible_color(user) << name << " has offered to tandem swim with you {type .accept if you agree , .break to reject}~RS\n";
   write_user(u, (char*)outputStream.str().c_str() );
   record_tell(u, (char*)outputStream.str().c_str() );
   u->follow_partner=user;
   u->follow_mode=FOLLOW_MODE_ASKING;
   return 0;
}

/** accept a follow request **/
int follow_accept(UR_OBJECT user)
{
   WriteRoomStream *roomStream = WriteRoomStream::getInstance();

   UR_OBJECT u;
   if (user->follow_mode==FOLLOW_MODE_NONE)
   {
      write_userf(user,"No one has asked to swim with you .\n");
      return 0;
   }

   u=user->follow_partner;
   if (u->room != user->room)
   {
      write_user(user,"You need to be in the same room with somephin to start tandem swimming :)\n");
      return 0;
   }
   user->follow_mode=FOLLOW_MODE_FOLLOWING;
   if(!strcmp(get_gender(user,"he"),"they")) write_userf(user,"You are now swimming in tandem with %s and will be with %s wherever %s go.\n",u->name,get_gender(u,"him"),get_gender(u,"he"));
   else write_userf(user,"You are now swimming in tandem with %s and will be with %s wherever %s goes.\n",u->name,get_gender(u,"him"),get_gender(u,"he"));
   write_userf(u,"%s swims close and tight at your side.\n",user->name);

   *roomStream  << setRoom( user->room ) << addExcept( user ) << addExcept( u )
               << user->name << " joins " << u->name << "'s side in a spectacular tandem swim formation.\n"
               << pod_send ;

   return 0; 
}

int follow_kill(UR_OBJECT user,int silent)
{
   UR_OBJECT u;
   int done_something=0;
   if (user->follow_mode==FOLLOW_MODE_FOLLOWING)
   {
      if(!silent)
      {
         write_userf(user->follow_partner,"%s stops swimming with you.\n",user->name);
         write_userf(user,"You stop swimming with %s.\n",user->follow_partner->name);
      }
      user->follow_partner=NULL;
      user->follow_mode=FOLLOW_MODE_NONE;
      done_something=1;
   }
   if (user->follow_mode==FOLLOW_MODE_ASKING)
   {
      if(!silent)
      {
         write_userf(user->follow_partner,"%s does not wish to tandem swim with you right now.\n",user->name);
         write_userf(user,"You decline %s's offer at this time.\n",user->follow_partner->name);
      }
      user->follow_partner=NULL;
      user->follow_mode=FOLLOW_MODE_NONE;
      done_something=1;
   }
   for(u=user_first;u!=NULL;u=u->next) 
   {
      if((u->follow_partner == user) && (u->follow_mode == FOLLOW_MODE_FOLLOWING))
      {
         if(!silent)
         {
            write_userf(u,"%s stops tandem swimming with you.\n",u->follow_partner->name);
            write_userf(u->follow_partner,"You stop tandem swimming with %s.\n",u->name);
         }
         u->follow_partner=NULL;
         u->follow_mode=FOLLOW_MODE_NONE;
         done_something=1;
      }
      if((u->follow_partner == user) && (u->follow_mode == FOLLOW_MODE_ASKING))
      {
         if(!silent)
         {
            write_userf(u,"%s withdraws %s request to tandem swim with you.\n",u->follow_partner->name,get_gender(u,"his"));
            write_userf(u->follow_partner,"You take back your request to swim with %s.\n",u->name);
         }
         u->follow_partner=NULL;
         u->follow_mode=FOLLOW_MODE_NONE;
      done_something=1;
      }
   }
   return done_something;
}

int follow_break(UR_OBJECT user)
{
   UR_OBJECT u,tandem_partner[MAX_TANDEM_FOLLOWERS];
   int i,follow_count=0;

   if(user->follow_mode == FOLLOW_MODE_NONE)
   {
      for(i=0; i < MAX_TANDEM_FOLLOWERS; i++ )
      {
         tandem_partner[i]=NULL;
      }
      for(u=user_first;u!=NULL;u=u->next) /* is somephin following me ? */
      {
         if( u->follow_partner == user && u->follow_mode == FOLLOW_MODE_FOLLOWING)
         {
            tandem_partner[follow_count]=u;
            follow_count++;
         }
      }
   }

   if(user->follow_mode == FOLLOW_MODE_FOLLOWING)
   {
      write_roomf(user->room,"~FT~OLWith opposite angled pec flippers, %s and %s break apart their tandem swim formation.\n",get_visible_name(user).c_str(),get_visible_name(user->follow_partner).c_str());
   }

   if(!follow_kill(user,FALSE))
   {
      write_user(user,"You aren't tandem swimming with anyphin\n");
      return 0;
   }   
   if(follow_count>=1)
   {
      pod_stringstream outputStream;

      outputStream << "~FT~OLWith opposite angled pec flippers, " << get_visible_name(user);
      for(i=0;i< follow_count;i++)
      {
         if(i != follow_count-1)
         {
            outputStream << ", ";
         }
         else
         {
            outputStream << " and ";
         }
         outputStream << get_visible_name(tandem_partner[i]);
      }
      outputStream << " break apart their tandem swim formation.\n";
      write_room(user->room,(char*)outputStream.str().c_str());
   }

   return 0;
}

/* moving function */
int multi_move_user_msg(UR_OBJECT user, RM_OBJECT rm,int mode,struct tandem_list *followers)
{
   int i;
   pod_stringstream outputStream;

   if(!followers->follow_count)
   {
      return 0;
   }

   if(mode == GO_MODE_MAGICAL_VORTEX)
   {
      outputStream << "~FT~OL" << get_visible_name(user);
   }
   else
   {
      outputStream << "~FT~OLIn perfect symmetry, " << get_visible_name(user);
   }

   for(i=0;i< followers->follow_count;i++) //pc100
   {
      if(i != followers->follow_count-1)
      {
         outputStream << ", ";
      }
      else
      {
         outputStream << " and ";
      }
      outputStream << get_visible_name(followers->tandem_partner[i]);
   }
    
   if(mode == GO_MODE_MAGICAL_VORTEX) 
   {
      outputStream << " shimmer and vanish into a magical blue vortex.\n";
   }
   else
   {
      if (rm->secret) 
      {
         outputStream << " swim out to a secret place.\n";
      }
      else
      {
         outputStream << " swim out to the ";
         outputStream << rm->name;
         outputStream << ".\n";
      }
   }

   write_room_except(user->room,(char*)outputStream.str().c_str(),user);

   outputStream.str("");

   if(mode == GO_MODE_MAGICAL_VORTEX)
   {
      outputStream << "~FT~OLWith a burst of light, " << get_visible_name(user);
   }
   else
   {
      outputStream << "~FT~OLIn perfect symmetry, " << get_visible_name(user);
   }

   for(i=0;i< followers->follow_count;i++)
   {
      if(i != followers->follow_count-1)
      {
         outputStream << ", ";
      }
      else
      {
         outputStream << " and ";
      }
      outputStream << get_visible_name(followers->tandem_partner[i]);
   }
   if(mode == GO_MODE_MAGICAL_VORTEX)
   {
      outputStream << " tandem swim in from the void.\n"; // other
   }
   else
   {
      outputStream << " come swimming in.\n";
   }
   write_room(rm,(char*)outputStream.str().c_str());

   return 1;
}

int multi_move_user_move(UR_OBJECT user,struct tandem_list *followers)
{
   int i;
   pod_stringstream outputStream;

   if(!followers->follow_count)
   {
      return 0;
   }
   outputStream << "~FT~OLIn perfect symmetry with you, ";

   for(i=0;i< followers->follow_count-1;i++)
   {
      outputStream << get_visible_name(followers->tandem_partner[i]);
      if(i != followers->follow_count-2)
      {
         outputStream << ", ";
      }
      else
      {
         outputStream << " and ";
      }
   }
   outputStream << get_visible_name(followers->tandem_partner[followers->follow_count-1]);
   if(followers->follow_count != 1)
   {
      outputStream << " swim in.\n";
   }
   else
   {
      outputStream << " swims in.\n";
   }

   write_user(user,(char*)outputStream.str().c_str());

   for(i=0;i< followers->follow_count;i++)
   {
      move_user(followers->tandem_partner[i],user->room,GO_MODE_SILENT);
   }
   return 1;
}

