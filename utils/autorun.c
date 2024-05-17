#include <time.h>
#include <stdarg.h>
#include <strings.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#define FALSE 0
#define TRUE 1

#define PROGNAME "-- ARun"
#define LOG_STR_MAXLEN 1000

int write_log(char *str, int write_time)
{
   FILE *fp;
   struct tm *tm_struct; 
   time_t tm_num;

   time(&tm_num);
   tm_struct=localtime(&tm_num);

   if (!(fp=fopen("logfiles/autorunlog","a"))) return 0;
   if (!write_time) fprintf(fp,"p:%5d %s",getpid(),str);
   else fprintf(fp,"p:%5d %02d/%02d %02d:%02d:%02d: %s",
                getpid(),
                tm_struct->tm_mday,
                tm_struct->tm_mon+1,
                tm_struct->tm_hour,
                tm_struct->tm_min,
                tm_struct->tm_sec,
                str);
   fclose(fp);
   return 0;
}

void write_logf(char *str, int write_time, ...)
{
   va_list args;
   static char text[LOG_STR_MAXLEN+1];

   text[0]='\0';
   va_start(args,write_time);
   vsnprintf(text,LOG_STR_MAXLEN,str,args);
   va_end(args);
   write_log(text, write_time);
}

/*
** Does the PID exist?
** Send null signal to find out.
*/

int p_exists(int pid)
{
#ifdef BE_LOUD
        printf("%s : Process %d is ", PROGNAME, pid);*/
#endif /* BE_LOUD */
        if (pid <= 0) {
#ifdef BE_LOUD
                printf("invalid\n");
#endif /* BE_LOUD */
                return(FALSE);
        }
        if (kill(pid, 0) < 0) {
                switch(errno) {
                case ESRCH:
#ifdef BE_LOUD
                        printf("dead\n");
#endif /* BE_LOUD */
                        return(FALSE);  /* pid does not exist */
                case EPERM:
#ifdef BE_LOUD
                        printf("alive\n");
#endif
                        return(TRUE);   /* pid exists */
                default:
#ifdef BE_LOUD
                        printf("state unknown: %s\n", strerror(errno));
#endif
                        return(TRUE);   /* be conservative */
                }
        }
#ifdef BE_LOUD
        printf("alive\n");
#endif
        return(TRUE);   /* pid exists */
}

int main(int argc, char *argv[])
{
   FILE *fp;
   int is_active;
   int talker_pid;
   char temp[30], *talker_exe_name,talker_exe_path[30];
   char *pod_args[2];

   printf("\n%s : Booting autorun.ex for %s\n",PROGNAME,argv[1]);
   printf("%s : Running with <pid: %d>\n",PROGNAME,getpid());
   write_logf("Autorun booted with pid: %d.\n", TRUE,getpid());

   setsid();
   /** Run in background **/
/*   switch(fork()) 
   {
      case -1: 
         printf("%s : Boot Failed...Shuting down autorun.ex\n",PROGNAME);
         write_logf("First fork failed: %s.\n", TRUE,strerror(errno));
         exit(0);
      case  0: 
#ifdef BE_LOUD
         printf("%s : Fork A-0 Continuing in Background\n",PROGNAME);
#endif
         switch(fork()) 
         {
            case -1: 
               printf("%s : Boot Failed...Shuting down autorun.ex\n",PROGNAME);
               write_logf("Second fork failed: %s.\n", TRUE,strerror(errno));
               exit(0);
            case  0: 
#ifdef BE_LOUD
               printf("%s : Fork B-0 Continuing in Background\n",PROGNAME);
#endif
               write_logf("Double fork succesfull.\n",TRUE);
               break;
            default: 
#ifdef BE_LOUD
               printf("%s : Fork B-cpid dead\n",PROGNAME);
#endif
               exit(0);
         }
         break;
      default: 
#ifdef BE_LOUD
         printf("%s : Fork A-cpid dead\n",PROGNAME);
#endif
         exit(0);
   }*/
/*   printf("%s : argv[0] %s\n",PROGNAME,argv[0]);
   printf("%s : argv[1] %s\n",PROGNAME,argv[1]);
   printf("%s : argv[2] %s\n",PROGNAME,argv[2]);*/
   talker_pid   = atoi(argv[2]);
   
   talker_exe_name=(char*)(strrchr(argv[1],'/')+1);
   sprintf(talker_exe_path,"bin/%s",talker_exe_name);

/*   printf("%s : found talker exe : %s\n",PROGNAME,talker_exe_name);*/
   printf("%s : found talker exe path : %s\n",PROGNAME,talker_exe_path);

   write_logf("Pid to monitor: %d .\n",TRUE,talker_pid);
   write_logf("Process executable: '%s'.\n",TRUE,talker_exe_path);

   while(TRUE) 
   {
      sleep(30);
      is_active=p_exists(talker_pid);
      if(!is_active) 
      {
         write_logf("Monitored process died\n",TRUE);
         if ( (fp=fopen("bootexit","r")) ) 
         {
            fscanf(fp,"%s",temp);
            printf("%s : bootexit : %s\n",PROGNAME,temp);
            write_logf("Found bootexit: %s.\n",TRUE,temp);
            fclose(fp);
            if (!strcmp(temp,"SHUTDOWN")) 
            {
               unlink("bootexit");
               write_logf("shutting down.\n",TRUE);
               exit(0);
            }
            if (!strcmp(temp,"REBOOT")) 
            {
               unlink("bootexit");
               write_logf("restarting process.\n",TRUE);
               pod_args[0]=talker_exe_path;
               pod_args[1]=NULL;
               execv(talker_exe_path,pod_args);
               printf("error re-starting pod: %s",strerror(errno));
               write_logf("Error restarting process '%s': %s\n",TRUE,talker_exe_path,strerror(errno));
               exit(0);
            }
         }
         write_logf("Found no (valid) bootexit.\n",TRUE);
         write_logf("restarting process.\n",TRUE);
         pod_args[0]=talker_exe_path;
         pod_args[1]=NULL;
         execv(talker_exe_path,pod_args);
         printf("error re-starting pod: %s",strerror(errno));
         write_logf("Error restarting process '%s': %s\n",TRUE,talker_exe_path,strerror(errno));
         exit(0);
      }
   }
   return 0;
}


