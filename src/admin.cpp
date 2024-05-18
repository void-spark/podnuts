#include <ctype.h>
#include <stdarg.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <iomanip>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "general_headers.h"
#include "string_misc.h"
#include "parse.h"
#include "move.h"
#include "tandem.h"
#include "StringLibrary.h"
#include "loadsave_user.h"
#include "macro.h"
#include "smail.h"
#include "more.h"
#include "admin.h"

extern char *noyes2[];

int no_delold(UR_OBJECT user)
{
   UR_OBJECT u;
   int on;

   if (words.word_count<2) 
   {
      write_user(user,"Usage: nodelold <user>\n");
      return 0;
   }
   words.word[1][0]=toupper(words.word[1][0]);

        if( (u=get_user(words.word[1])) )                         on=TRUE;
   else if( (u=temp_user_spawn(user,words.word[1],"no_delold")) ) on=FALSE;
   else return 0;

   if(!u->no_delold)
   {
      u->no_delold = TRUE;
      write_userf(user,"%s will now no longer be deleted after %d days.\n",u->name,purgedays.get());
   }
   else 
   {
      u->no_delold = FALSE;
      write_userf(user,"%s will now be deleted after %d days.\n",u->name,purgedays.get());
   }
   logStream << setLogfile( SYSLOG ) << user->name << " changed " << u->name << "'s nodelold setting.\n" << pod_send;

   if(!on)
   {
      save_user(u);
   }

   temp_user_destroy(u);
   return 0;
}

int pop_level(UR_OBJECT user, char *inpstr)
{
   UR_OBJECT u;
   int lev;
   StringLibrary * stringLibrary = StringLibrary::getInstance();
   WriteRoomStream *roomStream = WriteRoomStream::getInstance();

   if (words.word_count < 3)                  write_user(user,"Usage: pop <user> <level>\n");
   else if ( ! (u=get_user(words.word[1]) ) ) write_user_crt(user,stringLibrary->makeString("notloggedon").c_str());
   else if (u == user)                  write_user(user,"Nice try but you can't change your own level.\n");
   else if (u->level >= user->level)    write_user(user,"You cannot change the level of someone of equal or higher level than yourself.\n");
   else
   {
      pod_string levelString = words.word[2];
      strToUpper( levelString );
      lev=get_level( levelString.c_str() );
      if (lev >= user->level)    write_user(user,"You cannot pop a user to a level equal or higher than yourself.\n");
      else if(lev != -2 && lev != LEV_MIN && lev != LEV_BOT)
      {
         u->level=lev; 

         pod_ostringstream outStream;
         outStream << "~OL" << user->name << " ~RSpops~OL " << u->name << " ~RSto level: ~FR~OL" << getLevelName(u->level) << "\n";
         *roomStream  << setRoom( NULL ) << addExcept( u ) << outStream.str() << pod_send;
         logStream << setLogfile( PROMOLOG ) << outStream.str() << pod_send;

         write_userf(u,"~OL%s ~RSpops you to level: ~FR~OL%s\n",user->name,getLevelName(u->level));
      }
      else write_user(user,"Unknown level identifier.\n");
   }
   return 0;
}


int log_user(UR_OBJECT user,char* logger,char *str)
{
   char filename[80];
   FILE *fp;

   sprintf(filename,"%s/%s.I",USERFILES,user->name);
   if(!(fp=fopen(filename,"a")))
   {
      write_syslogf("ERROR: Couldn't open file %s to write in log_user().\n",TRUE,filename);
      return 1;
   }

   fprintf(fp,"%s %-*s:  %s",long_date(3).c_str(),USER_NAME_LEN,logger,str);
   fclose(fp);
   return 0;
}

int view_user_log(UR_OBJECT user,UR_OBJECT u)
{
   char filename[80];

   write_userf(user,"\n~OL~FR***~RS~OL %s's Log ~FR***\n\n",u->name);
   sprintf(filename,"%s/%s.I",USERFILES,u->name);
   switch(more(user,user->socket,filename)) 
   {
      case 0: write_userf(user,"%s's log is empty\n",u->name);
              return 0;
      case 1: user->misc_op=MISC_MORE;
   }
   return 0;
}

int log_user_com(UR_OBJECT user, char *inpstr)
{
   StringLibrary * stringLibrary = StringLibrary::getInstance();
   UR_OBJECT u;

   if (words.word_count<2) 
   {
      write_user(user,"~OLUlog who?\n");
      return 0;                                 
   }
   if ((u=get_user(words.word[1]))!=NULL) 
   {
      if (words.word_count==2) 
      {
         view_user_log(user,u);
         return 0;
      }
      if (user==u)
      {
         write_user(user,"~OLWhy are you trying to write to your own ulog?\n");
         return 0;
      }

      pod_stringstream outputStream;
      outputStream << remove_first(inpstr) << "\n";
      log_user(u,user->name, (char*)outputStream.str().c_str() );

      write_userf(user,"~FR~OLYou wrote to %s's ulog.\n",u->name);
      return 0;
   }
   /* User not logged on */
   if ((u=create_user())==NULL) 
   {
        write_userf(user,"%s: unable to create temporary user object.\n",stringLibrary->makeString("syserror").c_str() );
        write_syslog("ERROR: Unable to create temporary user object in log_user_com().\n",0);
        return 0;
   }
   strcpy(u->name,words.word[1]);
   if (!load_user(u)) 
   {
      write_user_crt(user,stringLibrary->makeString("nosuchuser").c_str());
   }
   if (words.word_count==2) 
   {
      view_user_log(user,u);
   }
   else
   {
      pod_stringstream outputStream;
      outputStream << remove_first(inpstr) << "\n";
      log_user(u,user->name, (char*)outputStream.str().c_str() );
      write_userf(user,"~FR~OLYou wrote to %s's ulog.\n",u->name);
   }

   destruct_user(u);
   return 0;
}

/* Force someone to do something.*/
int force(UR_OBJECT user, char *inpstr)
{
   StringLibrary * stringLibrary = StringLibrary::getInstance();
   UR_OBJECT u;
   if (words.word_count<3)                write_user(user,"Force who to do what?\n");
   else if (!(u=get_user(words.word[1]))) write_user_crt(user,stringLibrary->makeString("notloggedon").c_str());
   else if (u==user)                write_user(user, "Um.. just do it yourself.\n");
   else if (user->level<=u->level)  write_user(user, "I wouldn't do that if I were you.\n");
   else
   {
      inpstr=remove_first(inpstr);
      wordfind(inpstr);
      write_userf(u, "~FW%s~RS is forcing you to: ~FW%s~RS\n", user->name,inpstr);
      write_userf(user, "You force ~FW%s~RS to: ~FW%s~RS\n", u->name, inpstr);
      parse(u, inpstr);
   }
   return 0;
}

#ifndef __FreeBSD__
int is_i_or_d_file(const struct dirent *node)
#else
int is_i_or_d_file(struct dirent *node)
#endif
{
   static const pod_string userFileExtension = ".D.xml";
   static const pod_string userLogExtension = ".I";

   pod_string name = node->d_name;

   if( name.size() >= userLogExtension.size() &&
       name.compare( name.size() - userLogExtension.size(), userLogExtension.size(), userLogExtension ) == 0 )
   {
      return TRUE;
   }

   if( name.size() >= userFileExtension.size() &&
       name.compare( name.size() - userFileExtension.size(), userFileExtension.size(), userFileExtension ) == 0 )
   {
      return TRUE;
   }

   return FALSE;
}


int list_zombie_users(UR_OBJECT user)
{
   struct dirent **dir_list;
   int file_count=0,cnt=0,user_count=0;

   write_user(user,"\n~OL~FT*** ~FWZombies List ~FT***\n\n");
   write_user(user,"Name\n\n");
   if( (file_count = scandir(USERFILES, &dir_list, is_i_or_d_file, alphasort)) == -1)
   {
      write_syslogf("Error in list_zombie_users() while opening directory '%s' : '%s'.\n",TRUE,USERFILES,strerror(errno));
      return 0;
   }
   
   pod_string name1;
   pod_string name2;
   int dotPos1;
   int dotPos2;

   WriteUserStream *userStream = WriteUserStream::getInstance();

   for(cnt=0;cnt<file_count;cnt++)
   {
      if(is_d_file(dir_list[cnt]))
      {
         continue;
      }

      name1   = dir_list[cnt]->d_name;
      dotPos1 = name1.find('.',0);

      if(cnt!=0)
      {
         name2   = dir_list[cnt-1]->d_name;
         dotPos2 = name1.find('.',0);

         if( !name1.compare( 0, dotPos1, name2, 0, dotPos2) )
         {
            continue;
         }
      }
      user_count++;
      *userStream << addUser( user ) << std::left << std::setw( USER_NAME_LEN ) << name1.substr(0,dotPos1) << std::endl << pod_send;
   }

   for(cnt=0;cnt<file_count;cnt++)
   {
      free(dir_list[cnt]);
   }
   free(dir_list);
   write_userf(user,"\nTotal number of zombie users = %d\n\n",user_count);

   return 0;
}

int list_normal_users(UR_OBJECT user,int all, char which)
{
   struct dirent **dir_list;
   int file_count=0,cnt=0,user_count=0;

   UR_OBJECT u_loop;
   int timelen;
   int days;

   WriteUserStream *userStream = WriteUserStream::getInstance();

   write_user(user,"\n~OL~FT*** ~FWUser List ~FT***\n\n");
   write_userf(user,"%-*s %-*s Days offline\n\n",USER_NAME_LEN,"Name",8,"Delold");
   if( (file_count = scandir(USERFILES, &dir_list, is_d_file, alphasort)) == -1)
   {
      write_syslogf("Error in list_users() while opening directory '%s' : '%s'.\n",TRUE,USERFILES,strerror(errno));
      return 0;
   }
   pod_string name;
   int dotPos;
   for(cnt=0;cnt<file_count;cnt++)
   {
      name   = dir_list[cnt]->d_name;
      dotPos = name.find('.',0);
      name = name.substr(0,dotPos);

      if( !all && ( which != name[0] ) )
      {
         continue;
      }
      user_count++;
      if(!(u_loop=temp_user_spawn(NULL,(char*)name.c_str(),"view_ulist()")))
      {
         continue;
      }
      /* define (enumerated) subset of things to load, might speed quite much*/
      /* investigate, would need arrays of load/save stuff atleast I guess */
      timelen=(int)(time(0) - u_loop->last_login);
      days=timelen/86400;
      *userStream << addUser( user ) << std::left << std::setw( USER_NAME_LEN ) << name << " "
                                     << std::setw( 8 ) << noyes2[!u_loop->no_delold] << " "
                                     << days << std::endl << pod_send;
      temp_user_destroy(u_loop);
   }
   for(cnt=0;cnt<file_count;cnt++) 
   {
      free(dir_list[cnt]);
   }
   free(dir_list);
   if (all) 
   {
      write_userf(user,"\nTotal number of users = %d\n\n",user_count);
   }
   else 
   {
      write_userf(user,"\nTotal number of %c's = %d\n\n",which,user_count);
   }
   return 0;
}

int list_users(UR_OBJECT user)  
{
   if ((words.word_count<2)||(!strcmp(words.word[1],""))) 
   {                
      write_user(user,"Usage: ulist <letter/-a/-z>\n\n");
      return 0;
   }  
   strtolower(words.word[1]);
   if (!strcmp(words.word[1],"-z"))      list_zombie_users(user);
   else if (!strcmp(words.word[1],"-a")) list_normal_users(user,TRUE,0);
   else                            list_normal_users(user,FALSE,toupper(words.word[1][0]));
   
   return 0;
}

int jail_user(UR_OBJECT user, char *inpstr)
{
   UR_OBJECT u;
   pod_string name;
   WriteRoomStream *roomStream = WriteRoomStream::getInstance();

   if (words.word_count<2)
   {
      write_user(user,"~OLUsage: arrest <user> [reason]\n");
      return 0;
   }
   if ((u=get_user(words.word[1]))!=NULL)
   {
      if (user == u)                    write_user (user,"~OLWhy are you trying to arrest yourself??\n");
      else if (u->level >= user->level) write_user (user,"~OLSorry, you canot arrest a user of greater level than your own.\n");
      else if (u->level == LEV_MIN)     write_userf(user,"~OLThat user is already in %s!\n",globalSpecialRoomNames.getJailRoomName()->get().c_str());
      else
      {
         name = get_visible_name(user);
         write_userf(user,"~FR~OLYou call in two guardian dolphins to arrest %s!\n",u->name);

         *roomStream  << setRoom( NULL ) << addExcept( user ) << addExcept( u )
                     << "~FR~OL" << name << " calls in two guardian dolphins to arrest " << u->name << "!\n"
                     << pod_send;

         write_userf(u,"~FR~OL%s has called in two guardian dolphins to arrest you!\n",name.c_str());

         logStream << setLogfile( SYSLOG ) << user->name << " has arrested " << u->name << ".\n" << pod_send;

         pod_stringstream outputStream;
         if (words.word_count<3)
         {
            outputStream << "user has been arrested, reason: none given\n";
         }
         else
         {
            outputStream << "user has been arrested, reason: " << remove_first(inpstr) << "\n";
         }
         log_user(u,user->name, (char*)outputStream.str().c_str() );
         u->jailed = u->level;
         u->level  = LEV_MIN;
         follow_kill(u,FALSE);
         move_user(u,get_room(globalSpecialRoomNames.getJailRoomName()->get().c_str()),GO_MODE_GIANT_FLIPPER);
      }
      return 0;
   }
   /* User not logged on */
   if( !(u=temp_user_spawn(user,words.word[1],"jail_user")) ) return 0;
   else if (u->level >= user->level) write_user(user,"~OLSorry, you canot arrest a user of greater level than your own.\n");
   else if (u->level == LEV_MIN)     write_userf(user,"~OLThat user is already in %s!\n",globalSpecialRoomNames.getJailRoomName()->get().c_str());
   else
   {
      u->jailed = u->level;
      u->level  = LEV_MIN;
      save_user(u);
      write_userf(user,"~FR~OLYou arrest %s!\n",u->name);
      pod_stringstream outputStream;
      if (words.word_count<3)
      {
         outputStream << "user has been arrested, reason: none given\n";
      }
      else
      {
         outputStream << "user has been arrested, reason: " << remove_first(inpstr) << "\n";
      }
      log_user(u,user->name, (char*)outputStream.str().c_str() );
      logStream << setLogfile( SYSLOG ) << user->name << " has arrested " << u->name << ".\n" << pod_send;
   }
   temp_user_destroy(u);
   return 0;
}

int unjail_user(UR_OBJECT user, char *inpstr)
{
   UR_OBJECT u;
   pod_string name;
   WriteRoomStream *roomStream = WriteRoomStream::getInstance();

   if (words.word_count<2)
   {
      write_user(user,"~OLUsage: release <user> [reason]\n");
      return 0;
   }
   if ( (u=get_user(words.word[1])) )
   {
      if (user==u)                      write_userf(user,"~OLYou cant remove yourself from %s.\n",globalSpecialRoomNames.getJailRoomName()->get().c_str());
      else if (user->level < u->jailed) write_user (user,"~OLSorry, you dont have the power to release this user.\n");
      else if (u->level != LEV_MIN)     write_userf(user,"~OLThat person is not in ~FR%s~FW.\n",globalSpecialRoomNames.getJailRoomName()->get().c_str());
      else
      {
         name = get_visible_name(user);
         write_userf(user,"~FG~OLYou release %s from %s. %s is no longer under arrest.\n",u->name,globalSpecialRoomNames.getJailRoomName()->get().c_str(),u->name);

         *roomStream  << setRoom( NULL ) << addExcept( user ) << addExcept( u )
                      << "~FG~OL" << name << " releases " << u->name << " from "
                      << globalSpecialRoomNames.getJailRoomName()->get()
                      << ". " << u->name
                      << " is no longer under arrest.\n"
                      << pod_send;

         write_userf(u,"~FG~OL%s has released you from %s. You are no longer under arrest. Please obey the rules from now on.\n",name.c_str(),globalSpecialRoomNames.getJailRoomName()->get().c_str());
         logStream << setLogfile( SYSLOG ) << user->name << " has released " << u->name << ".\n" << pod_send;
         pod_stringstream outputStream;
         if (words.word_count<3)
         {
            outputStream << "user has been released, reason: none given\n";
         }
         else
         {
            outputStream << "user has been released, reason: " << remove_first(inpstr) << "\n";
         }
         log_user(u,user->name, (char*)outputStream.str().c_str() );
         u->level  = u->jailed;
         u->jailed = LEV_MIN;
         move_user(u,get_room(globalSpecialRoomNames.getMainRoomName()->get().c_str()),GO_MODE_GIANT_FLIPPER);
      }
      return 0;
   }
/* User not logged on */
   if( !(u=temp_user_spawn(user,words.word[1],"unjail")) ) return 0;
   else if (user->level < u->jailed) write_user(user,"~OLSorry, you dont have the power to release this user.\n");
   else if (u->level != LEV_MIN)     write_userf(user,"~OLThat person is not in ~FR%s~FW.\n",globalSpecialRoomNames.getJailRoomName()->get().c_str());
   else
   {
      u->level  = u->jailed;
      u->jailed = LEV_MIN;
      save_user(u);
      write_userf(user,"~FG~OLYou release %s from %s. %s is no longer under arrest.\n",u->name,globalSpecialRoomNames.getJailRoomName()->get().c_str(),u->name);
      logStream << setLogfile( SYSLOG ) << user->name << " has released " << u->name << ".\n" << pod_send;
      pod_stringstream outputStream;
      if (words.word_count<3)
      {
         outputStream << "user has been released, reason: none given\n";
      }
      else
      {
         outputStream << "user has been released, reason: " << remove_first(inpstr) << "\n";
      }
      log_user(u,user->name, (char*)outputStream.str().c_str() );
   }
   temp_user_destroy(u);
   return 0;
}
         
int wizon(UR_OBJECT user)
{
   char filename[80];

   sprintf(filename,"%s/%s",DATAFILES,WIZLISTFILE);
   switch(more(user,user->socket,filename)) 
   {
      case 0: write_user(user,"There is no Wizlist.\n"); break;
      case 1: user->misc_op=MISC_MORE;
   }

   return 0;
}


/* delete old users of inpstr length or older */
int delold_users(UR_OBJECT user)
{
   UR_OBJECT u_loop;
   DIR *my_dir;
   struct dirent *node;

   int timelen;
   int days=purgedays.get();
   int days2;
   int c=0;
   static const pod_string userFileExtension = ".D.xml";

   logStream << setLogfile( DELOLDLOG ) << noTime << "*** Users removed on " << long_date(2) << " ***\n" << pod_send;

   if(!(my_dir = opendir(USERFILES)))
   {
      write_syslogf("Error in delold_users() while opening directory '%s' : '%s'.\n",TRUE,USERFILES,strerror(errno));
      return 0;
   }
   /* check all the users.. */
   /*   if( !(u=temp_user_spawn(user,word[1],"no_delold")) ) return 0;*/
   pod_string name;
   int dotPos;
   
   while ( (node = readdir(my_dir)) ) 
   {
      name = node->d_name;
      dotPos = name.find('.',0);
  
      if( name.size() < userFileExtension.size() ||
          name.compare( name.size() - userFileExtension.size(), userFileExtension.size(), userFileExtension ) != 0 )
      {
         continue;
      }

      name = name.substr(0,dotPos);

      /* create a user object if user not already logged on */
      if( (u_loop=temp_user_spawn(NULL,(char*)name.c_str(),"delold")) )
      {
         timelen=(int)(time(0) - u_loop->last_login);
         days2=timelen/86400;
         if (days2>days && u_loop->no_delold == 0)
         {
            logStream << setLogfile( DELOLDLOG ) << noTime << USERFILES << "/" << name << userFileExtension << "\n" << pod_send;

            purge_user_files((char*)name.c_str());
            c++;
         }
         temp_user_destroy(u_loop);
      }
   }     
   if(closedir(my_dir)==-1) 
   {
      write_syslogf("Error in delold_users() while closing directory '%s' : '%s'.\n",TRUE,USERFILES,strerror(errno));
      return 0;
   }

   write_userf(user,"\n~OLTotal Number of ~FGDwellers~FW removed:~FB %d\n",c);

   return 0;
}

/*** Switch swearing ban on and off ***/
int swban(UR_OBJECT user)
{
   if ( ban_swearing.set( !(ban_swearing.get()) ) ) 
   {
      write_user(user,"Swearing ban ~FGON.\n");
      write_syslogf("%s switched swearing ban ON.\n",TRUE,user->name);
      return 0;
   }
   write_user(user,"Swearing ban ~FROFF.\n");
   write_syslogf("%s switched swearing ban OFF.\n",TRUE,user->name);
   return 0;
}

/*** Muzzle an annoying user so he cant speak, emote, echo, write, smail
	or bcast. Muzzles have levels from WIZ to GOD so for instance a wiz
     cannot remove a muzzle set by a god.  ***/
int muzzle(UR_OBJECT user)
{
   UR_OBJECT u;

   if (words.word_count<2) 
   {
      write_user(user,"Usage: muzzle <user>\n");  
      return 0;
   }
   if ((u=get_user(words.word[1]))!=NULL) 
   {
      if (u==user)                      write_user(user,"Trying to muzzle yourself is the ninth sign of madness.\n");
      else if (u->level>=user->level)   write_user(user,"You cannot muzzle a user of equal or higher level than yourself.\n");
      else if (u->muzzled>=user->level) write_userf(user,"%s is already muzzled.\n",u->name);
      else
      {
         write_userf(user,"~FR~OL%s now has a muzzle of level: ~RS~OL%s.\n",u->name,getLevelName(user->level));
         write_user(u,"~FR~OLYou have been muzzled!\n");
         write_syslogf("%s muzzled %s.\n",TRUE,user->name,u->name);
         u->muzzled=user->level;
      }
      return 0;
   }
   /* User not logged on */
   if( !(u=temp_user_spawn(user,words.word[1],"muzzle")) ) return 0;
   else if (u->level>=user->level)   write_user(user,"You cannot muzzle a user of equal or higher level than yourself.\n");
   else if (u->muzzled>=user->level) write_userf(user,"%s is already muzzled.\n",u->name);
   else
   {
      u->muzzled=user->level;
      save_user(u);
      write_userf(user,"~FR~OL%s given a muzzle of level: ~RS~OL%s.\n",u->name,getLevelName(user->level));
      send_mail(user,words.word[1],"~FR~OLYou have been muzzled!\n",0);
      write_syslogf("%s muzzled %s.\n",TRUE,user->name,u->name);
   }
   temp_user_destroy(u);
   return 0;
}

/*** Umuzzle the bastard now he's apologised and grovelled enough via email ***/
int unmuzzle(UR_OBJECT user)
{
   UR_OBJECT u;

   if (words.word_count<2)
   {
      write_user(user,"Usage: unmuzzle <user>\n");
      return 0;
   }
   if ((u=get_user(words.word[1]))!=NULL)
   {
      WriteUserStream *userStream = WriteUserStream::getInstance();

      if (u==user)
      {
         write_user(user,"Trying to unmuzzle yourself is the tenth sign of madness.\n");
         return 0;
      }
      if (!u->muzzled)
      {
         *userStream << addUser(user) << u->name << " is not muzzled.\n" << pod_send;
         return 0;
      }
      if (u->muzzled>user->level)
      {
         *userStream << addUser(user) << u->name << "'s muzzle is set to level " << getLevelName(u->muzzled) << ", you do not have the power to remove it.\n" << pod_send;
         return 0;
      }
      *userStream << addUser(user) << u->name << "~FG~OLYou remove " << u->name << "'s muzzle.\n" << pod_send;
      write_user(u,"~FG~OLYou have been unmuzzled!\n");
      logStream << setLogfile( SYSLOG ) << user->name << " unmuzzled " << u->name << ".\n" << pod_send;
      u->muzzled=0;
      return 0;
   }
/* User not logged on */
   if( !(u=temp_user_spawn(user,words.word[1],"unmuzzle")) ) return 0;
   if (u->muzzled>user->level)
   {
      write_userf(user,"%s's muzzle is set to level %s, you do not have the power to remove it.\n",u->name,getLevelName(u->muzzled));
      temp_user_destroy(u);
      return 0;
   }
   u->muzzled=0;
   save_user(u);
   write_userf(user,"~FG~OLYou remove %s's muzzle.\n",u->name);
   send_mail(user,words.word[1],"~FG~OLYou have been unmuzzled.\n",0);
   logStream << setLogfile( SYSLOG ) << user->name << " unmuzzled " << u->name << ".\n" << pod_send;
   temp_user_destroy(u);
   return 0;
}


/*** Promote a user ***/
int promote(UR_OBJECT user)
{
   UR_OBJECT u;
   char text2[80];
   pod_string name;
   WriteRoomStream *roomStream = WriteRoomStream::getInstance();

   if (words.word_count<2)
   {
      write_user(user,"Usage: promote <user>\n");
      return 0;
   }
   /* See if user is on atm */
   if ((u=get_user(words.word[1]))!=NULL)
   {
      if (u->level>=user->level-1l)
      {
         write_user(user,"You cannot promote a user to your own level.\n");
         return 0;
      }
      name=get_visible_name(user);
      u->level++;
      write_userf(user,"~FG~OLYou rake %s giving social status promotion to rank: ~RS~OL%s.\n",u->name,getLevelName(u->level));

      *roomStream  << setRoom( NULL ) << addExcept( user ) << addExcept( u )
                  << "~FG~OL" << name << " rakes " << u->name
                  << " giving social status promotion to rank: ~RS~OL" << getLevelName(u->level)
                  << pod_send;
      
      write_userf(u,"~FG~OL%s has raked you!  You have been given a social status promotion to rank: ~RS~OL%s!\n",name.c_str(),getLevelName(u->level));
      logStream << setLogfile( PROMOLOG ) << "~OL~FY" << name << " PROMOTED " << u->name << " to rank: ~FW" << getLevelName(u->level) << ".\n" << pod_send;

      return 0;
   }
   /* Create a temp session, load details, alter , then save. This is inefficient
      but its simpler than the alternative */
   if( !(u=temp_user_spawn(user,words.word[1],"promote")) ) return 0;
   if (u->level>=user->level-1) 
   {
      write_user(user,"You cannot promote a user your own level.\n");
      temp_user_destroy(u);
      return 0;
   }
   u->level++;  
   save_user(u);
   write_userf(user,"You rake %s promoting %s to rank: ~OL%s.\n",u->name,get_gender(u,"him"),getLevelName(u->level));
   sprintf(text2,"~FG~OLYou have been raked and promoted to rank: ~RS~OL%s.\n",getLevelName(u->level));
   send_mail(user,words.word[1],text2,0);
   logStream << setLogfile( PROMOLOG ) << "~OL~FY" << user->name << " rakes " << words.word[1] << " promoting " << get_gender(u,"him") << " to rank: ~FW" << getLevelName(u->level) << ".\n" << pod_send;

   temp_user_destroy(u);
   return 0;
}

/*** Demote a user ***/
int demote(UR_OBJECT user)
{
   UR_OBJECT u;
   RM_OBJECT rm;
   char text2[80];
   pod_string name;
   WriteRoomStream *roomStream = WriteRoomStream::getInstance();

   if (words.word_count<2)
   {
      write_user(user,"Usage: demote <user>\n");  return 0;
   }
   /* See if user is on atm */
   if ((u=get_user(words.word[1]))!=NULL)
   {
      if (u->level==LEV_MIN || u->level==LEV_ZER)
      {
         write_userf(user,"You cannot demote a user of level %s.\n",getLevelName(u->level));
         return 0;
      }
      if (u->level>=user->level)
      {
         write_user(user,"You cannot demote a user of an equal or higher level than yourself.\n");
         return 0;
      }
      name=get_visible_name(user);
      u->level--;
      write_userf(user,"~FR~OLYou fluke-thwap %s demoting %s social status to rank: ~RS~OL%s.\n",u->name,get_gender(u,"his"),getLevelName(u->level));
      rm=user->room;
      user->room=NULL;

      *roomStream  << setRoom( NULL ) << addExcept( u )
                  << "~FR~OL" << name << " fluke-thwaps " << u->name << " demoting social status to rank: ~RS~OL" << getLevelName(u->level) << ".\n"
                  << pod_send;

      user->room=rm;
      write_userf(u,"~FR~OL%s has fluke-thwapped you giving you social status demotion to rank: ~RS~OL%s!\n",name.c_str(),getLevelName(u->level));
      logStream << setLogfile( PROMOLOG ) << "~OL~FY" << user->name << " DEMOTED " << u->name << " to level: ~FW" << getLevelName(u->level) << ".\n" << pod_send;

      return 0;
   }
/* User not logged on */
   if( !(u=temp_user_spawn(user,words.word[1],"demote")) ) return 0;
   if (u->level==LEV_MIN || u->level==LEV_ZER)
   {
      write_userf(user,"You cannot demote a user of level %s.\n",getLevelName(u->level));
      temp_user_destroy(u);
      return 0;
   }
   if (u->level>=user->level) 
   {
      write_user(user,"You cannot demote a user of an equal or higher level than yourself.\n");
      temp_user_destroy(u);
      return 0;
   }
   u->level--;
   save_user(u);
   write_userf(user,"You demote %s to level: ~OL%s.\n",u->name,getLevelName(u->level));
   sprintf(text2,"~FR~OLYou have been demoted to level: ~RS~OL%s.\n",getLevelName(u->level));
   send_mail(user,words.word[1],text2,0);
   logStream << setLogfile( PROMOLOG ) << "~OL~FY" << user->name << " DEMOTED " << words.word[1] << " to level: ~FW" << getLevelName(u->level) << ".\n" << pod_send;
   temp_user_destroy(u);
   return 0;
}

int tag_user(UR_OBJECT user,char* inpstr)
{
   UR_OBJECT u;

   if (words.word_count<3) 
   {
      write_user(user,"Usage: tag <user> <reason>\n");  
      return 0;
   }
   if ((u=get_user(words.word[1]))!=NULL) 
   {
      if (u==user)                      write_user(user,"Whaaat, you're telling me we should keep an eye on you? :).\n");
      else if (u->level>=user->level)   write_user(user,"You cannot tag a user of equal or higher level than yourself.\n");
      else if (u->tag) write_userf(user,"%s is already tagged.\n",u->name);
      else
      {
         write_userf(user,"~FR~OL%s now is tagged.\n",u->name);
         write_syslogf("%s tagged %s.\n",TRUE,user->name,u->name);
         u->tag=TRUE;
         pod_stringstream outputStream;
         outputStream << "user has been tagged, reason: " << remove_first(inpstr) << "\n";
         log_user(u,user->name, (char*)outputStream.str().c_str() );
      }
      return 0;
   }
   /* User not logged on */
   if( !(u=temp_user_spawn(user,words.word[1],"tag")) ) return 0;
   else if (u->level>=user->level)   write_user(user,"You cannot tag a user of equal or higher level than yourself.\n");
   else if (u->tag) write_userf(user,"%s is already tagged.\n",u->name);
   else
   {
      u->tag=TRUE;
      save_user(u);
      write_userf(user,"~FR~OL%s now is tagged.\n",u->name);
      write_syslogf("%s tagged %s.\n",TRUE,user->name,u->name);
      pod_stringstream outputStream;
      outputStream << "user has been tagged, reason: " << remove_first(inpstr) << "\n";
      log_user(u,user->name, (char*)outputStream.str().c_str() );
   }
   temp_user_destroy(u);
   return 0;
}

int untag_user(UR_OBJECT user,char* inpstr)
{
   UR_OBJECT u;

   if (words.word_count<3)
   {
      write_user(user,"Usage: untag <user> <reason>\n");
      return 0;
   }
   if ((u=get_user(words.word[1]))!=NULL)
   {
      if (u==user)                      write_user(user,"Nope, no can do, you can't untag yourself.\n");
      else if (u->level>=user->level)   write_user(user,"You cannot untag a user of equal or higher level than yourself.\n");
      else if (!u->tag) write_userf(user,"%s is'nt tagged.\n",u->name);
      else
      {
         write_userf(user,"~FR~OL%s is untagged.\n",u->name);
         write_syslogf("%s untagged %s.\n",TRUE,user->name,u->name);
         u->tag=FALSE;
         pod_stringstream outputStream;
         outputStream << "user has been untagged, reason: " << remove_first(inpstr) << "\n";
         log_user(u,user->name, (char*)outputStream.str().c_str() );
      }
      return 0;
   }
   /* User not logged on */
   if( !(u=temp_user_spawn(user,words.word[1],"untag")) ) return 0;
   else if (u->level>=user->level)   write_user(user,"You cannot untag a user of equal or higher level than yourself.\n");
   else if (!u->tag) write_userf(user,"%s is'nt tagged.\n",u->name);
   else
   {
      u->tag=FALSE;
      save_user(u);
      write_userf(user,"~FR~OL%s is untagged.\n",u->name);
      write_syslogf("%s untagged %s.\n",TRUE,user->name,u->name);
      pod_stringstream outputStream;
      outputStream << "user has been untagged, reason: " << remove_first(inpstr) << "\n";
      log_user(u,user->name, (char*)outputStream.str().c_str() );
   }
   temp_user_destroy(u);
   return 0;
}

