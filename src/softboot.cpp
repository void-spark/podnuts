#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "general_headers.h"
#include "globals.h"
#include "macro.h"
#include "ignore.h"
#include "loadsave_user.h"
#include "tandem.h"
#include "hist.h"
#include "string_misc.h"
#include "file_io.h"
#include "xalloc.h"
#include "softboot.h"

TimeTGlobalVar firstBootTime("first_boot_time", FIRST_BOOT, time(0) );
IntGlobalVar   total_logins( "total_logins",    FIRST_BOOT, 0 );

struct UserLink
{
   pod_string name; /* name of person to link to */
   UR_OBJECT *ptr;
};

typedef std::vector<UserLink, pod_alloc< UserLink >::Type > UserLinksVector;
UserLinksVector userLinks;

FILE *logfp;

int unpoof_system_info()
{
   pod_string      filename         = SYSTMPINFO;
   bool            inGlobalsElement = false;
   bool            hasRead          = false;
   XmlTextReader * reader           = 0;
   int             type             = 0;
   pod_string      name;

   try
   {
      reader = new XmlTextReader( filename );   
   }
   catch (XmlError &e)
   {
      std::cerr << "Unable to open " << filename << std::endl;
      
      return 1;
   }
      
   hasRead = reader->Read();
   while( hasRead )
   {
      type = reader->NodeType();
   
      if( type == XML_READER_TYPE_ELEMENT || type == XML_READER_TYPE_END_ELEMENT )
      {
         name = reader->Name();
      
         if( type == XML_READER_TYPE_ELEMENT )
         {
            if( name == "globals" )
            {
               inGlobalsElement = true;
               globVarsFromXML( reader );
            }
         }
         else if( type == XML_READER_TYPE_END_ELEMENT )
         {
            if( name == "globals" && inGlobalsElement == true )
            {
               inGlobalsElement = false;
            }
         }
      }

      hasRead = reader->Read();
   }
   
   delete reader;   
   
   unlink( filename.c_str() );

   return 1;
}

int unpoof_rooms()
{
   pod_string      filename       = ROOMTMPINFO;
   RM_OBJECT       currentRoom    = 0;
   bool            inRoomsElement = false;
   bool            inRoomElement  = false;
   bool            hasRead        = false;
   XmlTextReader * reader         = 0;
   int             type           = 0;
   pod_string      name;   
   
   try
   {
      reader = new XmlTextReader( filename );   
   }
   catch (XmlError &e)
   {
      std::cerr << "Unable to open " << filename << std::endl;      
      return 1;
   }
      
   hasRead = reader->Read();
   while( hasRead )
   {
      type = reader->NodeType();
      
      if( type == XML_READER_TYPE_ELEMENT || type == XML_READER_TYPE_END_ELEMENT )
      {
         name = reader->Name();
         
         if( type == XML_READER_TYPE_ELEMENT )
         {
            if( name == "rooms" )
            {
               inRoomsElement = true;
            }
            else if( name == "room" && inRoomsElement == true)
            {
               inRoomElement = true;
               
               try
               {
                  currentRoom = get_room( reader->GetAttribute( "name" ).c_str() );
                  currentRoom->fromXML( reader );
               }
               catch (NoSuchAttribute &e)
               {
                  std::cerr << filename << " : room with no name!" << std::endl;
                  abort();
               }                              
            }
         }
         else if( type == XML_READER_TYPE_END_ELEMENT )
         {
            if( name == "rooms" && inRoomsElement == true )
            {
               inRoomsElement = false;
            }
            else if( name == "room" && inRoomElement == true )
            {
               inRoomElement = false;
            }
         }
      }

      hasRead = reader->Read();
   }
   
   delete reader;   

   unlink( filename.c_str() );

   return 1;
}

void set_user_link(struct user_struct **target, pod_string name )
{
   struct UserLink node;

   node.name = name;
   node.ptr  = target;
   userLinks.push_back( node );
}

int unpoof_users_info()
{
   pod_string      filename       = USRTMPINFO;
   UR_OBJECT       currentUser    = 0;
   bool            inUsersElement = false;
   bool            inUserElement = false;
   bool            hasRead        = false;
   XmlTextReader * reader         = 0;
   int             type           = 0;
   pod_string      name;   
   
   try
   {
      reader = new XmlTextReader( filename );   
   }
   catch (XmlError &e)
   {
      std::cerr << "Unable to open " << filename << std::endl;      
      return 1;
   }
      
   hasRead = reader->Read();
   while( hasRead )
   {
      type = reader->NodeType();

      if( type == XML_READER_TYPE_ELEMENT || type == XML_READER_TYPE_END_ELEMENT )
      {
         name = reader->Name();

         if( type == XML_READER_TYPE_ELEMENT )
         {
            if( name == "users" )
            {
               inUsersElement = true;
            }
            else if( name == "user" && inUsersElement == true)
            {
               inUserElement = true;
               int        userTypeInt = 0;
               pod_string userNameString;
               
               try
               {
                  userTypeInt = stringToInt( reader->GetAttribute( "type" ).c_str() );
               }
               catch (NoSuchAttribute &e)
               {
                  std::cerr << filename << " : user with no type!" << std::endl;
                  abort();
               }                              
               
               try
               {
                  userNameString = reader->GetAttribute( "name" );
               }
               catch (NoSuchAttribute &e)
               {
                  std::cerr << filename << " : user with no name!" << std::endl;
                  abort();
               }                              
                                             
               std::cout << "user type : " << userTypeInt << std::endl;
               std::cout << "user name : " << userNameString << std::endl;

               currentUser = create_user();
               if( currentUser == 0 )
               {
                  write_syslogf("*** BOOT failed softboot, create_user() failed\n",FALSE);
                  abort();
               }

               if( userNameString.length() > USER_NAME_LEN )
               {
                  write_syslogf("*** BOOT failed softboot, name too long\n",FALSE);
                  abort();
               }
               strcpy( currentUser->name, userNameString.c_str() );

               currentUser->type = userTypeInt;

               if( currentUser->type == CLONE_TYPE )
               {
                  currentUser->owner  = get_user(currentUser->name);
                  currentUser->socket = currentUser->owner->socket;

                  StrGlobalVar *desc     = (StrGlobalVar*)currentUser->getVar("desc");
                  desc->set("~BR(CLONE)");
                  currentUser->fromXMLSoft( reader );
               }
               else
               {
                  if ( !load_user( currentUser ) )
                  {
                     destruct_user( currentUser );
                     write_syslogf("*** BOOT failed softboot, load_user() (%s) failed\n", FALSE, currentUser->name );
                     abort();
                  }

                  pod_string userFilename;
                  ( ( ( userFilename = USERFILES ) += "/" ) += currentUser->name ) += ".A";

                  currentUser->macrolist.loadMacros( (char*)userFilename.c_str() );
                  currentUser->login=0;

                  currentUser->fromXMLSoft( reader );
               }
            }
         }
         else if( type == XML_READER_TYPE_END_ELEMENT )
         {
            if( name == "users" && inUsersElement == true )
            {
               inUsersElement = false;
            }
            else if( name == "user" && inUserElement == true )
            {
               inUserElement = false;
               currentUser = 0;
            }
         }
      }

      hasRead = reader->Read();
   }
   
   delete reader;   

   unlink( filename.c_str() );
   
   return 1;
}

void build_user_links()
{
   UserLinksVector::iterator userLinksNode;

   for( userLinksNode =  userLinks.begin();
        userLinksNode != userLinks.end();
        userLinksNode++ )
   {
      *(userLinksNode->ptr) = get_user( userLinksNode->name );
   }
}

int softboot_restart()
{	

   FILE *fp;
   pid_t old_pid;   

   logStream  << setLogfile( BOOTLOG ) << noTime << "-- SOFTBOOT : Checking if we should softboot.\n" << pod_send;

   if(!(fp = fopen(SOFT_BOOT_PID_FILE,"r"))) /* if no softboot */
   {
      logStream  << setLogfile( BOOTLOG ) << noTime << "-- SOFTBOOT : Nope, going on with a normal boot.\n" << pod_send;
      unlink(USRTMPINFO); /* just in case old softboot files */
      unlink(ROOMTMPINFO);
      unlink(SYSTMPINFO);
      unlink("temp.softbootlog");
      return 0;
   }

   fscanf(fp,"%i\n",&old_pid); 

   fclose(fp);

   if(getpid() != old_pid) /* uhoh, tried to softboot, went kinda wrong ;) */
   {
      rename(SOFT_BOOT_PID_FILE, "softboot_crash.pid");
      rename(USRTMPINFO,         "softboot_crash.users.xml");
      rename(ROOMTMPINFO,        "softboot_crash.rooms.xml");
      rename(SYSTMPINFO,         "softboot_crash.system.xml");
      rename("temp.softbootlog", "softboot_crash.log");

      write_syslogf("*** BOOT failed softboot, pid doesn't match\n",FALSE);
      return 0;
   }
   unlink(SOFT_BOOT_PID_FILE);
   
   printf("-- BOOT : Softboot, initiated\n");
   logStream  << setLogfile( BOOTLOG ) << noTime << "-- SOFTBOOT : Starting softboot.\n" << pod_send;

   if(!unpoof_system_info())
   {
      write_syslogf("*** BOOT failed softboot, system info couldn't be loaded\n",FALSE);
      exit(0);
   }
   if(!unpoof_rooms())
   {
      write_syslogf("*** BOOT failed softboot, room info couldn't be loaded\n",FALSE);
      exit(0);
   }
   if(!unpoof_users_info())
   {
      write_syslogf("*** BOOT failed softboot, user info couldn't be loaded\n",FALSE);
      exit(0);
   }
   unlink("temp.softbootlog");
   build_user_links();

   printf("-- BOOT : Softboot, done\n");
   logStream  << setLogfile( BOOTLOG ) << noTime << "-- SOFTBOOT : Softboot finished succesfully.\n" << pod_send;
   write_syslogf("*** BOOT softbooted\n",FALSE);

   return 1;
}

int poof_rooms()
{
   pod_string filename;
   RoomsVector::iterator roomNode;

   filename = ROOMTMPINFO;   
      
   xmlTextWriterPtr ptr = xmlNewTextWriterFilename( filename.c_str(),  0 );
   xmlTextWriterStartDocument( ptr, NULL, NULL, NULL );
   xmlTextWriterStartElement( ptr, reinterpret_cast<const xmlChar *>("rooms") );
   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("\n"));
   for(roomNode=roomsList.begin(); roomNode != roomsList.end(); roomNode++)
   {
      fprintf(logfp,"saving room: %s\n",(*roomNode)->name);
      (*roomNode)->toXML(ptr);
   }
   xmlTextWriterEndElement( ptr );
   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("\n"));
   xmlTextWriterEndDocument( ptr );
   xmlFreeTextWriter( ptr );

   return 1;
}

int poof_users()
{
   UR_OBJECT user;
   pod_string filename;
   filename = USRTMPINFO;

   xmlTextWriterPtr ptr = xmlNewTextWriterFilename( filename.c_str(),  0 );
   xmlTextWriterStartDocument( ptr, NULL, NULL, NULL );
   xmlTextWriterStartElement( ptr,reinterpret_cast<const xmlChar *>("users") );
   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("\n"));
   for(user=user_first;user!=NULL;user=user->next)
   {
      fprintf(logfp,"saving user: %s\n",user->name);

      if (user->login)
      {
         write_user(user,"\nSystem doing a soft reboot, please reconnect.\n");
         socketInterface.closeSocketHard(user->socket);
         destruct_user(user);
         continue;
      }

      user->toXMLSoft(ptr);
   }
   xmlTextWriterEndElement( ptr );
   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("\n"));
   xmlTextWriterEndDocument( ptr );
   xmlFreeTextWriter( ptr );

   return 1;
}

void poof_kill_user(UR_OBJECT user)
{
   char filename[80];
   RM_OBJECT rm;

   rm=user->room;

   fprintf(logfp,"saving user: %s\n",user->name);
   save_user(user);
   fprintf(logfp,"saveing macrolist user: %s\n",user->name);
   sprintf(filename,"%s/%s.A",USERFILES,user->name);
   user->macrolist.saveMacros(filename);
   fprintf(logfp,"follow_killing user: %s\n",user->name);
   follow_kill(user,TRUE);
   user->cloaked=0;
   fprintf(logfp,"malloc_start freeing user: %s\n",user->name);
   if (user->malloc_start!=NULL) xfree(user->malloc_start);
   /* Destroy any clones */
   fprintf(logfp,"destroying clones user: %s\n",user->name);
   destroy_user_clones(user,TRUE);
   fprintf(logfp,"destroying socket: %s\n",user->name);
   /* need to do this so the port doesn't get closed on a softboot, and the user remains connected */
   user->socket->flag_as_close_on_execv(false);
   socketInterface.deletePlainSocket(user->socket );
   fprintf(logfp,"destroying user: %s\n",user->name);
   destruct_user(user);
}

int poof_system()
{
   pod_string filename;
   filename = SYSTMPINFO;

   xmlTextWriterPtr ptr = xmlNewTextWriterFilename( filename.c_str(),  0 );
   xmlTextWriterStartDocument( ptr, NULL, NULL, NULL );
   xmlTextWriterStartElement( ptr, reinterpret_cast<const xmlChar *>("globals") );
   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("\n"));

   globVarsToXML(ptr);

   xmlTextWriterEndElement( ptr );
   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("\n"));
   xmlTextWriterEndDocument( ptr );
   xmlFreeTextWriter( ptr );

   return 1;
}

void softboot_shutdown()
{
   FILE *fp;

   logStream  << setLogfile( BOOTLOG ) << noTime << "-- SOFTBOOT : Softboot command given, shutting down.\n" << pod_send;
   if(!(logfp = fopen("temp.softbootlog","w")))
   {
      write_syslogf("error in softboot_shutdown(), couldn't open file:'%s', error:\n%s\n",FALSE,"temp.softbootlog",strerror(errno));
      return;
   }
   if(!(fp = fopen(SOFT_BOOT_PID_FILE,"w")))
   {
      write_syslogf("error in softboot_shutdown(), couldn't open file:'%s', error:\n%s\n",FALSE,SOFT_BOOT_PID_FILE,strerror(errno));
      return;
   }
   fprintf(fp,"%i\n",getpid());
   fclose(fp);

   fprintf(logfp,"starting to save system status\n");
   if(!poof_system()) return;
   fprintf(logfp,"system status saved, starting to save rooms\n");

   if(!poof_rooms()) return;
   fprintf(logfp,"rooms saved, starting to save users\n");

   if(!poof_users()) return;
   fprintf(logfp,"users saved, starting to quit users\n");
   
   while(user_first)
   {
      fprintf(logfp,"quitting user: %s\n",user_first->name);
      poof_kill_user(user_first);
   }
   fprintf(logfp,"users quit,starting closing of misc ports\n");
   socketInterface.closeAllPlainSockets();
   fprintf(logfp,"misc ports closed,starting closing of listen ports\n");
   socketInterface.closeListenSockets();
   fprintf(logfp," stopping alarm timer\n");
   
   alarm(0); /* UN-schedule any SIGALRM signal to be send in heartbeat seconds */
   fprintf(logfp,"alarm timer stopped, ending this log\n");
   fclose(logfp);
   logStream  << setLogfile( BOOTLOG ) << noTime << "-- SOFTBOOT : Switching to new process, now!\n" << pod_send;
   execv("bin/softbooter",glob_argv);

   write_syslogf("error in softboot_shutdown(), couldn't start the softbooter, we're dead, error:\n%s\n",FALSE,strerror(errno));
   exit(13);
}

void show_total_uptime(UR_OBJECT user)
{
   char bstr[40];
   time_t boot = firstBootTime.get();
   time_t uptime;
       
   /* Get some values */
   strcpy( bstr, ctime(&boot) );
   uptime=(int)(time(0)-(firstBootTime.get()));
        
   write_userf(user,"~FTFirst booted  : ~FG%s",bstr);
   write_userf(user,"~FTTotal Uptime  : ~FG%s\n",time2string(TRUE,uptime).c_str() );
}

