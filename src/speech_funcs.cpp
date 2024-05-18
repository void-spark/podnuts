#include <stdarg.h>
#include <sys/time.h>
#include <algorithm>
#include "GlobalVars.h"
#include "socket_funcs.h"
#include "Room.h"
#include "podnuts.h"
#include "string_misc.h"
#include <sys/uio.h>
#include "color.h"
#include "cmd_main.h"
#include "user_objects.h"
#include "levels.h"
#include "ignore.h"
#include "more.h"
#include "cmd_main.h"
#include "speech_funcs.h"

char textf[ARR_SIZE*2];  /* used in write_userf and such */

/*** write to admin levels, consider this a system msg.
     Multi parameter version. Calls write_level() ***/
void write_system_admf(char *str,  ...)
{
   va_list args;

   textf[0]='\0';
   va_start(args,str);
   vsnprintf(textf,ARR_SIZE*2,str,args);
   va_end(args);
   com_status = COM_SYS;
   if (system_logging.get()) write_level(LEV_THR,TRUE,textf,NULL);
   com_status = COM_UNSET;
}

/*** Write to users of level 'level' and above or below depending on 'above'
     if true then above, else then below. Multi parameter version.
     Calls write_level() ***/
void write_levelf(int level, int above, UR_OBJECT user,char *str,  ...)
{
   va_list args;

   textf[0]='\0';
   va_start(args,str);
   vsnprintf(textf,ARR_SIZE*2,str,args);
   va_end(args);
   write_level(level,above,textf,user);
}

/*** Write to users of level 'level' and above or below depending on 'above'
     if true then above, else then below. If user != NULL then don't write
     to the specified user ***/
int write_level(int level, int above, char *str, UR_OBJECT user)
{
   WriteUserStream *userStream = WriteUserStream::getInstance();

   for( UR_OBJECT u = user_first; u != NULL; u = u->next )
   {
      if ( u != user &&
           !u->login &&
           u->type != CLONE_TYPE &&
           !u->ignall &&
           !( GET_IGNORE_WIZ(u)->get() && ( com_status == WTELL   ||
                                            com_status == WEMOTE  ||
                                            com_status == WSTO    )) &&
           !( GET_IGNORE_SYS(u)->get() && ( com_status == COM_SYS )) &&
           ( (above && u->level>=level) || (!above && u->level<=level) ) )
      {
        *userStream << addUser( u ) ;
      }
      *userStream << str << pod_send ;
   }
   return 0;
}

/*** Send message to user. Multi parameter version. ***/
void write_userf(UR_OBJECT user, const char *str, ...)
{
   va_list args;

   textf[0]='\0';
   va_start(args,str);
   vsnprintf(textf,ARR_SIZE*2,str,args);
   va_end(args);
   write_user(user,textf);
}

/*** Send message to user. Multi parameter version. ***/
void write_user_crtf(UR_OBJECT user, const char *str, ...)
{
   va_list args;

   textf[0]='\0';
   va_start(args,str);
   vsnprintf(textf,ARR_SIZE*2,str,args);
   va_end(args);
   write_user_crt(user,textf);
}

/*** Send message to user ***/
int write_user(UR_OBJECT user, const char *str)
{
   WriteUserStream *userStream = WriteUserStream::getInstance();

   *userStream << addUser( user ) << str << pod_send ;
   return 0;
}

/*** Send message to user, ending the line with a carriage return ***/
int write_user_crt(UR_OBJECT user, const char *str)
{
   WriteUserStream *userStream = WriteUserStream::getInstance();

   *userStream << addUser( user ) << str << "~RS\r\n" << pod_send ;
   return 0;
}

/*** write to everyone, consider this a system msg.
     Multi parameter version. Calls write_room() ***/
void write_systemf(char *str, ...)
{
   va_list args;

   textf[0]='\0';
   va_start(args,str);
   vsnprintf(textf,ARR_SIZE*2,str,args);
   va_end(args);
   com_status = COM_SYS;
   WriteRoomStream *roomStream = WriteRoomStream::getInstance();
   *roomStream  << setRoom( NULL ) << textf << pod_send ;
   com_status = COM_UNSET;
}

void write_roomf(RM_OBJECT rm, char *str, ...)
{
   va_list args;

   textf[0]='\0';
   va_start(args,str);
   vsnprintf(textf,ARR_SIZE*2,str,args);
   va_end(args);
   write_room(rm,textf);
}

int write_room(RM_OBJECT rm, char *str)
{
   WriteRoomStream *roomStream = WriteRoomStream::getInstance();
   *roomStream  << setRoom( rm ) << str << pod_send ;
   return 0;
}

void write_room_exceptf(RM_OBJECT rm, UR_OBJECT user, char *str, ...)
{
   va_list args;

   textf[0]='\0';
   va_start(args,str);
   vsnprintf(textf,ARR_SIZE*2,str,args);
   va_end(args);
   write_room_except(rm,textf,user);
}

int write_room_except(RM_OBJECT rm, char *str, UR_OBJECT user)
{
   WriteRoomStream *roomStream = WriteRoomStream::getInstance();
   *roomStream  << setRoom( rm ) << addExcept( user ) << str << pod_send ;
   return 0;
}

////////////////////////////////STREAMS/////////////////////////////////

WriteRoomBuff::WriteRoomBuff() : pod_stringbuf( std::ios_base::out ) , room(NULL)
{
}

void WriteRoomBuff::setTargetRoom(RM_OBJECT targetRoom)
{
   room = targetRoom;
}

void WriteRoomBuff::addUserExcept(UR_OBJECT userExcept)
{
   exceptUsers.push_back(userExcept);
}

int WriteRoomBuff::overflow( int character )
{
/*   printf("Got '%c' (EOF = %i)\n",(char)character, (char)EOF);
   if( character == '\0' )
   {
      printf("Got 0\n");
      write_room( room, (char*)(str().c_str()) );
      str("");
      return character;
   }
   else
   {
      return pod_stringbuf::overflow(character);
   }*/
   return pod_stringbuf::overflow(character);
}

void WriteRoomBuff::reset()
{
   room = NULL;
   exceptUsers.clear();
}

std::streamsize WriteRoomBuff::xsputn(const char* s, std::streamsize n)
{
   std::streamsize retval = 0;
   const char* nullChar = (const char*)memchr(s, '\0', n);
   const char* curChar = s;
   const char* endChar = s + n;


   if(!nullChar)
   {
//      printf("Got '%*s' (%d)\n",n,s,n);
      return pod_stringbuf::xsputn(s,n);
   }
   else
   {
      while(true)
      {
         int pieceLength =  nullChar - curChar;

/*         if(pieceLength)
         {
            printf("SubGot '%*s'\n",pieceLength,curChar);
         }
         else
         {
            printf("SubGot zip'\n");
         }*/
         retval += pod_stringbuf::xsputn(curChar,pieceLength);
         retval += 1;

         for( UR_OBJECT u = user_first; u != NULL; u = u->next )
         {
            if ( u->login ||
                (u->room != room && room != NULL) ||
                (u->ignall && !force_listen) ||
                (GET_IGNORE_SHOUT(u)->get() &&  (com_status == SHOUT || com_status == SEMOTE) ) ||
                (GET_IGNORE_PIC(u)->get()   &&  picCmdsSet.count(currentCommand) ) ||
                (GET_IGNORE_ATMOS(u)->get() &&  (com_status == COM_ATMOS) ) ||
                (GET_IGNORE_SYS(u)->get()   &&  (com_status == COM_SYS) ) ||
                std::find( exceptUsers.begin() , exceptUsers.end() , u ) != exceptUsers.end() )
            {
               continue;
            }

            if ( u->type == CLONE_TYPE )
            {
               if (u->clone_hear==CLONE_HEAR_NOTHING || u->owner->ignall)
               {
                  continue;
               }
               /* Ignore anything not in clones room, eg shouts, system messages
                  and semotes since the clones owner will hear them anyway. */
               if ( room != u->room )
               {
                  continue;
               }

               if (u->clone_hear==CLONE_HEAR_SWEARS)
               {
                  if ( !contains_swearing( str() ) )
                  {
                     continue;
                  }
               }

               pod_string temp_buffer;
               
               temp_buffer = "~FT[ " ;
               temp_buffer += u->room->name ;
               temp_buffer += " ]:~RS " ;
               temp_buffer += str() ;

               write_user( u->owner, temp_buffer.c_str() );
            }
            else
            {
               write_user( u, str().c_str() );
            }
         }
         
         reset();
         str("");
         if(retval == n)
         {
            break;
         }
         curChar = nullChar + 1;
         int remainderLength = endChar - curChar;
         nullChar = (const char*)memchr(curChar, '\0', remainderLength);
         if(!nullChar)
         {
//            printf("SubGot '%*s'\n",remainderLength,curChar);
            retval += pod_stringbuf::xsputn(curChar, remainderLength);
            break;
         }
      }
      return retval;
   }
}

bool WriteRoomStream::instanceFlag = false;

WriteRoomStream::WriteRoomStream() :
std::basic_ostream<char, std::char_traits<char> >(&_streambuffer),
_streambuffer()
{
}

void WriteRoomStream::setTargetRoom(RM_OBJECT targetRoom)
{
   _streambuffer.setTargetRoom(targetRoom);
}

void WriteRoomStream::addUserExcept(UR_OBJECT userExcept)
{
   _streambuffer.addUserExcept(userExcept);
}

std::basic_ostream<char, std::char_traits<char> > & operator<<( std::basic_ostream<char, std::char_traits<char> > &  __os, _SetRoom __r)
{
   WriteRoomStream* writeRoomStream = dynamic_cast< WriteRoomStream* > ( &__os );
   writeRoomStream->setTargetRoom(__r._Rm);

   return __os;
}

std::basic_ostream<char, std::char_traits<char> > & operator<<( std::basic_ostream<char, std::char_traits<char> > &  __os, _AddExcept __excpt)
{
   WriteRoomStream* writeRoomStream = dynamic_cast< WriteRoomStream* > ( &__os );
   writeRoomStream->addUserExcept(__excpt._user);

   return __os;
}

WriteRoomStream* WriteRoomStream::getInstance()
{
    if(! instanceFlag)
    {
        single = new WriteRoomStream();
        instanceFlag = true;
        return single;
    }
    else
    {
        return single;
    }
}

WriteRoomStream* WriteRoomStream::single = NULL;

WriteRoomStream::~WriteRoomStream()
{
   instanceFlag = false;
}


///////////////// USERSTREAM ///////////////////////

WriteUserBuff::WriteUserBuff() : pod_stringbuf( std::ios_base::out )
{
}

void WriteUserBuff::addUser( UR_OBJECT user )
{
   users.push_back(user);
}

int WriteUserBuff::overflow( int character )
{
/*   printf("Got '%c' (EOF = %i)\n",(char)character, (char)EOF);
   if( character == '\0' )
   {
      printf("Got 0\n");
      write_room( room, (char*)(str().c_str()) );
      str("");
      return character;
   }
   else
   {
      return pod_stringbuf::overflow(character);
   }*/
   return pod_stringbuf::overflow(character);
}

void WriteUserBuff::reset()
{
   users.clear();
}

std::streamsize WriteUserBuff::xsputn( const char* s, std::streamsize n )
{
   std::streamsize retval = 0;
   const char* nullChar = (const char*)memchr(s, '\0', n);
   const char* curChar = s;
   const char* endChar = s + n;


   if(!nullChar)
   {
//      printf("Got '%*s' (%d)\n",n,s,n);
      return pod_stringbuf::xsputn(s,n);
   }
   else
   {
      while(true)
      {
         int pieceLength =  nullChar - curChar;

/*         if(pieceLength)
         {
            printf("SubGot '%*s'\n",pieceLength,curChar);
         }
         else
         {
            printf("SubGot zip'\n");
         }*/
         retval += pod_stringbuf::xsputn(curChar,pieceLength);
         retval += 1;


         for( UsersVector::iterator iterator = users.begin(); iterator != users.end(); iterator++ )
         {
            UR_OBJECT user = *iterator;
            
            IntGlobalVar *color_on    = (IntGlobalVar*)user->getVar("ColorOn");

            pod_istringstream inputStream( str() );
            std::ostream & outputStream = user->socket->getQueueStream();

            parse_stuff(outputStream,user,inputStream,FALSE);

            if(user->level==LEV_BOT)
            {
               outputStream << "\033[0mBOTEOT";
            }
            /* Reset terminal at end of string */
            if ( color_on->get() )
            {
               outputStream << decode_color("RS");
            }
            if(user->level==LEV_BOT)
            {
               outputStream << "\n";
            }
         }

         reset();
         str("");
         if(retval == n)
         {
            break;
         }
         curChar = nullChar + 1;
         int remainderLength = endChar - curChar;
         nullChar = (const char*)memchr(curChar, '\0', remainderLength);
         if(!nullChar)
         {
//            printf("SubGot '%*s'\n",remainderLength,curChar);
            retval += pod_stringbuf::xsputn(curChar, remainderLength);
            break;
         }
      }
      return retval;
   }
}

void WriteUserStream::addUser(UR_OBJECT user)
{
   _streambuffer.addUser(user);
}

std::basic_ostream<char, std::char_traits<char> > & operator<<( std::basic_ostream<char, std::char_traits<char> > &  __os, _AddUser __user)
{
   WriteUserStream* writeUserStream = dynamic_cast< WriteUserStream* > ( &__os );
   writeUserStream->addUser(__user._user);

   return __os;
}

bool WriteUserStream::instanceFlag = false;

WriteUserStream::WriteUserStream() :
std::basic_ostream<char, std::char_traits<char> >(&_streambuffer),
_streambuffer()
{
}

WriteUserStream* WriteUserStream::getInstance()
{
    if(! instanceFlag)
    {
        single = new WriteUserStream();
        instanceFlag = true;
        return single;
    }
    else
    {
        return single;
    }
}

WriteUserStream* WriteUserStream::single = NULL;

WriteUserStream::~WriteUserStream()
{
   instanceFlag = false;
}
