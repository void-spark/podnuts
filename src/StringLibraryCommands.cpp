#include "pod_string.h"
#include "levels.h"
#include "socket_funcs.h"
#include "wordfind.h"
#include "GlobalVars.h"
#include "Room.h"
#include "user_objects.h"
#include "speech_funcs.h"
#include "StringLibrary.h"
#include "StringLibraryCommands.h"

int reloadStringLibrary(UR_OBJECT user)
{
   StringsVector::iterator iter;
   StringLibrary *stringLibrary = StringLibrary::getInstance();
   StringsVector files = stringLibrary->getFileNames();

   if (words.word_count<2)
   {
      write_userf(user,"Usage: reload <glob filename>\n");
      write_user(user,"~FGFilenames :\n");
      for( iter  = files.begin(); 
           iter != files.end();
           iter++ )
      {
         write_userf(user,"~OL~FB %s\n",iter->c_str());
      }
      write_user(user," \n");
   }
   else
   {
      for( iter  = files.begin(); 
           iter != files.end();
           iter++ )
      {
         if(!strcmp( words.word[1],iter->c_str()))
         {
            stringLibrary->parseFile( *iter );
            write_userf(user,"Succesfully reloaded global strings from '%s'.\n",words.word[1]);
            return 0;
         }
      }
      write_userf(user,"No such file '%s'.\n",words.word[1]);
   }
   return 0;
}
