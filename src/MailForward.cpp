#include "general_headers.h"
#include "string_misc.h"
#include "GenericMessage.h"
#include "MailMessage.h"
#include "MailForward.h"

/* verify user's email when it is set specified */
int set_forward_email(UR_OBJECT user)
{
   FILE *fp,*fp_sig;
   char c;
   pod_string cmd;
   char sig_filename[80];
   StrGlobalVar *verify_code = (StrGlobalVar*)user->getVar("VrfyCode");
   StrGlobalVar *fwdaddy =     (StrGlobalVar*)user->getVar("FwdAddy");

   user->autofwd=0;
   user->mail_verified=0;
   if ( !fwdaddy->get().compare("#UNSET") )
   {
      write_user(user,"Your email address is currently ~FRunset~RS.  If you wish to use the\n");
      write_user(user,"auto-forwarding function then you must set your forwarding address.\n\n");
      return 0;
   }
   if (!forwarding.get())
   {
      write_user(user,"Even though you have set your email, the auto-forwarding function is currently unavaliable.\n");
      return 0;
   }

   cmd = "mail ";
   cmd += fwdaddy->get();
   if (!(fp=popen(cmd.c_str(),"w")))
   {
      write_syslog("Unable to open forward mail pipe in set_forward_email()\n",FALSE);
      return 0;
   }
   pod_ostringstream newCodeStream;
   newCodeStream << VERIFYCODE << rand()%9999;

   verify_code->set( newCodeStream.str() );

   /* email header */
   /* fprintf(fp,"From: %s Talker\n",TALKERNAME); */
   fprintf(fp,"To: %s <%s>\n",user->name,fwdaddy->get().c_str() );
   fprintf(fp,"RE: PODmail.\n");
   fprintf(fp,"\n");
   /* email body */
   fprintf(fp,"Hello, %s.\n\n",user->name);
   fprintf(fp,"Your verification code is: %s\n\n",verify_code->get().c_str() );
   fprintf(fp,"Thank you for setting your email address, and now that you have\n");
   fprintf(fp,"done so, you are able to use the auto-forwarding function on\n");
   fprintf(fp,"%s to have any smail sent to your email address.\n",TALKERNAME);
   fprintf(fp,"For security reasons, you must verify that you have received this email.\n");
   fprintf(fp,"Use this code with the 'verify' command when you next log onto POD.\n");
   fprintf(fp,"Thank you for coming to POD - we hope you enjoy it!\n\n Dolfin & Delphina.\n\n");
   /* add signature */
   sprintf(sig_filename,"%s/%s",DATAFILES,SIGFILE);
   if (!(fp_sig=fopen(sig_filename,"r")))
   {
      fprintf(fp,"-no sigfile available-");
   }
   else
   {
      c=getc(fp_sig);
      while(!feof(fp_sig))
      {
         putc(c,fp);
         c=getc(fp_sig);
      }
   }
   fclose(fp_sig);

   /* send the mail */
   if (pclose(fp)==-1)
   {
     write_syslog("Mail not sent in set_forward_email()\n",1);
     write_user(user,"\nThe email could not be sent to you.  Please check the address supplied.\n");
     write_user(user,"If you gave the wrong address, please use the ~FTset~RS command again.\n\n");
     verify_code->set("#NONE");
     return 0;
   }
   write_syslogf("%s had mail sent to %s by set_forward_email().\n",TRUE,user->name,get_gender(user,"him"));
   /* Inform them online */
   write_user(user,"Now that you have set your email you can use the auto-forward functions.\n");
   write_user(user,"You must verify your address with the code you will receive shortly, via email.\n");
   write_user(user,"If you do not receive any email, then use ~FTset fmail <address>~RS again, making\n");
   write_user(user,"sure you have the correct address.\n\n");
   return 0;
}

bool isValidEmail(char *email)
{
   unsigned int i;
   for (i = 0; i < strlen(email); i++)
   {
      if ( !isalnum(email[i]) &&
            email[i]!='.'     &&
            email[i]!='@'     &&
            email[i]!='-'        )
      {
         return false;
      }
   }
   return true;
}

int set_fmail(UR_OBJECT user, char *inpstr)
{
   strtolower(inpstr);
   inpstr = remove_first(inpstr);
   StrGlobalVar *fwdaddy =     (StrGlobalVar*)user->getVar("FwdAddy");

   if (!inpstr[0])
   {
      fwdaddy->set( "#UNSET" );
      write_user(user,"Email set to :~FRunset\n");
   }
   else
   {
      if( !isValidEmail(inpstr) )
      {
         write_user(user,"Not a valid email address.\n");
         return 0;
      }
      fwdaddy->set( inpstr );
      write_userf(user,"Forwarding address set to: ~FT%s\n",fwdaddy->get().c_str() );
   }
   set_forward_email(user);
   return 0;
}

/* verify that mail has been sent to the address supplied */
int verify_email(UR_OBJECT user)
{
   StrGlobalVar *verify_code = (StrGlobalVar*)user->getVar("VrfyCode");
   StrGlobalVar *fwdaddy =     (StrGlobalVar*)user->getVar("FwdAddy");

   if (words.word_count<2)
   {
      write_user(user,"Usage: verify <verification code>\n");
   }
   else if ( !fwdaddy->get().compare("#UNSET") )
   {
      write_user(user,"You have not yet set your forwarding address.  You must do this first.\n");
   }
   else if ( !verify_code->get().compare("#EMAILSET") )
   {
      write_user(user,"You have already verified your current email address.\n");
   }
   else if ( verify_code->get().compare(words.word[1]) || !verify_code->get().compare("#NONE") )
   {
      write_user(user,"That does not match your verification code.  Please check your code and try again.\n");
   }
   else
   {
      verify_code->set("#EMAILSET");
      user->mail_verified=1;
      user->autofwd=1;
      write_user(user,"\nThe verification code you gave was correct.\n");
      write_user(user,"You may now use the auto-forward functions.\n\n");
   }
   return 0;
}

int set_autofwd(UR_OBJECT user, char *inpstr)
{
   StrGlobalVar *fwdaddy =     (StrGlobalVar*)user->getVar("FwdAddy");

   strtolower(inpstr);
   if ( !fwdaddy->get().compare("#UNSET") )
   {
      write_user(user,"You have not yet set your email address - autofwd cannot be used until you do.\n");
   }
   else if (!user->mail_verified)
   {
      write_user(user,"You have not yet verified your email - autofwd cannot be used until you do.\n");
   }
   else if (!strcmp(words.word[1],"yes"))
   {
      if (user->autofwd==0)
      {
         user->autofwd=1;
         write_user(user,"You will also receive smails via email.\n");
      }
      else
      {
         write_user(user,"You are already recieving smail via email.\n");
      }
   }
   else if (!strcmp(words.word[1],"no"))
   {
      if (user->autofwd==1)
      {
         user->autofwd=0;
         write_user(user,"You will no longer receive smails via email.\n");
      }
      else
      {
         write_user(user,"You are not recieving smails via email.\n");
      }
   }
   else
   {
      write_user(user,"Usage: set autofwd <'no'/'yes'>\n\n");
   }
   return 0;
}

/* send smail to the email ccount */
int forward_email(char *name, MailMessage *message)
{
   FILE *fp,*fp_sig;
   UR_OBJECT u;
   char c,sig_filename[80];
   pod_string cmd;

   if (!forwarding.get()) return 0;
   if (!(u=get_user_exactmatch(name)))
   {
      if( !(u=temp_user_spawn(NULL,name,"forwd_email")) ) return 0;
   }
   if (!u->mail_verified || !u->autofwd)
   {
      temp_user_destroy(u);
      return 0;
   }

   StrGlobalVar *fwdaddy =     (StrGlobalVar*)u->getVar("FwdAddy");

   cmd = "/usr/sbin/sendmail -t";
   if(!(fp=popen(cmd.c_str(),"w")))
   {
      write_syslog("Unable to open forward mail file in set_forward_email()\n",0);
      temp_user_destroy(u);
      return 0;
   }
   if(!strchr(message->getFrom().c_str(),'@'))
   {
      fprintf(fp,"From: %s <%s@pod.tursiops.org>\n",message->getFrom().c_str(),message->getFrom().c_str() );
   }
   else
   {
      fprintf(fp,"From: %s\n",message->getFrom().c_str());
   }
   fprintf(fp,"To: %s <%s>\n", u->name, fwdaddy->get().c_str() );
   fprintf(fp,"\n");
/*   from=color_com_strip(from);
   fputs(from,fp);
   fputs("\n",fp);*/
   pod_string body = message->getBody();
   body=color_com_strip(body);
   fputs(body.c_str(),fp);
   fputs("\n\n",fp);
   sprintf(sig_filename,"%s/%s",DATAFILES,SIGFILE);
   if (!(fp_sig=fopen(sig_filename,"r")))
   {
      fputs("-no sigfile available-",fp);
   }
   else
   {
      c=getc(fp_sig);
      while(!feof(fp_sig))
      {
         putc(c,fp);
         c=getc(fp_sig);
      }
   }
   fclose(fp_sig);
   if (pclose(fp)==-1) write_syslog("Mail not sent in forward_email()\n",1);
   else                write_syslogf("%s had mail sent to %s email address.\n",TRUE,u->name,get_gender(u,"his"));
   temp_user_destroy(u);
   return 0;
}
