#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iomanip>
#include "../general_headers.h"
#include "../globals.h"
#include "../string_misc.h"
#include "../help.h"
#include "../more.h"
#include <string>

StrGlobalVar   *whoSession;
StrGlobalVar   *theSession;
TimeTGlobalVar *sessionTime;

int session(UR_OBJECT user,char* inpstr);
int comment(UR_OBJECT user,char *inpstr);

extern "C" void plugin_init()
{
   whoSession  = new StrGlobalVar  ("whosession",  USER_NAME_LEN, EVERY_BOOT, "");
   theSession  = new StrGlobalVar  ("thesession",  100,           EVERY_BOOT, "");
   sessionTime = new TimeTGlobalVar("sessiontime",                EVERY_BOOT, 0);

   glob_add_var_cust(whoSession);
   glob_add_var_cust(theSession);
   glob_add_var_cust(sessionTime);

   cmd_add("session",       LEV_ONE, MSG, &session);
   cmd_add("comment",       LEV_ONE, MSG, &comment);
}

int session(UR_OBJECT user,char* inpstr)
{
   struct words_struct* words_ptr = &words;
   int mins,secs,total,ret,hrs;
   if(words_ptr->word_count<2)
   {
      if(!theSession->isEmpty())
      {
         write_userf(user,"\n~FTThe Session is ~RS:  %s\n",theSession->get().c_str());
         write_seperator_line(user,NULL);
         if (!(ret=more(user,user->socket,"logfiles/session")))
         write_user(user,"No one has .comment'ed yet.\n\n");
         else if (ret==1) user->misc_op=MISC_MORE;
         return 0;
      }
      else
      {
         write_userf(user,"No session exists.  Would you like to set it?\n");
         return 0;  
      }
   }
   if(!strcmp(words_ptr->word[1],"-w"))
   {
      if(!theSession->isEmpty())
      {
         total=time(0) - sessionTime->get();
         secs=total%60;
         mins=(total-secs)/60;
         if(mins>=60) 
         {
            hrs=mins%60;
            mins=mins-(hrs*60);
            write_userf(user,"The session was set by %s, %d hours, %d minutes %d seconds ago.\n",whoSession->get().c_str(),hrs,mins,secs);
         } 
         else if(mins<60) write_userf(user,"The session was set by %s, %d minutes %d seconds ago.\n",whoSession->get().c_str(),mins,secs);
      }
      else write_userf(user,"No session exists.  Would you like to set it?\n");
      return 0;
   }
   if(time(0) < sessionTime->get() +600 && strcmp(user->name,whoSession->get().c_str()))
   {
      total=(sessionTime->get() + 600)-time(0);
      secs=total%60;
      mins=(total-secs)/60;
      write_userf(user,"The current session can be reset in: %d minutes and %d seconds.\n",mins,secs);
      return 0;
   }
   if(strlen(inpstr) > 100)
   {
      write_user(user,"Session can only be 100 characters big\n");
      return 0;
   }
   theSession->set(inpstr);
   unlink_checked("logfiles/session","session");
   write_roomf(NULL,"~OL>>~RS %s has set the session to: %s\n",user->name,inpstr);
   whoSession->set(user->name);
   sessionTime->setCurrentTime();
   return 0;
}

int comment(UR_OBJECT user,char *inpstr)
{
   struct words_struct* words_ptr = &words;
   FILE *fp;
   int total_len,cur_len,line_pos;
   if(theSession->isEmpty())
   {
      write_user(user,"No session exists.  Maybe you'd like to set it?\n");
      return 0;
   }
   if(words_ptr->word_count<2)
   {   
      write_user(user,"USAGE:  .comment <your comment>\n");
      return 0;
   }

   fp=fopen("logfiles/session","a");
   StrGlobalVar *color = (StrGlobalVar*)user->getVar("Color");

   pod_stringstream outputStream;

   outputStream << color->get() << std::setw(USER_NAME_LEN) << user->name << "~RS : " << inpstr << "\r\n";

   pod_string buffer = outputStream.str();
   line_pos=0;

   for(cur_len=0,total_len=buffer.length();cur_len<total_len;cur_len++,line_pos++)
   {
      putc(buffer[cur_len],fp);
      if(line_pos%76==0 && line_pos!=0)
      {
         putc('\r',fp);
         putc('\n',fp);
         fprintf(fp,"                       ");
         line_pos=23;
      }
   }
   fclose(fp);

   write_userf(user,"You comment to the session: %s\n",inpstr);
   return 0;
}
