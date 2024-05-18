#include <iomanip>
#include <fstream>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <map>
#include "general_headers.h"
#include "string_misc.h"
#include "color.h"
#include "account_reg.h"
#include "telnet.h"
#include "xalloc.h"
#include <sys/uio.h>
#include "more.h"

typedef std::map<pod_string, pod_string, std::less<pod_string>, pod_alloc< std::pair<pod_string, pod_string> >::Type > AttributesMap;

/*void buff_struct::setInvisibleFlag(bool val)
{
   if(val)
   {
      flags |= BUFF_FLAG_INVISIBLE;
   }
   else
   {
      flags &= ~BUFF_FLAG_INVISIBLE;
   }
}

bool buff_struct::getInvisibleFlag()
{
   return flags & BUFF_FLAG_INVISIBLE;
} */


void skip_spaces(std::istream & inputStream)
{
   while( inputStream.peek() == ' ' )
   {
      inputStream.ignore();
   }
}

int readToken(std::istream & inputStream, pod_string token)
{
   std::streampos orgPos = inputStream.tellg();
   unsigned int tokenLen = token.length();
   char currentChar;

   for(unsigned int pos = 0; pos < tokenLen; pos++)
   {
      inputStream.get(currentChar);
      if(currentChar != token[pos])
      {
         inputStream.seekg(orgPos, std::ios::beg);
         return 1;
      }
   }
   return 0;
}

int parse_color_code(std::ostream & outputStream, std::istream & inputStream, int color_on)
{
   char* found_color_code;
   char nextChar;
   char peekChar;
   char colorCode[3];
   std::streampos orgPos = inputStream.tellg();

   inputStream.get(nextChar);
   if (nextChar == '/')
   {
      peekChar = inputStream.peek();

      if(peekChar == '/' || peekChar == '~')
      {
         return FALSE;
      }

      inputStream.unget();
   }
   else if (nextChar == '~')
   {
      inputStream.get(colorCode[0]);
      inputStream.get(colorCode[1]);
      colorCode[2] = '\0';

      if( ( found_color_code = decode_color(colorCode) ) ) /* is existing colorcode */
      {
         if (color_on)
         {
            outputStream << found_color_code;
         }
         return TRUE;
      }
      inputStream.clear();
      inputStream.seekg(orgPos, std::ios::beg);
   }
   else
   {
      inputStream.unget();
   }

   return FALSE;
}

void accreq_tag_func( std::ostream & outputStream, AttributesMap & attributesMap, UR_OBJECT user)
{
   IntGlobalVar *accreq = (IntGlobalVar*)user->getVar("accreq");

   if (accreq->get() == ACCREQ_NONE)
   {
      outputStream << attributesMap["text"];
   }
}

void map_tag_func( std::ostream & outputStream, AttributesMap & attributesMap, UR_OBJECT user)
{
   if ( !strncmp(user->room->name, attributesMap["room"].c_str(), attributesMap["room"].length() ) )
   {
      outputStream << "~OL" << attributesMap["text"] << "~RS";
   }
   else
   {
      outputStream << attributesMap["text"];
   }
}

void parse_and_write_to_buff(std::ostream & outputStream, std::istream & inputStream, int color_on )
{
   char nextChar;
   while( !inputStream.eof() )
   {
      int retval = inputStream.peek();

      if(retval == EOF)
      {
         break;
      }

      nextChar = retval;

      if( !parse_color_code( outputStream, inputStream, color_on ) )
      {
         inputStream.get(nextChar);
         outputStream << nextChar;
      }
   }
   return;
}

void divider_func(std::ostream & outputStream, AttributesMap & attributesMap, UR_OBJECT user)
{
   pod_string caption;

   if( attributesMap.count("text") )
   {
      caption = attributesMap["text"];
   }

   outputStream << gen_seperator_line(caption);
}

void wizon_func(std::ostream & outputStream, AttributesMap & attributesMap, UR_OBJECT user)
{
   UR_OBJECT u;
   int first=TRUE;

   for(u=user_first;u!=NULL;u=u->next) 
   {      
      if (u->type == CLONE_TYPE || !u->room)
      {
         continue;
      }
      if (u->level >= LEV_THR)
      {
         if(!first)
         {
             /* Reset terminal before every newline */
            outputStream << "~RS" << "\r\n";
         }

         outputStream.setf(std::ios::left);
         outputStream <<  "   " << std::setw(14) << u->name ;
         outputStream << std::setw(13) << getLevelShortName(u->level) ;
         outputStream << std::setw(12) << u->room->name ;

         first=FALSE;
      }
   }
}

void version_func(std::ostream & outputStream, AttributesMap & attributesMap, UR_OBJECT user)
{
   outputStream << RVERSION;
}

void talker_name_func(std::ostream & outputStream, AttributesMap & attributesMap, UR_OBJECT user)
{
   outputStream << TALKER_NAME;
}

void user_count_func(std::ostream & outputStream, AttributesMap & attributesMap, UR_OBJECT user)
{
   outputStream << get_num_of_unhidden_users();
}

class MoreTagAttribute
{
   public:
      pod_string name;
      int  needed;
};

AttributesMap parse_tag_opts( MoreTagAttribute *opt, std::istream & inputStream)
{
   int cnt;
   int found_one;
   char nextChar;
   AttributesMap newMap;

#warning aborts here could be handled nicer, bit harsh now. throw error ?

   do
   {
      found_one=FALSE;
      skip_spaces(inputStream);

      for( cnt=0; opt[cnt].name.compare("*") ;cnt++ )
      {
         if ( !readToken(inputStream,opt[cnt].name) )
         {
            newMap[opt[cnt].name] = "";
            skip_spaces(inputStream);
            inputStream.get(nextChar);
            if(nextChar != '=')
            {
               abort();
            }
            skip_spaces(inputStream);
            inputStream.get(nextChar);
            if(nextChar != '\"')
            {
               abort();
            }
            inputStream.get(nextChar);
            while( (nextChar != '\"') && !inputStream.eof() )
            {
               newMap[opt[cnt].name] += nextChar;
               inputStream.get(nextChar);
            }
            skip_spaces(inputStream);
            found_one=TRUE;
         }
      }
   } while(found_one);

   for(cnt = 0; opt[cnt].name.compare("*"); cnt++)
   {
      if(opt[cnt].needed && newMap[opt[cnt].name].length() == 0)
      {
         abort();
      }
   }

   inputStream.get(nextChar);

   if(nextChar != '>')
   {
      abort();
   }

   return newMap;
}

struct more_tag
{
   char *name;
   MoreTagAttribute *opts;
   void (*func_pt)(std::ostream & outputStream, AttributesMap & attributesMap, UR_OBJECT user);
};

int parse_tag(std::ostream & outputStream, std::istream & inputStream, int color_on,UR_OBJECT user)
{
   AttributesMap attributesMap;
   int cnt;

   struct MoreTagAttribute map_tag[] = { {"room",  1  },
                                         {"text",  1  },
                                         {"*",     0  } };
   struct MoreTagAttribute divider[] = { {"text",  0  },
                                         {"*",     0  } };
   struct MoreTagAttribute empty[]   = { {"*",     0  } };

   struct more_tag tags[] = { {"map_tag",     map_tag, map_tag_func     },
                              {"if_unregged", divider, accreq_tag_func  },
                              {"divider",     divider, divider_func     },
                              {"version",     empty,   version_func     },  
                              {"talker_name", empty,   talker_name_func },  
                              {"user_count",  empty,   user_count_func  },  
                              {"wizzes_on",   empty,   wizon_func       },
                              {"*",           NULL,    NULL             } };

   inputStream.ignore();
   skip_spaces(inputStream);

   for(cnt=0;tags[cnt].name[0] != '*';cnt++)
   {
      if ( !readToken(inputStream,tags[cnt].name) )
      {
         attributesMap = parse_tag_opts(tags[cnt].opts, inputStream);

         pod_stringstream bufferStream;
         (*(tags[cnt].func_pt))(bufferStream,attributesMap,user);
         parse_and_write_to_buff(outputStream,bufferStream,color_on);

         return TRUE;
      }
   }
   return FALSE;
}

int parse_stuff(std::ostream & outputStream, UR_OBJECT user,std::istream  & inputStream,int is_more)
{
   std::streampos orgPos;
   int color_on = (user && ((IntGlobalVar*)user->getVar("ColorOn"))->get() );
   int do_default;
   char nextChar;

   while( !inputStream.eof() )
   {
      do_default=FALSE;
      int retval = inputStream.peek();

      if(retval == EOF)
      {
         break;
      }

      nextChar = retval;

      switch(nextChar)
      {
         case '\r':
            inputStream.ignore();
            break;
         case '\n':
            inputStream.ignore();
            if (color_on)
            {
               /* Reset terminal before every newline */
               outputStream << decode_color("RS");
            }
            outputStream << "\r\n";
            break;
         case '/':
         case '~':
            if( !parse_color_code( outputStream, inputStream, color_on ) )
            {
               do_default=TRUE;
            }
            break;
         case '<':
            if (is_more)
            {
               orgPos = inputStream.tellg();
               if(!parse_tag(outputStream,inputStream,color_on,user))
               {
                  inputStream.seekg(orgPos, std::ios::beg);
                  do_default=TRUE;
               }
               break;
            }
            do_default=TRUE;
            break;
         default:
            do_default=TRUE;
      }
      if(do_default)
      { 
         inputStream.get(nextChar);
         outputStream << nextChar;
      }
   }
   return 0;
}

int endsWith(const pod_string input, const pod_string end)
{
   if(input.length() < end.length())
   {
      return 0;
   }
   int comparison = input.compare(  input.length()-end.length(), end.length(), end );

   return !comparison;
}

int isTempFile(const pod_string filename)
{
   return endsWith(filename,".tempfile") || endsWith(filename,".more_tempfile");
}

void misc_more(UR_OBJECT user, const char *inpstr)
{
   StrGlobalVar *page_file    = (StrGlobalVar*)user->getVar("page_file");

   if (toupper(inpstr[0])=='E' || more(user,NULL,page_file->get()) != 1)
   {
      if(isTempFile(page_file->get()))
      {
         unlink(page_file->get().c_str());
      }
      user->ignall=user->ignall_store;
      user->misc_op=MISC_NONE;
      user->filepos=0;
      page_file->set("");
      prompt(user);
   }
   return;
}

/* Page a file out to user. Colour commands in files will only work if
   user!=NULL since if NULL we dont know if his terminal can support color_on 
   or not. Return values: 0 = cannot find file, 1 = found file, 2 = found and finished */

int more(UR_OBJECT user, PlainSocket *socket, const pod_string filename)
{
   return more_ext(user,socket,filename,TRUE);
}

/*
   Output a file one screen at a time to a user, deletes the input file if it ends in .tempfile
   generates an own tempfile to store the parsed input file till it's entirely shown.
*/
int more_ext(UR_OBJECT user, PlainSocket *socket, const pod_string whichfile, int prompt)
{
   int lines,len;
   pod_string filename = whichfile;
   len=0;
   lines=0;
   enum
   {
      NO_SUCH_FILE = 0,
      FOUND_NOT_FINISHED,
      FOUND_AND_FINISHED
   } retval;


   retval = FOUND_NOT_FINISHED;

   if( !user || !prompt || !user->misc_op )
   {
      pod_string work_filename;

      if( !user || !prompt )
      {
         work_filename = filename + ".anonymous.more_tempfile";
      }
      else
      {
         work_filename = filename + "." + user->name + ".more_tempfile";
      }
      std::ifstream inputFileStream ( filename.c_str() );
      if( inputFileStream.fail() )
      {
         return NO_SUCH_FILE;
      }
      std::ofstream outputFileStream( work_filename.c_str() );

      parse_stuff( outputFileStream, user, inputFileStream, TRUE );

      inputFileStream.close();
      outputFileStream.close();

      if(isTempFile(filename))
      {
         unlink( filename.c_str() );
      }

      filename = work_filename;
   }

   PodFile inputFile(filename);
   if( inputFile.open_read() != 0 )
   {
      if (user)
      {
         user->filepos=0;  
      }
      return NO_SUCH_FILE;
   }

   if (user)
   {
      socket = user->socket;
      inputFile.setPosition(user->filepos); /* jump to reading posn in file, should be 0 initially */
      if(!user->misc_op && prompt )       /* not halfway in a file, initial call */
      {  
         user->ignall_store=user->ignall;
         user->ignall=TRUE;               /* don't interrupt pages */
      }
   }

   std::ostream & outputStream = socket->getQueueStream();
   pod_string newLine;
   pod_string strippedLine;
   while( !inputFile.eof() && (lines < MAX_MORE_LINES || !user || !prompt) )
   {
//      outputStream << "currentLines: " << lines << std::endl;
      newLine = inputFile.fgets();
      outputStream << newLine;
      strippedLine = str_ansi(newLine);
//      outputStream << "strippedLine: \"" << strippedLine << "\"" << std::endl;
      len = strippedLine.length();
      #warning we can go past 23 lines, if we've a very long line at the end, ahwell, that's not too bad :)
      lines += len / 80 + 1;
//      outputStream << "newLines (last len : "<< len << ") : " << lines << std::endl;
   }

   if (!user || !prompt)  /* if user is logging on dont page file */
   {
      if(isTempFile(filename))
      {
         unlink( filename.c_str() );
      }
      retval = FOUND_AND_FINISHED;
   }
   else if ( inputFile.eof() )
   {
      user->filepos=0;
      no_prompt=FALSE;
      user->ignall=user->ignall_store;
      if(isTempFile(filename))
      {
         unlink( filename.c_str() );
      }
      retval = FOUND_AND_FINISHED;
   }
   else  
   {
      StrGlobalVar *page_file    = (StrGlobalVar*)user->getVar("page_file");


      user->filepos = inputFile.getPosition();
      if( !page_file->set( filename ) )
      {
         write_user(user,"Internal error 242, notify admin.");
         logStream << setLogfile( SYSLOG ) << noTime <<  "ERROR: temp filename too long: " << filename << ".\n" << pod_send;
      }
      write_user(user,"           ~OL~FB*** Press <return> to continue, 'e'<return> to exit ***");
      telnet_eor_send(user);
      no_prompt=TRUE;
   }
   inputFile.close();

   return retval;
#warning !!!! OOH! :) instead of checkking return of more, just check existence of a temp file by the given more file, \
 autogen tempfile name each time, delete after last line of tempfile, hrm, naw.., what if file does not exist ? \
 do look into this a bit tho
#warning see what more you can do here
#warning oh! make the tags a .map file, and make a nice system to add them dynamicly (make a vector of the attributes) \
 and then make plugins of all (most) of the tags! :) this could be another way of making .ex etc expandable ? :) \
 (like <nerf score>) might have to make tags always interpreted then, could be fun :) do it for everything in .ex then tho ? :p
#warning hmm.. in .ex and so, just let a plugin add one whole line ? and a header to put it onder (games, stuff) and mayby an order hint for the header and line
}
