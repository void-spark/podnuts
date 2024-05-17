#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "general_headers.h"
#include <sys/uio.h>
#include "more.h"
#include "cmd_main.h"
#include "string_misc.h"
#include "help.h"

struct game_group
{
   char *name;
   int   id;
} game_groups[] = {  { "CARD  ",  CAR },
                     { "COMBAT",  FIG },
                     { "BOARD ",  BOA },
                     { "*",       0   } };

struct command_group
{
   char *name;
   int   id;
} command_groups[] = {  { "Messages)",  MSG   },
                        { "Rooms)",     ROOM  }, 
                        { "General)",   GEN   },
                        { "Chitter)",   SPCH  },
                        { "System)",    SYS   },
                        { "Info)",      INF   },
                        { "Admin)",     ADM   },
                        { "FunStuffs)", FUN   },
                        { "Clones)",    CLO   },
                        { "Bot)",       BOTY  },
                        { "*",          0     } };

int is_game(Command *command)
{
  int grp;

  for(grp=0;game_groups[grp].name[0] != '*';grp++) 
  {
     if(command->getCommandGroup() == game_groups[grp].id) return 1;
  }
  return 0;
}


int get_cmdcnt(UR_OBJECT user)
{
   int total_commands = 0;
   CommandMap::iterator iterator;

   for(iterator = commandMap.begin();iterator != commandMap.end();iterator++)
   {
      if (iterator->second->getLevel() <= user->level) total_commands++;
   }
   return total_commands;
}


int help_index(UR_OBJECT user)
{
   pod_string filename;

   filename = DATAFILES;
   filename += "/";
   filename += MAINHELPFILE;

   if( !more_ext(user,NULL,filename,FALSE) ) 
   {
      write_user(user,"There is no Wizlist.\n");
   }   
   return 0;
}

/*command to check if all commands have a helpfile*/

/* still needs pondering */
int help_cmd_lst_misc(UR_OBJECT user)
{
   CommandMap::iterator iterator;
   int count=0,cnt=0,col_cnt=0,reg_com;
   struct dirent **dir_list;
   char temp[20],*c,null[]="";
/*   char *d;*/
   char command_holder[20];
   pod_string output;

   if( (count = scandir(HELPFILES, &dir_list, NULL, alphasort)) == -1)
   {
      write_syslogf("Error in timed() while opening directory '%s' : '%s'.\n",TRUE,HELPFILES,strerror(errno));
      return 0;
   }
   write_userf(user,"~OL~FRNo_cmd)    ~FB[ ");
   c="~FT";
   for(cnt=0;cnt<count;cnt++)
   {
      reg_com=FALSE;

      for(iterator = commandMap.begin();iterator != commandMap.end();iterator++)
      {
         strcpy(command_holder,iterator->first.c_str());
/*         if( (d = strrchr(command_holder,' ')) ) *d = '\0';*/
         if (!strcmp(command_holder,dir_list[cnt]->d_name) ||
             !strcmp(".",dir_list[cnt]->d_name) ||
             !strcmp("..",dir_list[cnt]->d_name) )  reg_com=TRUE; 
      }
      if(!reg_com)
      {
         if (col_cnt==5) 
         {
            output += "\n";
            write_user(user,output.c_str() );
            output = "             ";
            col_cnt=0;
         }

         sprintf(temp,"%s%-10s",c,dir_list[cnt]->d_name);
         output += temp;
         c=null;
         col_cnt++;
         if(col_cnt<5)
         {
            output += "  ";
         }
      }
   }
   while(col_cnt<5)
   {
      output += " -        ";
      col_cnt++;
      if(col_cnt<5)
      {
         output += "  ";
      }
   }
   output += "~OL~FB ]\n";
   write_user(user,output.c_str());

   for(cnt=0;cnt<count;cnt++)
   {
      free(dir_list[cnt]);
   }
   free(dir_list);

   return 0;
}

int help_cmd_lst_grp(UR_OBJECT user, int wiz)
{
   int cnt,grp;
   CommandMap::iterator iterator;
   Command* command;
   char temp[20],*c,null[]="";
   char temp2[20];
   pod_string output;

   for(grp=0;command_groups[grp].name[0]!='*';grp++)
   {
      cnt=0;

      for(iterator = commandMap.begin();iterator != commandMap.end();iterator++)
      {
         command = iterator->second;
         if (          (command->getLevel() > user->level) ||
              (!wiz && (command->getLevel() >= LEV_THR))   ||
              (wiz  && (command->getLevel() <  LEV_THR))   ||
              is_game(command)                             ||
              command->getCommandGroup() != command_groups[grp].id )
              continue;

              cnt++;
      }

      if(!cnt) continue;

      write_userf(user,"~OL~FR%-11s~FB[ ",command_groups[grp].name);
      c="~FT";

      cnt=0;
      output = "";

      for(iterator = commandMap.begin();iterator != commandMap.end();iterator++)
      {
         command = iterator->second;
         if (          (command->getLevel() > user->level) ||
              (!wiz && (command->getLevel() >= LEV_THR))   ||
              (wiz  && (command->getLevel() <  LEV_THR))   ||
              is_game(command)                             ||
              command->getCommandGroup() != command_groups[grp].id )
              continue;

              if (cnt==5) 
              {
                  output += "\n";
                  write_user(user,output.c_str());
                  output = "             ";
                  cnt=0;
              }

              output += c;
              if(command->getShortcut()) sprintf(temp,"%s %c",iterator->first.c_str(),command->getShortcut());
              else sprintf(temp,"%s",iterator->first.c_str());
              sprintf(temp2,"%-10s",temp);
              output += temp2;
              c=null;
              cnt++;
              if(cnt<5)
              {
               output += "  ";
              }
      }
      while(cnt<5)
      {
         output += " -        ";
         cnt++;
         if(cnt<5)
         {
            output += "  ";
         }
      }
      output += "~OL~FB ]\n";
      write_user(user,output.c_str());
   }

   return 0;
}




int help_cmd_lst_lvl(UR_OBJECT user, int wiz)
{
   int cnt,lev,nwiz=0;
   CommandMap::iterator iterator;
   CommandShortcutsMap::iterator shortcutsIterator;
   Command* command;
   char temp[20],*c,null[]="";
   char temp2[20];
   pod_string output;

   if(wiz) lev=LEV_THR;
   else    lev=LEV_MIN;

   while(lev<=user->level && !nwiz)
   {
      cnt=0;
      output ="";

      write_userf(user,"~OL~FR%c)~RS ",getLevelName(lev)[0]);
      c="~FT";

      for(shortcutsIterator = commandShortcutsMap.begin();
          shortcutsIterator != commandShortcutsMap.end();
          shortcutsIterator++)
      {
         command = shortcutsIterator->second;
         if ( (command->getLevel() != lev) ||
              is_game(command) )
         continue;

         output += c;
         if(command->getShortcut()) sprintf(temp,"%s %c",command->getName().c_str(),command->getShortcut());
         else sprintf(temp,"%s",command->getName().c_str());
         sprintf(temp2,"%-11s",temp);
         output += temp2;
         c=null;
         cnt++;
         if (cnt==6)
         {
            output += "\n";
            write_user(user,output.c_str());
            output="";
            cnt=0;
         }
         if (!cnt)
         {
            output += "   ";
         }
      }

      for(iterator = commandMap.begin();iterator != commandMap.end();iterator++)
      {
         command = iterator->second;
         if ( (command->getLevel() != lev) ||
               command->getShortcut() ||
              is_game(command) )
         continue; 
         
         output += c;
         if(command->getShortcut()) sprintf(temp,"%s %c",iterator->first.c_str(),command->getShortcut());
         else sprintf(temp,"%s",iterator->first.c_str());
         sprintf(temp2,"%-11s",temp);
         output += temp2;
         c=null;
         cnt++;
         if (cnt==6) 
         {  
            output += "\n";
            write_user(user,output.c_str());
            output = "";
            cnt=0;  
         }
         if (!cnt)
         {
            output += "   ";
         }
      }
      if ((cnt>0) & (cnt<6)) 
      {
         output += "\n";
         write_user(user,output.c_str());
      }
      lev++;
      if (lev>=LEV_THR && !wiz) nwiz=1;
   }

   return 0;
}


/*** Show the command available ***/
int help_commands(UR_OBJECT user, int wiz, int grp_sort)
{
   write_userf(user,"\n~OL~FB*** Commands available for level: %s ***\n\n",getLevelName(user->level));

   if(grp_sort) 
   {
      help_cmd_lst_grp(user, wiz);
      if(wiz) help_cmd_lst_misc(user);
   }
   else        
   {
      help_cmd_lst_lvl(user, wiz);
   }

   if( user->level>=LEV_THR )
   {
      if(!strcmp(words.word[1],"-w") || !strcmp(words.word[1],""))
      {
         if (!wiz)	write_user(user,"\nType '~FR.help -w~RS' to see your wiz commands.\n");
         else      write_user(user,"\nType '~FR.help~RS' to see your regular commands.\n");
      }
      else
      {
         if (!wiz)	write_userf(user,"\nType '~FR.help %s -w~RS' to see your wiz commands.\n",words.word[1]);
         else      write_userf(user,"\nType '~FR.help %s~RS' to see your regular commands.\n",words.word[1]);
      }
   }

   write_userf(user,"\nTotal of ~FT%d~RS commands available to you.\n",get_cmdcnt(user)); 
   write_user(user,"Type '~FG.help <command name>~RS' for specific help on a command.\n");
   write_user(user,"Remember, you can use a '~FG.~RS' on its own to repeat your last command or speech.\n");
   write_user(user,"For the main helpfile, type ~FG.help index~RS.\n\n");
   return 0;
}

int help_games(UR_OBJECT user)
{
   int  cnt;
   char temp[20];
   int  grp;
   CommandMap::iterator iterator;
   Command* command;
   pod_string output;

   for(grp=0;game_groups[grp].name[0]!='*';grp++)
   {
      cnt=0;
      output = "";

      write_userf(user,"~OL~FR%s~RS ",game_groups[grp].name);
      for(iterator = commandMap.begin();iterator != commandMap.end();iterator++)
      {
         command = iterator->second;
         if ( (command->getLevel() > user->level) ||
              command->getCommandGroup() != game_groups[grp].id )
              continue;

         sprintf(temp,"%-11s ",iterator->first.c_str());
         output += temp;
         cnt++;
         if (cnt==5)
         {  
            output += "\n";
            write_user(user,output.c_str());
            output="";
            cnt=0;  
         }
         if (!cnt)
         {
            output += "       ";
         }
      }
      if ((cnt>0) & (cnt<5))
      {
         output += "\n";
         write_user(user,output.c_str());
      }
   }
   write_user(user,"\n");
   return 0;
}

int help_credits(UR_OBJECT user)
{
   pod_string filename;

   filename = DATAFILES;
   filename += "/";
   filename += CREDITSFILE;

   if( !more_ext(user,NULL,filename,FALSE) ) 
   {
      write_user(user,"Unable to find the credits file.\n");
   }   
   return 0;
}

/*** Do the help ***/
int help(UR_OBJECT user)
{
   int ret;
   char filename[80];
   char *c;

   if ((words.word_count<2)||(!strcmp(words.word[1],""))) 
   { 
      switch(user->help_mode)
      {
         case HELP_INDEX:       
            help_index(user); 
            return 0; 
         case  HELP_LIST : 
            help_commands(user,0,0);  
            return 0;  
         case HELP_COMMANDS :
            help_commands(user,0,1);  
            return 0;  
         default: /* we shouldn't get here */
            break;
      }
   }

   if (!strcmp(words.word[1],"-w")) 
   {
      switch(user->help_mode)
      {
         case  HELP_LIST : 
            help_commands(user,1,0); 
            return 0; 
         case HELP_COMMANDS :
            help_commands(user,1,1); 
            return 0; 
         default: 
            break;
      }
   }
   
   if (!strcmp(words.word[1],"index")) 
   {  
      help_index(user); 
      return 0; 
   }
   if (!strcmp(words.word[1],"list")) 
   {  
      if ((!strcmp(words.word[2],"-w"))&&(user->level>=LEV_THR))
      { 
         help_commands(user,1,0); 
         return 0; 
      }
      help_commands(user,0,0);  
      return 0;  
   }
   if (!strcmp(words.word[1],"commands")) 
   {  
      if ((!strcmp(words.word[2],"-w"))&&(user->level>=LEV_THR))
      { 
         help_commands(user,1,1); 
         return 0; 
      }
      help_commands(user,0,1);  
      return 0;  
   }
   if (!strcmp(words.word[1],"credits")) 
   {  
      help_credits(user);  
      return 0;  
   }
   if (!strcmp(words.word[1],"games")) 
   {  
      help_games(user);  
      return 0;  
   }

/* Check for any illegal crap in searched for filename so they cannot list 
   out the /etc/passwd file for instance. */
   c=words.word[1];
   while(*c)
   {
      if (*c=='.' || *c++=='/') 
      {
         write_user(user,"Sorry, there is no help on that topic.\n");
         return 0;
      }
   }
   sprintf(filename,"%s/%s",HELPFILES,words.word[1]);
   if (!(ret=more(user,NULL,filename)))
   write_user(user,"Sorry, there is no help on that topic.\n");
   if (ret==1) user->misc_op=MISC_MORE;
   return 0;
}


int ranks_faq(UR_OBJECT user, int faq)
{
   int ret;
   char filename[80];
   char *c;
        
   if (words.word_count<2) 
   {
      if (faq) sprintf(filename,"%s/%s",FAQFILES,MAINFAQFILE);
      else sprintf(filename,"%s/%s",RANKFILES,MAINRANKSFILE);
      if (!(ret=more(user,NULL,filename))) 
      {
         if (faq) write_user(user,"There are no faqz at the moment.\n");
         else write_user(user,"There are no ranks at the moment.\n");
         return 0;
      }
      write_user(user,"\n");
      if (ret==1) user->misc_op=MISC_MORE;
      return 0;
   }
   /* Check for any illegal crap in searched for filename so they cannot list
      out the /etc/passwd file for instance. */
   c=words.word[1];
   while(*c) 
   {
      if (*c=='.' || *c=='/') 
      {
         write_user(user,"Sorry, there is no information about that level at the moment.\n\n");
         if (ret==1) user->misc_op=MISC_MORE;
         return 0;
      }
      if ((*c=='*')&&(user->level<LEV_FIV))
      {
         write_user(user,"Sorry, there is no file called that.\n");
         if (ret==1) user->misc_op=MISC_MORE;
         return 0;
      }
      ++c;
   }
   if (faq) sprintf(filename,"%s/%s",FAQFILES,words.word[1]);
   else sprintf(filename,"%s/%s",RANKFILES,words.word[1]);
   if (!(ret=more(user,NULL,filename)))
   {
      if (faq) write_user(user,"Sorry, there is no information about that faq at the moment.\n\n");
      else  write_user(user,"Sorry, there is no information about that level at the moment.\n\n");        
   }
   if (ret==1) user->misc_op=MISC_MORE;
   return 0;
}


