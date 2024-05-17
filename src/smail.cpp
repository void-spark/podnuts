#include <string>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include "IoError.h"
#include "XmlParsingError.h"
#include "general_headers.h"
#include "string_misc.h"
#include "cmd_main.h"
#include "StringLibrary.h"
#include "xalloc.h"
#include "more.h"
#include "GenericMessage.h"
#include "MailMessage.h"
#include "MailForward.h"
#include "MailCopyToVar.h"
#include "MailBox.h"
#include "smail.h"
   
pod_string userNameToMailBoxFileName(pod_string input)
{
   pod_ostringstream filenameStream;
   filenameStream << USERFILES << "/" << input << ".M.xml";
   return filenameStream.str();
}

pod_string renderMessageToTelnet( MailMessage *message )
{
   pod_string headerColor = "~OL";
   pod_ostringstream outputStream;

   outputStream << headerColor << "From: " << message->getFrom() << "  [ " << message->getDate() << " ]" << (message->getIsCC() ? " (CC)" : "" ) << std::endl;
   outputStream << message->getBody() << std::endl;

   return outputStream.str();
}

/*
 * count the messages a user has, returns 0 if no messages or error
 */
int mail_count(UR_OBJECT user)
{
   MailBox mailBox( userNameToMailBoxFileName(user->name) );
   try
   {
      mailBox.open(0);
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

   return mailBox.message_count();
}

/*
 * Reads out either specified messages, or new messages.
 */
int read_specific_mail(UR_OBJECT user,MailBox *mailBox)
{
   int cnt,total,smail_number;
   int get_new=FALSE;
   int readSomething = 0;
   std::ostringstream filenameStream;

   if(!strcmp(words.word[1],"new")) get_new=TRUE;
   
   if( words.word_count > 2 ||
       ( !( smail_number = atoi(words.word[1]) ) && !get_new ) )  write_user(user,"Usage: rmail [message #/new]\n");
   else if( !( total = mail_count(user) ) )                       write_user(user,"You currently have no mail.\n");
   else if(get_new)
   {
      write_user(user,"\n~OL~FB*** New mail ***\n\n");
      cnt=1;

      int lastReadValue = mailBox->getLastReadValue();

      while( mailBox->hasNextMessage() )
      {
         MailMessage *message = mailBox->getMessageAndAdvance();

         if (lastReadValue < message->getTime())
         {
            write_user(user,renderMessageToTelnet( message ).c_str());
            cnt++;
         }
      }
      if( cnt <= 1) write_user(user,"You have no new mail.\n\n");
      else readSomething = 1;
   }
   else if( smail_number > total ) write_userf(user,"You only have %d messages in your mailbox.\n",total);
   else
   {
      MailMessage *message = mailBox->getMessage( smail_number - 1 );

      write_user(user,"\n");
      write_user(user,renderMessageToTelnet( message ).c_str());
      write_userf(user,"Message number ~FM~OL%d~RS out of ~FM~OL%d~RS.\n\n",smail_number,total);

      readSomething = 1;
   }
   if(readSomething)
   {
      mailBox->setLastReadValue( time(0) );
      mailBox->save();
   }
   return 0;
}


/*
 * Read your mail
 */
int rmail(UR_OBJECT user)
{
   int ret;

   MailBox mailBox( userNameToMailBoxFileName(user->name) );
   try
   {
      mailBox.open(0);
   }
   catch( NoSuchFile e)
   {
      write_user(user,"You have no mail.\n");
      return 0;
   }
   catch( PodRuntimeError e)
   {
      write_syslogf( "%s\n",false,e.what() );
      write_user(user,"You have no mail.\n");
      return 0;
   }

   if (words.word_count > 1)
   {
      return read_specific_mail(user,&mailBox);
   }

   pod_ostringstream filenameStream;
   filenameStream << mailBox.getFileName() << "." << user->name << ".tempfile";

   std::ofstream outfile;
   outfile.open ( filenameStream.str().c_str() );
   if( outfile.fail() )
   {
      #warning needs testing on errs
   }
   else
   {
      while( mailBox.hasNextMessage() )
      {
         outfile << renderMessageToTelnet( mailBox.getMessageAndAdvance() );
      }
   }
   outfile.close();

   mailBox.setLastReadValue( time(0) );
   mailBox.save();

   write_user(user,"\n~OL~FB*** Your mail ***\n\n");

   ret=more(user,0,filenameStream.str());
   if (ret==1) user->misc_op=MISC_MORE;
   return 0;
}

/*
 * Check if user has new mail.
 */
int has_unread_mail(UR_OBJECT user)
{
   MailBox mailBox( userNameToMailBoxFileName(user->name) );
   try
   {
      mailBox.open(0);
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
   int lastMessageTime = 0;

   while( mailBox.hasNextMessage() )
   {
      lastMessageTime = mailBox.getMessageAndAdvance()->getTime();
   }

   if ( mailBox.getLastReadValue() < lastMessageTime )
   {
      return 1;
   }

   return 0;
}

int doesFileExist(pod_string filename)
{
   std::ifstream CheckFile;

   CheckFile.open(filename.c_str());
   if (CheckFile.fail())
   {
      return 0;
   }
   else
   {
      CheckFile.close();
      return 1;
   }
}

int send_level_mail( UR_OBJECT user, pod_string to, char *ptr)
{
   struct dirent **dir_list;
   int file_count=0,cnt=0,user_count=0;
   UR_OBJECT u_loop;
   int level = get_level( to.c_str() );

   if(level < 0 || level >= get_level("BOT") )
   {
      return 1;
   }

   write_userf(user,"Attempting to send smail to all %s level or higher users...\n\n",getLevelName(level) );

   if( (file_count = scandir(USERFILES, &dir_list, is_d_file, alphasort)) == -1)
   {
      write_syslogf("Error while opening directory '%s' : '%s'.\n",TRUE,USERFILES,strerror(errno));
      return 0;
   }
   pod_string name;   
   int dotPos;
   for( cnt=0; cnt < file_count; cnt++)
   {
      name   = dir_list[cnt]->d_name;
      dotPos = name.find('.',0);
      name = name.substr(0,dotPos);   
      if( !( u_loop = temp_user_spawn( NULL, (char*)name.c_str(), "send_level_mail()" ) ) )
      {
         continue;
      }
      if(u_loop->level < level || u_loop == user)
      {
         continue;
      }
      user_count++;

      send_mail(user,u_loop->name,ptr,1);
      temp_user_destroy(u_loop);
   }
   for( cnt = 0; cnt<file_count; cnt++)
   {
      free(dir_list[cnt]);
   }
   free(dir_list);
   write_userf(user,"\nTotal number of users sent to = %d\n\n",user_count);
   return 0;
}
/*
 * Send mail message
 */
int smail(UR_OBJECT user, char *inpstr, int done_editing)
{
   StringLibrary * stringLibrary = StringLibrary::getInstance();
   StrGlobalVar *mail_to = (StrGlobalVar*)user->getVar("mail_to");
   int level;

   if (done_editing)
   {
      if (*user->malloc_end--!='\n') *user->malloc_end--='\n';

      if( send_level_mail( user, mail_to->get(), user->malloc_start) )
      {
         send_mail(user,mail_to->get(),user->malloc_start,0);
      }
      user_get_copyto(user)->send(user,user->malloc_start);
      mail_to->set("");
      return 0;
   }
   if (words.word_count < 2)
   {
     write_user(user,"Smail who?\n");
     return 0;
   }
   words.word[1][0] = toupper(words.word[1][0]);

   /* See if user exists */

   level = get_level( words.word[1] );
   if(level >= 0 && level < get_level("BOT") )
   {
      if( user->level < LEV_FOU)
      {
         write_user_crt(user,stringLibrary->makeString("nosuchuser").c_str());
         return 0;
      }
   }
   else
   {
      UR_OBJECT u = NULL;
      if ( !( u=get_user_exactmatch(words.word[1]) ) )
      {
         pod_ostringstream filenameStream;
         filenameStream << USERFILES << "/" << words.word[1] <<".D.xml";

         if(!doesFileExist(filenameStream.str()))
         {
            write_user_crt(user,stringLibrary->makeString("nosuchuser").c_str());
            return 0;
         }
      }
      if (u==user && user->level<LEV_ONE)
      {
        write_user(user,"Trying to mail yourself is the fifth sign of madness.\n");
        return 0;
      }
      if ( u!=NULL )
      {
        strcpy(words.word[1],u->name);
      }
   }
   if (words.word_count > 2)
   {
#warning this all could be done neater, notice the  (char*)myBuffer.c_str()
      pod_string myBuffer;
      myBuffer = inpstr;
      myBuffer += "\n";
      if( send_level_mail( user, words.word[1], remove_first((char*)myBuffer.c_str()) ) )
      {
         send_mail( user, words.word[1], remove_first((char*)myBuffer.c_str()), 0 );
      }
      user_get_copyto(user)->send(user,remove_first((char*)myBuffer.c_str()));
      return 0;
   }
   write_userf(user,"\n~OL~FB*** Writing mail message to %s ***\n\n",words.word[1]);
   user->misc_op=MISC_MAIL;
   mail_to->set(words.word[1]);
   editor(user,NULL);
   return 0;
}

/*
 * This is function that sends mail to other users
 * user   from    gen_from
 * NULL   NULL    "MAILDEAMON" , send_mail with user = NULL, system mail, messages only sent to syslog
 * VAL    NULL    user->name   , regular send_mail, messages sent to user and syslog
 * NULL   VAL     from = email , received external message, messages sent to syslog only
 * VAL    VAL      N/A         , <ERROR!>
 */

int send_mail_ex(UR_OBJECT user, char *from, pod_string to, pod_string body, int iscopy)
{
   StringLibrary * stringLibrary = StringLibrary::getInstance();
   UR_OBJECT u;

   if(!user && !from)
   {
      from="MAILDAEMON";
   }
   else if(user)
   {
      from=user->name;
   }

   MailBox mailBox( userNameToMailBoxFileName(to) );
   try
   {
      mailBox.open(MailBox::CREATE);
   }
   catch( PodRuntimeError e)
   {
      write_syslogf( "%s\n",false,e.what() );
      write_userf(user,"%s: cannot parse file.\n",stringLibrary->makeString("syserror").c_str());
      return 0;
   }

   pod_string wrappedBody = wordWrap( body, 80 );
   MailMessage *message = new MailMessage( from,wrappedBody,iscopy );
   mailBox.insertMessage(message);
//   mailBox.insertMessage(from,body,iscopy);
   mailBox.save();

   if(!iscopy)
   {
      if(user) write_userf(user,"Mail is delivered to %s\n",to.c_str() );
      write_syslogf("%s sent mail to %s\n",TRUE,from,to.c_str() );
   }
   else
   {
      if(user) write_userf(user,"Mail is copied to %s\n",to.c_str() );
      write_syslogf("%s sent a copy of mail to %s.\n",TRUE,from,to.c_str() );
   }
   #warning EVIL!
   char* temp_to = (char*)to.c_str();
   if ( (u=get_user_exactmatch( temp_to )) )
   {
      write_user(u,"\07~OL~FR-- MAILDAEMON : ~LIYou have new mail!\n");
   }
   forward_email( temp_to, message );

   return 1;
}

int send_mail(UR_OBJECT user, pod_string to, char *ptr, int iscopy)
{
   return send_mail_ex(user, NULL, to, ptr, iscopy);
}


/*
 * Delete some or all of your mail.
 */
int dmail(UR_OBJECT user)
{
   int *parameter_list;
   int cnt,num;

   if (words.word_count<2) 
   {
      write_user(user,"Usage: dmail all\n");
      write_user(user,"Usage: dmail <#><-#> <#><-#> ...\n");
      return 0;
   }

   if(!( parameter_list = get_wipe_parameters(user,(num=mail_count(user))) )) return 0;

   if (parameter_list[0]==-1) 
   {
      write_user(user,"\n\07~FR~OL~LI*** WARNING - This will delete all your smail! ***\n\nAre you sure about this (y/n)? ");
      user->misc_op=MISC_DEL_MAIL;
      no_prompt=1;
   }
   else
   {
      MailBox mailBox( userNameToMailBoxFileName( user->name ) );
      try
      {
         mailBox.open(0);
      }
      catch( NoSuchFile e)
      {
         write_user(user,"You have no mail to delete.\n");
         xfree(parameter_list);
         return 0;
      }
      catch( PodRuntimeError e)
      {
         write_syslogf( "%s\n",false,e.what() );
         write_user(user,"You have no mail to delete.\n");
         xfree(parameter_list);
         return 0;
      }
      cnt = mailBox.wipeMessages(parameter_list);


      if (cnt==num)
      {
         mailBox.wipeAllMessages();
         write_user(user,"All mail messages deleted.\n");
      }
      else
      {
         mailBox.save();
         write_userf(user,"%d mail messages deleted.\n",cnt);
      }
   }
   xfree(parameter_list);
   return 0;
}


/*
 * Show list of people your mail is from without seeing the whole lot
 */
int mail_from(UR_OBJECT user)
{
   int cnt=0;

   MailBox mailBox( userNameToMailBoxFileName(user->name) );
   try
   {
      mailBox.open(0);
   }
   catch( NoSuchFile e)
   {
      write_user(user,"~OL~FT-- MAILDAEMON : You have no mail!\n");
      return 0;
   }
   catch( PodRuntimeError e)
   {
      write_syslogf( "%s\n",false,e.what() );
      write_user(user,"~OL~FT-- MAILDAEMON : You have no mail!\n");
      return 0;
   }

   write_user(user,"\n~OL~FT-- MAILDAEMON : Mail from...\n\n");

   while( mailBox.hasNextMessage() )
   {
      MailMessage *message = mailBox.getMessageAndAdvance();
      cnt++;
      write_userf(user,"#%02d :  %s  [ %s ]%s\n",cnt,message->getFrom().c_str(),message->getDate().c_str(),message->getIsCC() ? " (CC)" : "");
   }

   write_userf(user,"\nTotal of %d messages.\n\n",cnt);
   return 0;
}
