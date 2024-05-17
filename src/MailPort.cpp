#define MAX_MAIL_PORT_CONNECTIONS 3
#define MAX_FROM_LEN 100
#define MAX_BODY_LEN 35*80    /* around 35 lines */

#include "general_headers.h"
#include "loadsave_config.h"
#include "string_misc.h"
#include "globals.h"
#include "smail.h"
#include "MailCopyToVar.h"
#include "MailPort.h"

class MailPortSocket : public PlainSocket
{
   public:
      MailPortSocket( int fileDescriptor );
      void mail_port_error(char *err_str);
      ~MailPortSocket(){};

      virtual void initializeSocket();
      virtual int input_handler(pod_string input);
      virtual int disconnect_handler();

   protected:
      char   user_name[USER_NAME_LEN+1];
      char   from[MAX_FROM_LEN+1];
      char   body[MAX_BODY_LEN+1];
};

MailPortSocket::MailPortSocket( int fileDescriptor ) : PlainSocket( fileDescriptor )
{
   user_name[0]='\0';
   from[0]='\0';
   body[0]='\0';
}

void MailPortSocket::initializeSocket()
{
/*   if (mail_port_node == NULL)
   {
      sock_write(socket,"FULL\n");
      sock_close_conn(&socket);
      return 0;
   }*/
#warning full checking deisabled

   getQueueStream() << "READY\n";

   return;
}

int MailPortSocket::disconnect_handler()
{
   return 0;
}

int MailPortSocket::input_handler(pod_string input)
{
   #warning we get unbuffered input here, so lines can be chopped up.
   char *ptr;
   UR_OBJECT user;

   terminate(input);

   if( !input.compare(0,strlen("MAILTO"),"MAILTO") )
   {
      if( user_name[0] )
      {
         mail_port_error("name already set!");
         return 0;
      }

      pod_string::size_type equalPos = input.find('=');
      std::cout << "equalPos: " << equalPos << std::endl;
      if( equalPos == pod_string::npos )
      {
         mail_port_error("Wrong MAILTO line (not 'MAILTO=name')");
         return 0;
      }

      pod_string name( input, equalPos + 1 );
      std::cout << "name: '" << name << "'" << std::endl;
      for(int cnt = 0; cnt < name.size(); cnt++)
      {
         std::cout << "[" << (int)name[cnt] << "]";
      }
      std::cout << "(" << name.size() << ")"<< std::endl;

      if( name.size() > USER_NAME_LEN )
      {
         mail_port_error("name to long!");
         return 0;
      }

      name[0] = toupper( name[0] );

      if ( !(user=get_user_exactmatch( (char*)name.c_str())) )
      {
         if( !(user=temp_user_spawn(NULL,(char*)name.c_str(), "mail_port_input_handler()")) )
         {
            mail_port_error("No such user");
            return 0;
         }
      }
      temp_user_destroy(user);
      strcpy( user_name, name.c_str() );

      getQueueStream() << "READY\n";

      return 0;
   }

   if( !input.compare(0,strlen("FROM"),"FROM") )
   {
      if( from[0])
      {
         mail_port_error("from already set!");
         return 0;
      }

      pod_string::size_type equalPos = input.find('=');
      if( equalPos == pod_string::npos )
      {
         mail_port_error("Wrong FROM line (not 'FROM=name')");
         return 0;
      }

      pod_string fromString( input, equalPos + 1 );

      if( fromString.size() > MAX_FROM_LEN )
      {
         mail_port_error("from to long!");
         return 0;
      }

      strcpy( from, fromString.c_str() );

      getQueueStream() << "READY\n";

      return 0;
   }

   if( !input.compare(0,strlen("LINE"),"LINE") )
   {
      pod_string::size_type equalPos = input.find('=');
      if( equalPos == pod_string::npos )
      {
         mail_port_error("Wrong LINE line (not 'LINE=text')");
         return 0;
      }

      pod_string line( input, equalPos + 1 );

      pod_string cutmsg("--clipped--\n");

      if( strlen(body) + line.size() + cutmsg.size() + 1 > MAX_BODY_LEN )
      {
         strcat( body, cutmsg.c_str() );
         send_mail_ex(NULL, from, user_name,body, 0);
         mail_port_error("body to long!");
         return 0;
      }

      strcat( body, line.c_str() );
      strcat( body, "\n" );

      getQueueStream() << "READY\n";

      return 0;
   }

   if( !input.compare(0,strlen("FINISHED"),"FINISHED") )
   {
      if(!user_name[0])
      {
         mail_port_error("username unset!");
      }
      else
      {
         ptr=body+(strlen(body)-1);

         while(*ptr == '\n' || *ptr == ' ')
         {
            *ptr = '\0';
            ptr--;
         }
         strcat(body,"\n");
         send_mail_ex(NULL, from, user_name,body, 0);
         getQueueStream() << "READY\n";
      }

      flag_as_closed();

      return 0;
   }

   mail_port_error("unrecognized reply");

   return 0;
}

void MailPortSocket::mail_port_error(char *err_str)
{
   getQueueStream() << "ERROR " << err_str << "\n";
   flag_as_closed();
}

class MailPortSockObjectCreator : public SockObjectCreator
{
   PlainSocket* getObjectInstance( int fileDescriptor );
};

PlainSocket* MailPortSockObjectCreator::getObjectInstance( int fileDescriptor )
{
   return new MailPortSocket( fileDescriptor );
}

int mail_port_init_structs()
{
   static LimitedIntGlobalVar  mailport ( "mailport",  BY_LOAD_CONFIG, -1,  -1,  65535 );
   addConfigVar(&mailport);
   socketInterface.addListenSocket( new ListenSocket( "EML", new MailPortSockObjectCreator(), mailport.get() ) );

   user_var_add_cust( "copyto", new CopyToObjectCreator(), USR_SAVE_SOFT );

   return 0;
}
