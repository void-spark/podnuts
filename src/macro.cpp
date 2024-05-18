#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <string>
#include <ctype.h>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "general_headers.h"
#include "string_misc.h"
#include "cmd_main.h"
#include "StringLibrary.h"
#include "xalloc.h"
#include "parse.h"
#include "macro.h"

#define MAX_MACRO_LEN 255

MacroListGlobalVar system_macrolist;
MacroListGlobalVar system_actionlist;

inline int todigit(char c) /* little function, aint he cute? =) */
{
   return (c-'0');
}

void list_user_macros(UR_OBJECT user)
{
   write_user(user,"System macros\n");
   system_macrolist.list_macros(user);
   
   write_user(user,"\nPersonal macros\n");
   user->macrolist.list_macros(user);
}

void list_action_macros(UR_OBJECT user) 
{
   write_user(user,"System actions\n");
   system_actionlist.list_macros(user);
}

void MacroListGlobalVar::list_macros(UR_OBJECT user)
{
   MacroList::iterator iterator;

   for ( iterator = macros.begin(); iterator != macros.end(); iterator++ ) 
   {
      write_userf(user,"%s=%s\n",(*iterator).first.c_str(),(*iterator).second.getValue().c_str());
   }
}

int MacroListGlobalVar::saveMacros(char *filename) 
{ 
   FILE *fp;	
   MacroList::iterator iterator;


   if (! ( fp=fopen( filename,"w" ) ) ) 
   {
      write_syslogf("Can't open file <%s> for writing",TRUE,filename);
      return FAIL;
   }
   
   for ( iterator = macros.begin(); iterator != macros.end(); iterator++ ) 
   {
      fprintf(fp,"%s=%s\n",(*iterator).first.c_str(),(*iterator).second.getValue().c_str());
   }

   if (fclose(fp)) 
   {
      write_syslogf("Can't close file <%s>",TRUE,filename);
      return FAIL;
   }   

   return SUCCESS;
}

int MacroListGlobalVar::loadMacros(char *filename) 
{
   pod_string macrostring,name,value;
   std::ifstream fileIn;
   pod_string::size_type pos;

   fileIn.open(filename);
   if ( fileIn.fail() )
   {
      return FAIL; /* not everyone has macros */
   }

   while ( true )
   {
      getline(fileIn,macrostring);
      if(fileIn.eof())
      {
         break;
      }

      if( (pos=macrostring.find('=')) == pod_string::npos || pos == (macrostring.size()-1) )
      {
         write_syslogf("In function load_macros, file <%s>\n",TRUE,filename);	
         write_syslogf("Read bad value for macro:<%s>\n",TRUE,macrostring.c_str());
         continue;
      }
      name.assign(macrostring,0,pos);
      value.assign(macrostring,pos+1,macrostring.size()-(pos+1));
      addMacro(name,value);
   }
   fileIn.close();
#warning how do I check if this went well ?
/*   {
      write_syslogf("Can't close file <%s>",TRUE,filename);
      return FAIL;
   }   */

   return SUCCESS;
}


void MacroListGlobalVar::cmd(char *input,UR_OBJECT user,bool is_action)
{
   pod_string::size_type pos;
   pod_string macrostring(input);
   pod_string value;
   pod_string name;

   if (macrostring.empty()) /* If just .macro by itself, list the macros */
   {
      if(!is_action)
      {
         list_user_macros(user);
      }
      else
      {
         list_action_macros(user);
      }
      return;
   }

   if (macrostring.size() > MAX_MACRO_LEN) /* if macro too long */
   {
      write_userf(user,"macro string too long (curr. %d, %d max.)\n",strlen(input),MAX_MACRO_LEN);
      return;
   }

   if( (pos=macrostring.find('=')) == pod_string::npos)
   {
      write_user(user,"You need to put an = sign in\n");
      return;
   }
   name.assign(macrostring,0,pos);
   value.assign(macrostring,pos+1,macrostring.size()-(pos+1));

   if (value.empty())       /* if macro has no value portion, delete macro */
   {
      deleteMacro(name);
      return;
   }
   if ( macros.count(name) ) 
   {
      updateMacro(name,value);
      return;
   }
   addMacro(name,value);
}


pod_string Macro::getValue()
{
   return value;
}

void Macro::setValue( pod_string value )
{
   this->value = value;
   calcStar();
}

/** store the starting place of a $* expansion, for speed */
int Macro::calcStar()
{
   int num;
   char replace_type;
   pod_string::size_type pos=0;
   star=0;

   while ( (pos = value.find('$',pos)) != pod_string::npos )
   {
      replace_type = value[ pos + 1 ];
      if ( isdigit(replace_type) )
      {
         num = todigit( replace_type );
         if (num > star)
         {
            star = num;
         }
      }

      if (replace_type)
      {
         pos=pos+2;
      }
      else /* Hit end of string, we're done */
      {
         break;
      }

   }

   star++; /* we start with first *unused* word */
   return 0;
}

int MacroListGlobalVar::deleteMacro(pod_string &name)
{
   macros.erase(name);
   return 0;
}

int MacroListGlobalVar::updateMacro(pod_string &name,pod_string &value)
{
   macros[name].setValue( value );
   return SUCCESS;
}
  
int MacroListGlobalVar::addMacro(pod_string &name,pod_string &value)
{
   macros[name].setValue( value );

   return 0;
}

int name_is_exeption(UR_OBJECT user,char *name)
{
   int list_num=0;
   pod_string hold_input("");
   static const char *except[] =
   {  "someone",
      "nobody",
      "himself",
      "herself",
      "themselves",
      "all",
      "everyone",
      "everybody",
      "everyphin",
      "allphins",
      "*"          };


   if(!name[0])
   {
      return FALSE;
   }

   hold_input=name;
   strToLower(hold_input);
   while(strcmp(except[list_num],"*"))
   {
      if(hold_input == except[list_num])
      {
         return TRUE;
      }
      else list_num++;
   }

   return FALSE;
}
	
int macro_gender_match(UR_OBJECT user,pod_string &expansion,char* type)
{
   UR_OBJECT u;

   if(name_is_exeption(user,words.word[1]))
   {  
      expansion += get_gender(NULL,type);
   }
   else 
   {
      if( !(u=get_user_and_check(user,words.word[1])) )
      {
         return TRUE;
      }
      expansion += get_gender(u,type);  
   }
   return FALSE;
}

int MacroListGlobalVar::macroExpand( UR_OBJECT user )
{
   Macro *macro;

   if ( !( macro = isMacro( words.word[0] ) ) )
   {
      return FALSE;
   }

   return macro->expand( user );
}

/*find a partial match */
Macro* MacroListGlobalVar::isMacro(const char *command) 
{
   MacroList::iterator iterator;
   for ( iterator = macros.begin(); iterator != macros.end(); iterator++ ) 
   {
      if ( !strncmp(command,(*iterator).first.c_str(),strlen(command)))
      {
         return &((*iterator).second);
      }
   }
   return NULL;
}

int Macro::expand( UR_OBJECT user )
{
   StringLibrary * stringLibrary = StringLibrary::getInstance();
   UR_OBJECT u=0;
   int cnt;
   char replace_type;
   pod_string type;
   pod_string::size_type pos=0;
   pod_string::size_type reppos;
   pod_string expansion;

   if (is_running) return FALSE; /* we expand macros only once */


   while (value[pos])
   {
      if ( (reppos=value.find('$',pos)) == pod_string::npos )
      {
         expansion.append(value,pos,value.size()-pos);
         break;
      }

      expansion.append(value,pos,reppos-pos);

      if ( !(replace_type=value[reppos+1]) )
      {
         break;
      }

      StrGlobalVar *species;
      StrGlobalVar *mood;

      switch(replace_type)
      {
         case '0': /** single word replacement */
         case '1':
         case '2':
         case '3':
         case '4':
         case '5':
         case '6':
         case '7':
         case '8':
         case '9':
            if( !(todigit(replace_type) > words.word_count-1) )
            {
               expansion += words.word[ todigit(replace_type) ];
            }
            break;

         case '*': /** all words replacement */
            if(!(words.word_count < star + 1))
            {
               for (cnt=star; cnt < words.word_count; cnt++)
               {
                  expansion += words.word[cnt];
                  if(cnt < words.word_count-1)
                  {
                    expansion += " ";
                  }
               }
            }
            break;

         case 'c': /** users color replacment */
            expansion += "~RS";
            expansion += get_visible_color(user);
            break;

         case 't': /** users species replacement */
            species  = (StrGlobalVar*)user->getVar("species");

            expansion += species->get();
            break;

         case 'h': /** users mood replacement */
            mood = (StrGlobalVar*)user->getVar("mood");
            if( mood->get().length() != 0 && strcmp(mood->get().c_str(), "?" ))
            {
               expansion += mood->get();
            }
            break;

         case 'w': /** users sound replacement */
            type = get_visible_sound(user, "bogus"); /* bogus cause it only checks if a ?/! at end of that line */
            expansion += type;
            break;

         case 'C': /** target users color replacement */
            expansion += "~RS";
            if(!name_is_exeption(user,words.word[1]))
            {
               if(!(u=get_user_and_check(user,words.word[1]))) return TRUE;
               StrGlobalVar *color       = (StrGlobalVar*)u->getVar("Color");
               expansion += color->get();
            }
            else
            {
               expansion += "~OL";
            }
            break;

         case 'T': /** target users species replacement */
            if(!name_is_exeption(user,words.word[1]))
            {
               if(!(u=get_user_and_check(user,words.word[1]))) return TRUE;

               species  = (StrGlobalVar*)u->getVar("species");
               if(species->get().length() != 0)
               {
                  expansion += species->get();
               }
               else
               {
                  expansion += "unknown species";
               }
            }
            else
            {
               expansion += "random species";
            }
            break;

         case 'e':
         case 'y':
            expansion += get_gender(user,"e");
            break;

         case 'm':
            expansion += get_gender(user,"m");
            break;

         case 's':
         case 'r':
            expansion += get_gender(user,"s");
            break;

         case 'E':
         case 'Y':
            if(macro_gender_match(user,expansion,"e")) return TRUE;
            break;

         case 'M':
            if(macro_gender_match(user,expansion,"m")) return TRUE;
            break;

         case 'S':
         case 'R':
            if(macro_gender_match(user,expansion,"s")) return TRUE;
            break;

         case 'n':
            if(name_is_exeption(user,words.word[1]))
            {
               expansion += words.word[1];
            }
            else
            {
               if(!(u=get_user_and_check(user,words.word[1])))  return TRUE;
               expansion += u->name;
            }
            break;
      } /* switch */

      pos = reppos+2;

   } /* while */
   if(expansion.length() >= ARR_SIZE-1)
   {
      write_user_crt(user,stringLibrary->makeString("macro_too_big").c_str());
      return 0;
   }

   is_running = true;
   parse(user,expansion); /* re-parse the expanded string */
   is_running = false;
   return TRUE;
}
