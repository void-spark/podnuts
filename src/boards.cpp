#include <stdio.h>
#include <fstream>
#include <sstream>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "IoError.h"
#include "XmlParsingError.h"
#include "general_headers.h"
#include "StringLibrary.h"
#include "string_misc.h"
#include "more.h"
#include "xalloc.h"
#include "time_utils.h"
#include "Board.h"
#include "boards.h"

extern LimitedIntGlobalVar mesg_life;

pod_string boardNameToBoardFileName(pod_string input)
{
   pod_ostringstream filenameStream;
   filenameStream << BOARDFILES << "/" << input << ".xml";
   return filenameStream.str();
}

pod_string renderMessageToTelnet( BoardMessage *message )
{
   pod_string headerColor = "~OL~FB";
   pod_ostringstream outputStream;

   outputStream << headerColor << "From: " << message->getFrom() << "  [ " << message->getDate() << " ]" /* << (carbonCopy ? " (CC)" : "" )*/ << std::endl;
   outputStream << message->getBody() << std::endl;

   return outputStream.str();
}

/*
 * count number of messages on a news box
 */
int news_count(pod_string board_name)
{
   Board board( boardNameToBoardFileName(board_name) );
   try
   {
      board.open(0);
   }
   catch( NoSuchFile e)
   {
      return 0;
   }
   catch( PodRuntimeError e)
   {
      write_syslogf( "%s\n",false,e.what() );
      return 0;
   }
   return board.message_count();
}
   
/*
 * Write on the message board
 */
int write_board(UR_OBJECT user, char *inpstr, int done_editing)
{
   StringLibrary * stringLibrary = StringLibrary::getInstance();
   RM_OBJECT rm=user->room;
   pod_string body = "";

   int flag=0;
   pod_string name;

   if (!done_editing)
   {
      if (words.word_count<2) 
      {
         rm=user->room;
         write_userf(user,"\n~OL~FB*** Sonar-blasting board message for %s ***\n\n",user->room->name);
         strcpy(user->boardwrite,user->room->name);
         user->misc_op=MISC_BOARD;
         editor(user,NULL);
         return 0;
      }
      if(words.word_count==2) 
      {
         if (!strncmp(ADMINBOARD,words.word[1],strlen(words.word[1]))) 
         {
            if(user->level>=LEV_THR)
            {
               words.word[1]=ADMINBOARD;
               flag=1;
            }
         }
         else if (!strncmp(NEWSBOARD,words.word[1],strlen(words.word[1]))) 
         {
            if(user->level>=LEV_THR)
            {
               words.word[1]=NEWSBOARD;
               flag=1;
            }
         }
         else if (!strncmp(UPGRADESBOARD,words.word[1],strlen(words.word[1]))) 
         {
            if(user->level >= LEV_THR)
            {
               words.word[1]=UPGRADESBOARD;
               flag=1;
            }
         }
         else if(user->level >= LEV_FOU)
         {
            if ((rm=get_room(words.word[1]))!=NULL) 
            {  
               words.word[1] = rm->name;
               flag=1;
            }
         }
      }
      if(flag!=0)
      {
         write_userf(user,"\n~OL~FB*** Sonar-blasting a message for %s ***\n\n",words.word[1]);
         strcpy(user->boardwrite,words.word[1]);
         user->misc_op=MISC_BOARD;
         editor(user,NULL);
         return 0;
      }
      strcpy(user->boardwrite,user->room->name);

      body = inpstr;
      body += '\n';
   }
   else 
   {
      body=user->malloc_start;
   }
   Board board( boardNameToBoardFileName(user->boardwrite) );

   try
   {
      board.open(Board::CREATE);
   }
   catch( PodRuntimeError e)
   {
      write_userf(user,"%s: error opening boardfile.\n",stringLibrary->makeString("syserror").c_str());
      write_syslogf( "%s\n",false,e.what() );
      return 0;
   }

   name = get_visible_name(user);
   board.insertMessage(name,body);
   board.save();

   write_userf(user,"You sonar-blast a message on the board %s..",user->boardwrite);
   if (!strcmp(user->boardwrite,user->room->name)) 
   {
      write_room_exceptf(user->room,user,"%s sonar-blasts a message on the board.\n",name.c_str());
   }
   write_user(user,"...Message Stored.\n");
   if ((rm=get_room(user->boardwrite))!=NULL)
   {
      rm->mesg_cnt++;
   }
   return 0;
}

/*** Read the message board ***/
int read_board(UR_OBJECT user)
{
   StringLibrary * stringLibrary = StringLibrary::getInstance();
   RM_OBJECT rm;
   pod_ostringstream filenameStream;
   pod_string name;
   int ret,flag=0;

   if (words.word_count<2)
   {
      rm=user->room;
      strcpy(user->boardwrite,user->room->name);
   }
   else
   {
      if(user->level<LEV_TWO)
      {
         write_user(user,"Sorry, only member and up can view other boards then main.\n\n");
         return 0;
      }
      if ((rm=get_room(words.word[1]))==NULL)
      {
         if (!strncmp(ADMINBOARD,words.word[1],strlen(words.word[1])))
         {
            if(user->level<LEV_THR)
            {
               write_user(user,"Sorry, you do not have access to that board.\n\n");
               return 0;
            }
            words.word[1]=ADMINBOARD;
            flag=1;
         }
         else if (!strncmp(NEWSBOARD,words.word[1],strlen(words.word[1])))
         {
            words.word[1]=NEWSBOARD;
            flag=1;
         }
         else if (!strncmp(UPGRADESBOARD,words.word[1],strlen(words.word[1])))
         {
            words.word[1]=UPGRADESBOARD;
            flag=1;
         }
      }
      else
      {
         words.word[1]=rm->name;
         flag=1;
         if (!has_room_access(user,rm))
         {
            write_user(user,"That room is currently private, you cannot read the board.\n");
            return 0;
         }
      }
      if (flag==0)
      {
         if (user->level<LEV_FOU)
         {
            write_userf(user,"%s\n",stringLibrary->makeString("nosuchroom").c_str());
            return 0;
         }
         else
         {
            write_user(user,"Sorry, that board does not exist.\n\n");
            return 0;
         }
      }
      strcpy(user->boardwrite,words.word[1]);
   }
   write_userf(user,"\n~OL~FR*** The %s message board ***\n\n",user->boardwrite);

   name=get_visible_name(user);

   Board board( boardNameToBoardFileName(user->boardwrite) );
   try
   {
      board.open(0);
   }
   catch( NoSuchFile e)
   {
      write_user(user,"There are no messages on the board.\n\n");
      if (!strcmp(user->room->name,user->boardwrite)) write_room_exceptf(user->room,user,"%s scans the message board.\n",name.c_str());
      return 0;
   }
   catch( PodRuntimeError e)
   {
      write_syslogf( "%s\n",false,e.what() );
      write_user(user,"There are no messages on the board.\n\n");
      if (!strcmp(user->room->name,user->boardwrite)) write_room_exceptf(user->room,user,"%s scans the message board.\n",name.c_str());
      return 0;
   }

   filenameStream << board.getFileName() << "." << user->name << ".tempfile";
   std::ofstream outfile;

   outfile.open ( filenameStream.str().c_str() );
   if( outfile.fail() )
   {
      #warning needs testing on errs
   }
   else
   {
      while( board.hasNextMessage() )
      {
         outfile << renderMessageToTelnet( board.getMessageAndAdvance() );
      }
   }
   outfile.close();

   if (!(ret=more(user,user->socket,filenameStream.str())))
   write_user(user,"There are no messages on the board.\n\n");
   else if (ret==1) user->misc_op=MISC_MORE;
   if (!strcmp(user->room->name,user->boardwrite)) write_room_exceptf(user->room,user,"%s scans the message board.\n",name.c_str());
   return 0;
}

/*** Wipe some messages off the board ***/
int wipe_board(UR_OBJECT user,char *inpstr)
{
   int* parameter_list;
   int num,cnt,flag=0;
   pod_string name;
   RM_OBJECT rm;

   if (words.word_count<2)
   {
      if(user->level<LEV_THR)
      {
         write_user(user,"Usage: wipe all\n");
         write_user(user,"Usage: wipe <#><-#> <#><-#> ...\n");
         return 0;
      }
      else
      {
         write_user(user,"Usage: wipe [board name] all\n");
         write_user(user,"Usage: wipe [board name] <#><-#> <#><-#> ...\n");
         return 0;
      }
   }

   if (!strncmp(ADMINBOARD,words.word[1],strlen(words.word[1])) && user->level>=LEV_THR)
   {
      strcpy(user->boardwrite,ADMINBOARD);
      flag=1;
   }
   else if (!strncmp(NEWSBOARD,words.word[1],strlen(words.word[1])) && user->level>=LEV_THR)
   {
      strcpy(user->boardwrite,NEWSBOARD);
      flag=1;
   }
   else if (!strncmp(UPGRADESBOARD,words.word[1],strlen(words.word[1])) && user->level>=LEV_THR)
   {
      strcpy(user->boardwrite,UPGRADESBOARD);
      flag=1;
   }
   else if (((rm=get_room(words.word[1]))!=NULL) && (user->level>=LEV_FOU) )
   {
      strcpy(user->boardwrite,rm->name);
      flag=1;
   }
   else
   {
      rm=user->room;
      strcpy(user->boardwrite,rm->name);
   }

   if (flag)
   {
      if(words.word_count<3)
      {
         write_user(user,"Usage: wipe [board name] all\n");
         write_user(user,"Usage: wipe [board name] <#><-#> <#><-#> ...\n");
         return 0;
      }
      wordfind(inpstr);
   }

   Board board( boardNameToBoardFileName(user->boardwrite) );

   try
   {
      board.open(0);
   }
   catch( NoSuchFile e)
   {
      write_user(user,"The message board is empty.\n");
      return 0;
   }
   catch( PodRuntimeError e)
   {
      write_syslogf( "%s\n",false,e.what() );
      write_user(user,"The message board is empty.\n");
      return 0;
   }
   num=board.message_count();

   if(!( parameter_list = get_wipe_parameters(user,num) ))   return 0;

   name=get_visible_name(user);
   if (parameter_list[0]==-1) 
   {
      board.wipeAllMessages();
      write_userf(user,"All messages erased from board %s.\n",user->boardwrite);
      if (!strcmp(user->boardwrite,user->room->name)) write_room_exceptf(user->room,user,"%s grabs ahold of a shark and uses its rough skin to erase the board.\n",name.c_str());
      write_syslogf("%s wiped all messages from the board %s.\n",TRUE,user->name,user->boardwrite);
      if ((rm=get_room(user->boardwrite))!=NULL)	rm->mesg_cnt=0;
      xfree(parameter_list);
      return 0;
   }

   cnt = board.wipeMessages(parameter_list);

   xfree(parameter_list);

   if (cnt==num) 
   {
      board.wipeAllMessages();
      write_user(user,"All messages deleted.\n");
      if ((rm=get_room(user->boardwrite))!=NULL) rm->mesg_cnt=0;
      write_syslogf("%s wiped all messages from the board %s.\n",TRUE,user->name,user->boardwrite);
   }
   else 
   {
      board.save();
      write_userf(user,"%d messages deleted.\n",cnt);
      if ((rm=get_room(user->boardwrite))!=NULL) rm->mesg_cnt-=cnt;
      write_syslogf("%s wiped %d messages from the board %s.\n",TRUE,user->name,cnt,user->boardwrite);
   }
   if (!strcmp(user->boardwrite,user->room->name))
   {
      write_room_exceptf(user->room,user,"%s grabs ahold of a shark and uses its rough skin to erase some messages from the board.\n",name.c_str());
   }
   return 0;
}

/*
 * Search all the boards for the words given in the list. Rooms fixed to
 * private will be ignore if the users level is less than gatecrash_level
 */
int search_boards(UR_OBJECT user)
{
   RoomsVector::iterator roomNode;
   pod_string filename;
   int w,cnt,room_given;

   if (words.word_count<2)
   {
      write_user(user,"Usage: search <word list>\n");
      return 0;
   }
   cnt=0;

   for(roomNode=roomsList.begin();roomNode != roomsList.end();roomNode++)
   {
      Board board( boardNameToBoardFileName((*roomNode)->name) );

      try
      {
         board.open(0);
      }
      catch( NoSuchFile e)
      {
         return 0;
      }
      catch( PodRuntimeError e)
      {
         write_syslogf( "%s\n",false,e.what() );
         return 0;
      }

      if (!has_room_access(user,(*roomNode)))
      {
         continue;
      }

      room_given=0;

      BoardMessage *thisMessage;
      while( board.hasNextMessage() )
      {
         thisMessage = board.getMessageAndAdvance();
         for(w=1;w<words.word_count;++w)
         {
            if(thisMessage->getBody().find(words.word[w]) != pod_string::npos)
            {
               if (!room_given)
               {
                  write_userf(user,"~BB*** %s ***\n\n",(*roomNode)->name);
                  room_given=1;
               }
               write_userf(user,renderMessageToTelnet(thisMessage).c_str() );
               cnt++;
            }
         }
      }
   }
   if (cnt) write_userf(user,"Total of %d matching messages.\n\n",cnt);
   else write_user(user,"No occurences found.\n");
   return 0;
}


/*
 * Remove any expired messages from boards unless force = 2 in which case just do a recount.
 */
int check_messages(UR_OBJECT user, int force)
{
   RoomsVector::iterator roomNode;
   pod_string filename;
   int board_cnt,old_cnt,bad_cnt,old_count;
   static int done=0;
   struct tm *tm_struct;
   tm_struct = get_current_localtime();

   switch(force) 
   {
      case 0:
         if ( mesg_check_time.getHour() == tm_struct->tm_hour &&
              mesg_check_time.getMin()  == tm_struct->tm_min )
         {
            if (done) return 0;
         }
         else
         {
            done=0;
            return 0;
         }
         break;

      case 1:
         printf("-- BOOT : Checking boards...\n");
   }
   done=1;
   board_cnt=0;
   old_cnt=0;
   bad_cnt=0;
 
   for(roomNode=roomsList.begin();roomNode != roomsList.end();roomNode++)
   {
      Board board( boardNameToBoardFileName((*roomNode)->name) );
      try
      {
         board.open(0);
      }
      catch( NoSuchFile e)
      {
         continue;
      }
      catch( PodRuntimeError e)
      {
         write_syslogf( "%s\n",false,e.what() );
         return 0;
      }

      board_cnt++;

      old_count = (*roomNode)->mesg_cnt;
      if (force != 2)
      {
         old_cnt = board.deleteOldMessages(mesg_life.get());
      }
      (*roomNode)->mesg_cnt = board.message_count();
      if ((*roomNode)->mesg_cnt != old_count) bad_cnt++;
   }

   switch(force)
   {
      case 0:
         if (bad_cnt) write_syslogf("CHECK_MESSAGES: %d files checked, %d had an incorrect message count, %d messages deleted.\n",TRUE,board_cnt,bad_cnt,old_cnt);
         else write_syslogf("CHECK_MESSAGES: %d files checked, %d messages deleted.\n",TRUE,board_cnt,old_cnt);
      break;

      case 1:
         printf("          %d board files checked, %d out of date messages found.\n",board_cnt,old_cnt);
      break;

      case 2:
         write_userf(user,"%d board files checked, %d had an incorrect message count.\n",board_cnt,bad_cnt);
         write_syslogf("%s forced a recount of the message boards.\n",TRUE,user->name);
   }
   return 0;
}
