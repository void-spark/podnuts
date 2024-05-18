#include "general_headers.h"
#include "string_misc.h"
#include "socket_funcs.h"
#include "login.h"
#include "telnet.h"
#include "parse.h"
#include "crash.h"
#include "more.h"
#include "podnuts.h"
#include "StringLibrary.h"
#include "TelnetSocket.h"
#include "UserTelnetHandler.h"

#define TELOPT_ECHO     1       /* echo */
#define TELOPT_SGA      3       /* suppress go ahead */
#define TELOPT_EOR      25      /* end or record */
#define TELOPT_NAWS     31      /* window size */

int user_input_handler(UR_OBJECT user,pod_string inputString);


UserTelnetHandler::UserTelnetHandler()
{
   telnetSocket = 0;
   userPtr      = 0;
   debug = false;
}

UserTelnetHandler::~UserTelnetHandler()
{
}

void UserTelnetHandler::setSocket( TelnetSocket * socket )
{
   telnetSocket = socket;
}

bool is_in_file2( pod_string filename, pod_string str)
{
   std::ifstream input;
   
   input.open( filename.c_str() );
   pod_string buffer;
   
   while(!input.eof())
   {
      input >> buffer;
      if(buffer == str)
      {
         input.close();
         return true;
      }
   }
   input.close();
   
   return false;
}

void UserTelnetHandler::handleConnect()
{
   StringLibrary * stringLibrary = StringLibrary::getInstance();

   userPtr = 0;

   int totalConnections = get_num_of_users() + get_num_of_logins();
   bool tooManyUsers    = totalConnections >= max_users.get();
   bool onWizardPort    = telnetSocket->getLocalPort() == socketInterface.getListenPortNumber("W");

   if( tooManyUsers && !onWizardPort )
   {
      telnetSocket->getQueueStream() << "\r\nSorry, the talker is full at the moment.\r\n\r\n";
      sock_close_conn( telnetSocket );
      return;
   }

   userPtr = create_user();
   if( userPtr == 0 )
   {
      telnetSocket->getQueueStream() << "\r\n" << stringLibrary->makeString("syserror") << ": unable to create session.\r\n\r\n";
      sock_close_conn( telnetSocket );
      return;
   }

   userPtr->socket     = telnetSocket;
   userPtr->login      = LOGIN_CHECK_NAME;
   userPtr->last_input = time(0);

   telnetSocket->requestHimEnable( TELOPT_NAWS );

   more(NULL,telnetSocket,MOTD1);

   #warning I believe the next line to be unnececary:
   //sendEchoOn(userPtr);

   pod_string filename = "datafiles/ignorelogin";
   if(!is_in_file2(filename,telnetSocket->getPeerSite()))
   {
      write_system_admf("~OL~FR-- login ~RS~OL: %s ~FR--~RS\n" ,telnetSocket->getPeerSite().c_str() );
   }

   if(telnetSocket->getLocalPort() == socketInterface.getListenPortNumber("W"))
   {
      write_user(userPtr,"** Wizport login **\n\n");
   }

   /*** Ask to accept EOR's***/
   telnetSocket->requestUsEnable( TELOPT_EOR );
   
   write_user(userPtr,"User login: ");

   //DO 1,3,5
   //WILL 3,6,24,32,33,34,35,39
   // (NAWS)
   return;
}


void UserTelnetHandler::handleDisconnect()
{
   if(userPtr)
   {
      quit( userPtr );  /* writes a bye msg on a closed socket (broken pipe err.)*/
   }
   return;
}

void UserTelnetHandler::setUserPtr(UR_OBJECT user)
{
   userPtr=user;
}

bool should_echo(UR_OBJECT user)
{
   // Are we in a password prompt
   bool passwordPrompt = user->login == LOGIN_CHECK_PW ||
                         user->login == LOGIN_NEWBIE_4 ||
                         user->login == LOGIN_NEWBIE_5;

   // Are we supposed to echo characters back (TELOPT_ECHO)
   bool shouldEcho = user->socket->isUsEnabled( TELOPT_ECHO );
   
   // Are we in a password prompt, and do we want to hide it
   bool hiddenPasswordPrompt = passwordPrompt && !password_echo.get();
   
   if( shouldEcho && !hiddenPasswordPrompt )
   {
      return true;
   }

   return false;
}

void UserTelnetHandler::handleText(pod_string input)
{
   pod_string line;
   curr_user_destructed=FALSE;
   curr_user=userPtr;

   if( debug )
   {
      std::cout << "UserTelnetHandler::handleText() called with chars : ";
      for(unsigned int cnt=0;cnt < input.size();cnt++)
      {
         unsigned char curChar = input[cnt];
         std::cout << "[" << (unsigned int)curChar << "]";
      }
      std::cout << std::endl;
   }   
      
   if( input.size() == 0 )
   {
      return;
   }

   pod_string echo = userPtr->buffer.putData( input );

   if( should_echo( userPtr ) && echo.size() != 0 )
   {
      WriteUserStream *userStream = WriteUserStream::getInstance();
      *userStream << addUser( userPtr );
      *userStream << echo << pod_send;
   }

   while( userPtr->buffer.hasNextLine() ) /* 0,1 or 2 rounds */
   {
      line = userPtr->buffer.getLine();

      user_input_handler(userPtr,line);

      if (curr_user_destructed)
      {
         break;
      }

   }

   curr_user_destructed=FALSE;
   curr_user=NULL;

   return;
}

void UserTelnetHandler::doWeEnable( unsigned char option )
{
   if( option == TELOPT_EOR )
   {
#warning ehrm.. , we send a WILL EOR, then write_user(userPtr,"User login: "); , then wait for a response, then send an EOR...
      telnet_eor_send( userPtr );
   }
}

void UserTelnetHandler::doWeDisable( unsigned char option )
{
}

void UserTelnetHandler::doThemEnable( unsigned char option )
{
   if( option == TELOPT_NAWS )
   {
   }
}

void UserTelnetHandler::doThemDisable( unsigned char option )
{
}

void UserTelnetHandler::handleSubNegotiation( unsigned char option, pod_string value )
{
   if( option == TELOPT_NAWS && value.size() == 4 )
   {
      unsigned char width1  = value[0];
      unsigned char width0  = value[1];
      unsigned char height1 = value[2];
      unsigned char height0 = value[3];

      int width  = width0 + 256 * width1;
      int height = height0 + 256 * height1;

      IntGlobalVar *widthVar = (IntGlobalVar*)userPtr->getVar("Width");
      widthVar->set( width );
      
      IntGlobalVar *heightVar = (IntGlobalVar*)userPtr->getVar("Height");
      heightVar->set( height );
   }
}

int user_input_handler(UR_OBJECT user,pod_string inputString)
{
   static char input[ARR_SIZE];
      
   if( strlen( inputString.c_str() ) > ARR_SIZE - 1 )
   {
      abort();
   }
   strcpy(input, inputString.c_str());
   

#ifdef DEBUG_CMD_TIME
   struct timeval current_time;
   struct timeval after_time;
#endif /* DEBUG_CMD_TIME */

   no_prompt=0;
   com_status=COM_UNSET;
   force_listen=0;
   user->last_input=time(0);

   if (user->login)
   {
      login(user,input);
      return 0;
   }

   /* If a dot on its own then execute last inpstr unless its a misc op  */
   if (!(user->misc_op || user->nextCommand))
   {
      if ((!strcmp(input,".") || !strncmp(input,". ",2)))
      {
         if( user->inpstr_old[0] )
         {
            strcpy(input,user->inpstr_old);
            write_user_crt(user,input);
         }
         else
         {
            write_user(user,"Sorry, no last command to repeat.\n");
            prompt(user);
            return 0;
         }
      }
      /* else save current one for next time */
      else
      {
         if (input[0])
         {
            strncpy(user->inpstr_old,input,INPSTR_OLD_LEN);
            /* by crandonkphin : in case length of inpstr > INPSTR_OLD_LEN,
               then strncpy will result in a not null-terminated string,
               line after comment fixes that.
               ( note: char inpstr_old[INPSTR_OLD_LEN+1]; ) */
            user->inpstr_old[INPSTR_OLD_LEN] = '\0';
         }
      }
   }
   if (input[0]) crash_save(user, input);

   /* Main input check */
   wordfind(input);

   if( afk_check(user) ) return 0;
   if( misc_ops(user,input) ) return 0;
   if (!words.word_count)
   {
      if (user->command_mode) prompt(user);
      return 0;
   }
   com_status=COM_UNSET;

#ifdef DEBUG_CMD_TIME
   gettimeofday(&(current_time),NULL);
#endif /* DEBUG_CMD_TIME */
   if (input[0]) parse(user,input);/* if we have any input, parse it */

   if (!curr_user_destructed)  prompt(user);
   com_status=COM_UNSET;
#ifdef DEBUG_CMD_TIME
   gettimeofday(&(after_time),NULL);

   write_userf(user,"Entire line took %ld sec, %ld msec, %ld usec .\n",
               timeval_diff_usec(&after_time,&current_time)/1000000,
               (timeval_diff_usec(&after_time,&current_time)%1000000)/1000,
               timeval_diff_usec(&after_time,&current_time)%1000  );
#endif /* DEBUG_CMD_TIME */

   return 0;
}

bool UserTelnetHandler::mayHeEnable( unsigned char code )
{
   return false;
}

bool UserTelnetHandler::mayWeEnable( unsigned char code )
{
   if( code == TELOPT_EOR || code == TELOPT_ECHO || code == TELOPT_SGA )
   {
      return true;
   }
   return false;
}
