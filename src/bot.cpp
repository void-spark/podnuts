#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "general_headers.h"
#include "xalloc.h" 
#include "bot.h"

int bot_act(UR_OBJECT user, char *inpstr)
{
    char *output;
 
    output=(char*)xalloc((strlen(inpstr)+15),"bot_cat_str");
 
    strcpy(output,"{ BOT } ");
    strcat(output,inpstr);
    write_userf(user,"Sent '%s' to bot.\n",output);
    strcat(output,"\r\n");
 
    write_level(LEV_BOT,TRUE,output,user);

    xfree(output);
    return 0;
}

int my_system (char *command)
{
   int pid, status;

   if (command == 0)
   {
      return 1;
   }
   pid = fork();
   if (pid == -1)
   {
      return -1;
   }
   if (pid == 0)
   {
      char *argv[4];
      argv[0] = "sh";
      argv[1] = "-c";
      argv[2] = command;
      argv[3] = 0;
      execv("/bin/sh", argv);
      exit(127);
   }
   do
   {
      if (waitpid(pid, &status, 0) == -1)
      {
         if (errno != EINTR)
         {
            return -1;
         }
      }
      else
      {
         return status;
      }
   } while(1);
}

int start_bot(UR_OBJECT user)
{

    my_system("cd $HOME/botdir/bin; ./bot&");
    return 0;
}

int stop_bot(UR_OBJECT user)
{
    bot_act(user,".quit");

    return 0;
}

