#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <iomanip>
#include <stdlib.h>
#include <ctype.h>
#include <netinet/in.h>
#include "general_headers.h"
#include "xalloc.h"
#include "globals.h"
#include "loadsave_config.h"
#include "softboot.h"
#include "string_misc.h"
#include "hist.h"
#include "info_port.h"

#define MAX_INFO_PORT_CONNECTIONS 10

struct args_buffer;
class Args;

int info_port_spodlist (PlainSocket *socket);
int info_port_userinfo (PlainSocket *socket, pod_string name);
int info_port_sysstats (PlainSocket *socket);
int info_port_histhours(PlainSocket *socket);
int info_port_histdays (PlainSocket *socket);
int info_port_ulist(PlainSocket *socket,Args *results,Args *select);

struct spod_entry
{
   char name[USER_NAME_LEN+1];
   time_t total_active_time;
   struct spod_entry *next;
};

class Args
{
   public:
      typedef std::vector<pod_string, pod_alloc< pod_string >::Type > ArgsVector;

      ArgsVector argv;
      Args();
      void   setFromString(pod_string str);
      ~Args();
};

Args::Args()
{
}

Args::~Args()
{
}

void Args::setFromString(pod_string str)
{
   unsigned int ptr,old_ptr;
   int str_size = str.size();

   for(ptr=0; ptr < str_size; )
   {
      old_ptr = ptr;
      while(ptr < str_size && str[ptr] != ',') ptr++;
      argv.push_back( str.substr(old_ptr,ptr-old_ptr) );
   }
}

struct spod_entry *spod_list = NULL;

class InfoPortSocket : public PlainSocket
{
   public:
      InfoPortSocket( int fileDescriptor );
      void info_port_error(char *err_str);
      ~InfoPortSocket();

      virtual void initializeSocket();
      virtual int input_handler(pod_string input);
      virtual int disconnect_handler();

   protected:
      char   remote_site[MAX_SITE_LEN+1];
      char   remote_ip_num[MAX_SITE_LEN+1];
      Args select;
      Args results;
};

InfoPortSocket::InfoPortSocket( int fileDescriptor ) : PlainSocket( fileDescriptor )
{
   remote_site[0]='\0';
   remote_ip_num[0]='\0';
}

InfoPortSocket::~InfoPortSocket()
{
}

void InfoPortSocket::info_port_error(char *err_str)
{
   getQueueStream() << "ERROR " << err_str << "\n";
   flag_as_closed();
}

void InfoPortSocket::initializeSocket()
{
/*   if (info_port_node == NULL)
   {
      sock_write(socket,"FULL\n");
      sock_close_conn(&socket);
      return 0;
   }*/
#warning full checking disabled

   getQueueStream() << "READY\n";

   return;
}

int InfoPortSocket::disconnect_handler()
{
   return 0;
}

int InfoPortSocket::input_handler(pod_string input)
{
   #warning we get unbuffered input here, so a line can be chopped up!
   terminate(input);

   pod_string sub_string = remove_first(input);

   if( !input.compare(0,strlen("SET"),"SET") )
   {
      pod_string::size_type equalPos = input.find('=');
      if( equalPos == pod_string::npos )
      {
         info_port_error("Wrong SET option (not 'var=val')");
         return 0;
      }

      pod_string value( input, equalPos + 1 );

      if( !sub_string.compare(0,strlen("REMOTE_HOST"),"REMOTE_HOST") )
      {
         strcpy( remote_site, value.c_str() );

         getQueueStream() << "READY\n";
         return 0;
      }

      if( !sub_string.compare(0,strlen("REMOTE_ADDR"),"REMOTE_ADDR") )
      {
         strcpy( remote_ip_num, value.c_str() );

         getQueueStream() << "READY\n";
         return 0;
      }

      if( !sub_string.compare(0,strlen("RESULTS"),"RESULTS") )
      {
         int cnt;
         results.setFromString(value);

         for(cnt=0;cnt < results.argv.size();cnt++)
         {
            if(results.argv[cnt].compare("name")   &&
               results.argv[cnt].compare("url")    &&
               results.argv[cnt].compare("online") &&
               results.argv[cnt].compare("icq")       )
            {
               info_port_error("Wrong argument");
            }
         }

         getQueueStream() << "READY\n";
         return 0;
      }
      if( !sub_string.compare(0,strlen("SELECT"),"SELECT") )
      {
         int cnt;
         select.setFromString(value);

         for(cnt=0;cnt < select.argv.size();cnt++)
         {
            if(select.argv[cnt].compare("wiz")    &&
               select.argv[cnt].compare("wizon")  &&
               select.argv[cnt].compare("wizoff") &&
               select.argv[cnt].compare("user")   &&
               select.argv[cnt].compare("useron") &&
               select.argv[cnt].compare("useroff")&&
               select.argv[cnt].compare("all")     )
            {
               info_port_error("Wrong argument");
            }
         }

         getQueueStream() << "READY\n";
         return 0;
      }

      info_port_error("Wrong SET option");
      return 0;
   }

   if( !input.compare(0,strlen("GET"),"GET") )
   {
      if( !sub_string.compare(0,strlen("SPODLIST"),"SPODLIST") )
      {
         if (system_logging.get())
         {
            write_levelf(LEV_THR,1,NULL,"~OL~FR-- info port 'GET SPODLIST' request ~RS~OL~FRfrom: ~FY%s ~FR--~RS\n",remote_site);
         }
         info_port_spodlist(this);

         getQueueStream() << "READY\n";
         flag_as_closed();
         return 0;
      }
      if( !sub_string.compare(0,strlen("SYSSTATS"),"SYSSTATS") )
      {
         if (system_logging.get())
         {
            write_levelf(LEV_THR,1,NULL,"~OL~FR-- info port 'GET SYSSTATS' request ~RS~OL~FRfrom: ~FY%s ~FR--~RS\n",remote_site);
         }
         info_port_sysstats(this);

         getQueueStream() << "READY\n";
         flag_as_closed();
         return 0;
      }
      if( !sub_string.compare(0,strlen("HISTHOURS"),"HISTHOURS") )
      {
         if (system_logging.get())
         {
            write_levelf(LEV_THR,1,NULL,"~OL~FR-- info port 'GET HISTHOURS' request ~RS~OL~FRfrom: ~FY%s ~FR--~RS\n",remote_site);
         }
         info_port_histhours(this);

         getQueueStream() << "READY\n";
         flag_as_closed();
         return 0;
      }
      if( !sub_string.compare(0,strlen("HISTDAYS"),"HISTDAYS") )
      {
         if (system_logging.get())
         {
            write_levelf(LEV_THR,1,NULL,"~OL~FR-- info port 'GET HISTDAYS' request ~RS~OL~FRfrom: ~FY%s ~FR--~RS\n",remote_site);
         }
         info_port_histdays(this);

         getQueueStream() << "READY\n";
         flag_as_closed();
         return 0;
      }
      if( !sub_string.compare(0,strlen("USERINFO"),"USERINFO") )
      {
         pod_string name = remove_first(sub_string);

         if( name.size() >  USER_NAME_LEN )
         {
            info_port_error("User name to long");
            return 0;
         }

         name[0] = toupper( name[0] );

         if (system_logging.get())
         {
            write_levelf(LEV_THR,1,NULL,"~OL~FR-- info port 'GET USERINFO %s' request ~RS~OL~FRfrom: ~FY%s ~FR--~RS\n",name.c_str(),remote_site);
         }

         info_port_userinfo( this, name );

         getQueueStream() << "READY\n";
         flag_as_closed();
         return 0;
      }
      if( !sub_string.compare(0,strlen("ULIST"),"ULIST") )
      {
         if (system_logging.get())
         {
            write_levelf(LEV_THR,1,NULL,"~OL~FR-- info port 'GET ULIST' request ~RS~OL~FRfrom: ~FY%s ~FR--~RS\n",remote_site);
         }
         info_port_ulist(this,&results,&select);

         getQueueStream() << "READY\n";
         flag_as_closed();
         return 0;
      }

      info_port_error("Wrong GET option");
      return 0;
   }

   info_port_error("Expected GET");
   return 0;
}

class InfoPortSockObjectCreator : public SockObjectCreator
{
   PlainSocket* getObjectInstance( int fileDescriptor );
};

PlainSocket* InfoPortSockObjectCreator::getObjectInstance( int fileDescriptor )
{
   return new InfoPortSocket( fileDescriptor );
}

int info_port_init_structs()
{
   static LimitedIntGlobalVar  infoport ( "infoport",  BY_LOAD_CONFIG, -1,  -1,  65535 );
   addConfigVar(&infoport);
   socketInterface.addListenSocket( new ListenSocket( "INF", new InfoPortSockObjectCreator(), infoport.get() ) );

   return 0;
}

#warning remove entirely data_ptr, inheritance, (public user_)

int add_spod(char* name,time_t active_time)
{
   struct spod_entry *spod_node_new , *spod_node_loop, *spod_node_prev;

   if( !(spod_node_new = (struct spod_entry*)xalloc(sizeof(struct spod_entry),"spod_node") ) ) return 0;

   strcpy(spod_node_new->name,name);
   spod_node_new->total_active_time = active_time;
   spod_node_prev=NULL;

   for( spod_node_loop=spod_list;
        spod_node_loop != NULL;
        spod_node_loop=spod_node_loop->next )
   {
      if(spod_node_new->total_active_time > spod_node_loop->total_active_time) break;
      spod_node_prev=spod_node_loop;
   }

   spod_node_new->next=spod_node_loop;
   if(spod_node_prev) spod_node_prev->next=spod_node_new;
   else spod_list = spod_node_new;

   return 0;
}

int delete_spodlist()
{
   struct spod_entry *spod_node_loop, *spod_node_next;

   for( spod_node_loop=spod_list;
        spod_node_loop != NULL;
        spod_node_loop=spod_node_next )
   {
      spod_node_next=spod_node_loop->next;
      xfree(spod_node_loop);
   }

   spod_list=NULL;

   return 0;
}


int info_port_spodlist(PlainSocket * socket)
{
   struct dirent **dir_list;
   int file_count=0,cnt=0,user_count=0;
   struct spod_entry *spod_node_loop;

   UR_OBJECT u_loop;

   if( (file_count = scandir(USERFILES, &dir_list, is_d_file, alphasort)) == -1)
   {
      write_syslogf("Error in info_port_spodlist() while opening directory '%s' : '%s'.\n",TRUE,USERFILES,strerror(errno));
      return 0;
   }

   pod_string name;
   int dotPos;
   for(cnt=0;cnt<file_count;cnt++)
   {
      name   = dir_list[cnt]->d_name;
      dotPos = name.find('.',0);
      name = name.substr(0,dotPos);

      user_count++;
      if ( !(u_loop=get_user_exactmatch((char*)name.c_str())) )
      {
         if(!(u_loop=temp_user_spawn(NULL,(char*)name.c_str(),"info_port_spodlist()"))) continue;
      }
      add_spod(u_loop->name,u_loop->total_active_time);
      temp_user_destroy(u_loop);
   }
   for(cnt=0;cnt<file_count;cnt++)
   {
      free(dir_list[cnt]);
   }
   free(dir_list);

   for( spod_node_loop  = spod_list;
        spod_node_loop != NULL;
        spod_node_loop  = spod_node_loop->next )
   {
      socket->getQueueStream() << "n:" << spod_node_loop->name << "\nt:" << time2string(FALSE,spod_node_loop->total_active_time) << "\n";
   }
   delete_spodlist();

   return 0;
}

int info_port_userinfo(PlainSocket *socket, pod_string name)
{
   UR_OBJECT user;

   if ( !(user=get_user_exactmatch((char*)name.c_str())) )
   {
      if(!(user=temp_user_spawn(NULL,(char*)name.c_str(),"info_port_userinfo()")))
      {
         socket->getQueueStream() << "ERROR No such user '" << name << "'\n";
         return 0;
      }
   }

   StrGlobalVar *desc        = (StrGlobalVar*)user->getVar("desc");
   StrGlobalVar *species     = (StrGlobalVar*)user->getVar("species");
   StrGlobalVar *email       = (StrGlobalVar*)user->getVar("Email");
   StrGlobalVar *url         = (StrGlobalVar*)user->getVar("Url");
   StrGlobalVar *gend_desc   = (StrGlobalVar*)user->getVar("Gender");

   std::ostream &socketStream = socket->getQueueStream();
   socketStream << "name:" << user->name << "\n";
   socketStream << "desc:" << desc->get() << "\n";
   socketStream << "gend:" << gend_desc->get() << "\n";
   socketStream << "spec:" << species->get() << "\n";
   if( user->hide_email )
   {
      socketStream << "email:" << "email address only viewable by wizards" << "\n";
   }
   else
   {
      socketStream << "email:" << email->get() << "\n";
   }
   if( user->hide_url )
   {
      socketStream << "url:" << "homepage url only viewable by wizards" << "\n";
   }
   else
   {
      socketStream << "url:" << url->get() << "\n";
   }
   socketStream << "t_login:" << time2string(FALSE,user->total_login) << "\n";
   socketStream << "t_actv:" << time2string(FALSE,user->total_active_time) << "\n";
   socketStream << "t_idle:" << time2string(FALSE,user->total_idle_time) << "\n";
   socketStream << "t_since:" << ctime(&user->total_active_first) ;

   temp_user_destroy(user);
   return 0;
}

int info_port_sysstats(PlainSocket *socket)
{
   int count=0,cnt=0;
   float seconds;
   float hour_avg;
   float day_avg;
   float hrs;
   float days;
   struct dirent **dir_list;
   time_t boot = firstBootTime.get();

   if( (count = scandir(USERFILES, &dir_list, is_d_file, alphasort)) == -1)
   {
      write_syslogf("Error in info_port_sysstats() while opening directory '%s' : '%s'.\n",TRUE,USERFILES,strerror(errno));
      return 0;
   }
   for(cnt=0;cnt<count;cnt++)
   {
      free(dir_list[cnt]);
   }
   free(dir_list);

   std::ostream &socketStream = socket->getQueueStream();
   socketStream << "name:" << TALKER_NAME << "\n";
   socketStream << "ver:" << RVERSION << "\n";
   socketStream << "time:" << long_date(2) << "\n";
   socketStream << "boot:" << ctime(&boot_time);
   socketStream << "online:" << time2string(FALSE,time(0)-boot_time) << "\n";
   socketStream << "logins:" << login_cnt << "\n";
   socketStream << "firstboot:" << ctime(&boot);
   socketStream << "tot_online:" << time2string(FALSE,time(0)-firstBootTime.get()) << "\n";
   socketStream << "tot_logins:" << total_logins.get() << "\n";

   seconds = (float)(time(0)-firstBootTime.get());
   hrs  = seconds/(60.0*60.0);
   days = hrs/24.0;
   if(hrs!=0.0) hour_avg = (float)(total_logins.get()) / hrs;
   else hour_avg=0.0;
   if(days!=0.0) day_avg = (float)(total_logins.get()) / days;
   else day_avg=0.0;
   socketStream << "tot_logins_h:" << std::setprecision(1) << hour_avg << " per hour (" << std::setprecision(1) << day_avg << " per day)\n";

   socketStream << "users:" << count << "\n";
   socketStream << "age:" << time2string(FALSE,time(0)-(time_t)(906415200)) << "\n"; /*date --date '22 sep 1998' +%s*/
   socketStream << "longest_conn:Phin (12 hours, 32 mins and 47 secs)\n";

   return 0;
}

int info_port_histhours(PlainSocket *socket)
{
   int cnt;

   std::ostream &socketStream = socket->getQueueStream();

   socketStream << "t:" << long_date(2) << "\n";
   socketStream << "f:" << (usrcnt_hist_curr.get()+1)%(24*4) << "\n";

   for(cnt=0;cnt<(24*4);cnt++)
   {
      socketStream << "u:" << cnt << ":" << std::setprecision(1) << usrcnt_hist[cnt] << "\n";
   }
   for(cnt=0;cnt<(24*4);cnt++)
   {
      socketStream << "w:" << cnt << ":" << std::setprecision(1) << histw_1day[cnt] << "\n";
   }

   return 0;
}

int info_port_histdays(PlainSocket *socket)
{
   int cnt;

   std::ostream &socketStream = socket->getQueueStream();

   socketStream << "t:" << long_date(2) << "\n";
   socketStream << "f:" << (hist_last_daypiece.get()+1)%(12*7) << "\n";

   for(cnt=0;cnt<(12*7);cnt++)
   {
      socketStream << "u:" << cnt << ":" << std::setprecision(1) << hist_7days[cnt] << "\n";
   }
   for(cnt=0;cnt<(12*7);cnt++)
   {
      socketStream << "w:" << cnt << ":" << std::setprecision(1) << histw_7day[cnt] << "\n";
   }

   return 0;
}

int info_port_ulist(PlainSocket *socket,Args *results,Args *select)
{
   struct dirent **dir_list;
   int file_count=0,cnt=0;
   int cur_col;
   int go_on;
   int is_on;
   UR_OBJECT u_loop;

   if( (file_count = scandir(USERFILES, &dir_list, is_d_file, alphasort)) == -1)
   {
      write_syslogf("Error in info_port_ulist() while opening directory '%s' : '%s'.\n",TRUE,USERFILES,strerror(errno));
      return 0;
   }
   
   pod_string name;   
   int dotPos;
   for(cnt=0;cnt<file_count;cnt++)
   {
      name   = dir_list[cnt]->d_name;
      dotPos = name.find('.',0);
      name = name.substr(0,dotPos);
      is_on=TRUE;
      if ( !(u_loop=get_user_exactmatch((char*)name.c_str())) )
      {
         is_on=FALSE;
         if(!(u_loop=temp_user_spawn(NULL,(char*)name.c_str(),"info_port_ulist()"))) continue;
      }
      go_on=FALSE;
      for(cur_col=0;cur_col<select->argv.size();cur_col++)
      {
         if( ( !select->argv[cur_col].compare("wiz" )    && (u_loop->level >= LEV_THR) ) ||
             ( !select->argv[cur_col].compare("wizon")   && (u_loop->level >= LEV_THR) && is_on) ||
             ( !select->argv[cur_col].compare("wizoff")  && (u_loop->level >= LEV_THR) && !is_on) ||
             ( !select->argv[cur_col].compare("user")    && (u_loop->level <  LEV_THR) ) ||
             ( !select->argv[cur_col].compare("useron")  && (u_loop->level <  LEV_THR) && is_on) ||
             ( !select->argv[cur_col].compare("useroff") && (u_loop->level <  LEV_THR) && !is_on) ||
             ( !select->argv[cur_col].compare("all" ) ) )
         {
            go_on=TRUE;
            break;
         }
      }
      if(go_on)
      {
         StrGlobalVar *url         = (StrGlobalVar*)u_loop->getVar("Url");
         StrGlobalVar *icq         = (StrGlobalVar*)u_loop->getVar("Icq");

         std::ostream &socketStream = socket->getQueueStream();

         for(cur_col=0;cur_col<results->argv.size();cur_col++)
         {
            if(!results->argv[cur_col].compare("name"))
            {
               socketStream << "name:" << u_loop->name << "\n";
            }
            if(!results->argv[cur_col].compare("url"))
            {
               if( u_loop->hide_url )
               {
                  socketStream << "url:" <<  "hidden"<< "\n";
               }
               else
               {
                  socketStream << "url:" << url->get() << "\n";
               }
            }
            if(!results->argv[cur_col].compare("icq"))
            {
               socketStream << "icq:" << icq->get() << "\n";
            }
            if(!results->argv[cur_col].compare("online"))
            {
               if( is_on && (!u_loop->cloaked) )
               {
                  socketStream << "online:" <<  "online"<< "\n";
               }
               else
               {
                  socketStream << "online:" << "offline" << "\n";
               }
            }
         }
      }
      temp_user_destroy(u_loop);
   }
   for(cnt=0;cnt<file_count;cnt++)
   {
      free(dir_list[cnt]);
   }
   free(dir_list);
   return 0;
}

