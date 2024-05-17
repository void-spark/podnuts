#include "../general_headers.h"
#include "../globals.h"
#include "../string_misc.h"
#include "../help.h"
#include "../more.h"
#include "logviewer_subsystem.h"

#define SUGGESTIONS  "logfiles/suggestions"

int rsuggest(UR_OBJECT user, char *inpstr);
int suggest(UR_OBJECT user, char *inpstr);

extern "C" void plugin_init()
{
   cmd_add("rsuggest",   LEV_THR, ADM, &rsuggest);
   cmd_add("suggest",   LEV_ONE, GEN, &suggest);
}

int rsuggest(UR_OBJECT user, char *inpstr)
{
   struct words_struct* words_ptr = &words;
   log_type suggestionLog( "suggestion", "Suggestion", SUGGESTIONS );

   if (words_ptr->word_count==1)
   {
      showLogfile( user, suggestionLog );
      return 0;
   }
   else if (!isNumber(words_ptr->word[1]))
   {
      write_user(user,"Usage: rsuggest [<lines from the end>]\n");
      return 0;
   }

   showLogfileLines(user, suggestionLog, atoi(words_ptr->word[1]) );
   return 0;
}

/*** Write a string to Suggestion Log ***/
int write_suggest(char *str, int write_time)
{
   FILE *fp;

   if (!system_logging.get() || !(fp=fopen(SUGGESTIONS,"a")))
   {
      return 0;
   }
   if (!write_time)
   {
      fputs(str,fp);
   }
   else fprintf(fp,"%s: %s",long_date(3).c_str(),str);
   fclose(fp);
   return 0;
}

/* Write suggestion to file */
int suggest(UR_OBJECT user, char *inpstr)
{
   struct words_struct* words_ptr = &words;
   if ((words_ptr->word_count<2)||(!strcmp(words_ptr->word[0],"")))
   {
      write_user(user,"Usage: suggest <any suggestions you might have, for commands, rooms, etc...>\n");
      return 0;
   }

   pod_stringstream outputStream;

   outputStream << "SUGGESTION FROM ~OL" << user->name << "~RS:\n~OL~FT " << inpstr << "\n\n";
   write_suggest((char*)outputStream.str().c_str(),1);
   write_user(user,"\nSuggestion Noted.\n");
   return 0;
}
