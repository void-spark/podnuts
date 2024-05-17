#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "../general_headers.h"
#include "../help.h"
#include "../ignore.h"
#include "../dyn_str.h"
#include "../string_misc.h"

/*** Array for picture names. ***/
StringsArray picnames;

int picture(UR_OBJECT user, char *inpstr);
int ppicture(UR_OBJECT user, char *inpstr);
int preview(UR_OBJECT user, char *inpstr);
int banner(UR_OBJECT user,char *inpstr);

extern "C" void plugin_init()
{
   pod_string filename;

   
   filename += DATAFILES;
   filename += "/pic_index";

   cmd_add("picture",   LEV_TWO, FUN,  &picture);
   cmd_add("ppicture",  LEV_TWO, FUN,  &ppicture);
   cmd_add("preview",   LEV_TWO, FUN,  &preview);

   cmd_add("banner",    LEV_TWO, FUN, &banner);

   if(dyn_arr_file_add(filename,&picnames))
   {
      logStream << setLogfile( BOOTLOG ) << noTime << "BOOT FAILURE: pics_init() failed.\n" << pod_send;
      exit(133);
   }
   return;
}

int picture_list(UR_OBJECT user)
{
   char temp[20];
   int pic=0,col=0;
   pod_stringstream outputStream;
   outputStream << "\n";
   while (picnames[pic][0] != '*')
   {
      if(picnames[pic][0] == '+') 
      {
         if (col != 0)  
         outputStream << "\n";
         write_user(user, outputStream.str().c_str() );
         write_userf(user,"~OL~FR%s)\n", picnames[pic].substr(1).c_str() );
         outputStream.str("");
         col = 0;
      }
      else
      {
         sprintf(temp, "~OL~FB%-11s", picnames[pic].c_str());
         outputStream << temp;
         col++;
         if (col == 7)
         {
            outputStream << "\n";
            write_user(user, outputStream.str().c_str() );
            outputStream.str("");
            col = 0;
         }
      }
      pic++;
   }

   if ( outputStream.str().length() != 0 ) // Print out remainder of line.
   {
      outputStream << "\n";
      write_user(user, outputStream.str().c_str() );
   }
   return 0;
}
/** Show an ascii art picture ***/

int picture(UR_OBJECT user, char *inpstr)
{
   struct words_struct* words_ptr = &words;
   pod_string filename;
   char *c;

   if ((words_ptr->word_count<2)||(words_ptr->word_count>2))
   {
      write_userf(user,"Usage : picture <picture name>\n");
      write_userf(user, "\n~FG** Pictures Available **\n");
      picture_list(user);
      return 0;
   }

   /* Check for any illegal crap in searched for file so they cannot list
      out the /etc/passwd file for instance. */
   c=words_ptr->word[1];
   while(*c)
   {
      if (*c=='.' || *c=='/')
      {
         write_user(user,"~FRSorry, that picture is not available.\n");
         return 0;
      }
      ++c;
   }

   filename = PICTUREFILES;
   filename += "/";
   filename += words_ptr->word[1];

   PodFile file( filename );

   if( file.open_read() != 0 )
   {
      write_user(user,"~FRSorry, that picture is not available.\n");
      return 0;
   }

   write_roomf(user->room,"~OL%s sonars :\n\n",get_visible_name(user).c_str());

   pod_string buffer;

   buffer = file.fgets();
   while( buffer.length() != 0 )
   {
      write_room(user->room,(char*)buffer.c_str() );
      buffer = file.fgets();
   }

   file.close();

   return 0;
}


/** Show an ascii art picture to someone private (only to one phin)
   written by crandonkphin 1999 ***/

int ppicture(UR_OBJECT user, char *inpstr)
{
   struct words_struct* words_ptr = &words;
   UR_OBJECT u;
   char *chr;
   pod_string filename;


   if (user->level == LEV_MIN) write_userf(user,"~OLYou are in ~FR%s~FW....noone can see what you show.\n",globalSpecialRoomNames.getJailRoomName()->get().c_str());
   else if (words_ptr->word_count != 3)
   {
      write_userf(user,"Usage : ppicture <to whom> <picture name>\n");
      write_userf(user, "\n~FG** Pictures Available **\n");
      picture_list(user);
   }
   else if((u=get_user_and_check(user,words_ptr->word[1])))
   {
      if (u==user) write_user(user,"You can't show pics to yourself, use .preview\n");


      else if (!afk_check_verbal(user,u))
      {
         if (u->ignall && NO_OVERR(user,u))
         {
            if (u->malloc_start!=NULL) write_userf(user,"%s is using the editor at the moment.\n",u->name);
            else                       write_userf(user,"%s is ignoring everyone at the moment.\n",u->name);
         }
         else if ( GET_IGNORE_TELL(u)->get() && NO_OVERR(user,u)) write_userf(user,"%s is ignoring tells at the moment.\n",u->name);
         else if ( GET_IGNORE_PIC(u)->get()  && NO_OVERR(user,u)) write_userf(user,"%s is ignoring pictures at the moment.\n",u->name);
         else
         {
            /* Check for any illegal crap in searched for file so they cannot list
               out the /etc/passwd file for instance. */
            for(chr=words_ptr->word[2];*chr;++chr)
            {
               if (*chr=='.' || *chr=='/')
               {
                  write_user(user,"~FRSorry, that picture is not available.\n");
                  return 0;


               }
            }
            filename = PICTUREFILES;
            filename += "/";
            filename += words_ptr->word[2];

            PodFile file( filename );

            if( file.open_read() != 0 )
            {
               write_user(user,"~FRSorry, that picture is not available.\n");
               return 0;
            }

            write_userf(user,"~OLYou sonar %s the picture %s.\n",u->name,words_ptr->word[2]);
            /*write_room(user->room,"~OL%s sonars a picture to %s\n",user->name);*/
            write_userf(u,"~OL %s sonars you :\n\n",get_visible_name(user).c_str());

            pod_string buffer;

            buffer = file.fgets();
            while( buffer.length() != 0 )
            {
               write_user(u,(char*)buffer.c_str());
               buffer = file.fgets();
            }
            file.close();
         }
      }
   }
   return 0;
}
/* end of ppicture by crandonkphin */


int preview(UR_OBJECT user, char *inpstr)
{
   struct words_struct* words_ptr = &words;
   pod_string filename;
   char *c;

   if ((words_ptr->word_count<2)||(words_ptr->word_count>2)) 
   {
      write_userf(user,"Usage : preview <picture name>\n");
      write_userf(user, "\n~FG** Pictures Available **\n");
      picture_list(user);
      return 0;
   }
            
   /* Check for any illegal crap in searched for file so they cannot list
      out the /etc/passwd file for instance. */
   c=words_ptr->word[1];
   while(*c) 
   {
      if (*c=='.' || *c=='/') 
      {
         write_user(user,"~FRSorry, that picture is not available.\n");
         return 0;
      }
      ++c;
   }

   filename = PICTUREFILES;
   filename += "/";
   filename += words_ptr->word[1];

   PodFile file( filename );

   if( file.open_read() != 0 )
   {
      write_user(user,"~FRSorry, that picture is not available.\n");
      return 0;
   }


   write_userf(user,"~OLYou preview the picture %s.\n",words_ptr->word[1]);

   pod_string buffer;

   buffer = file.fgets();
   while( buffer.length() != 0 )
   {
      write_user(user,(char*)buffer.c_str());
      buffer = file.fgets();
   }
   file.close();

   return 0;
}

int banner(UR_OBJECT user,char *inpstr)
{
   struct words_struct* words_ptr = &words;
   const unsigned int buffsize = 90;
   char buffer[buffsize];
   FILE *fp;
//   char *ptr;
   pod_string cmd("/usr/games/banner -w79 \"");
   char allowed[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ12324567890 !?,.*:;)(=^";

   if(!words_ptr->word[1][0])
   {
      write_user(user,"Please supply an input string.\n");
      return 0;
   }
   if(strlen(inpstr) > 15)
   {
      write_user(user,"Whoa hossie! that'd be some serious spam :)\n");
      return 0;
   }

/*   if( (ptr = strpbrk(inpstr,"\"[]")) )
   {
      write_userf(user,"No '%c' allowed in the input string.\n",*ptr);
      return 0;
   }*/
   if(strspn(inpstr, allowed) != strlen(inpstr))
   {
      write_userf(user,"Sorry, only the characters : \"%s\" are allowed\n",allowed);
      return 0;
   }
   cmd += inpstr;
   cmd += "\"";


   if (!(fp=popen(cmd.c_str(),"r")))
   {
      write_syslog("Unable to open banner pipe in banner()\n",FALSE);
      return 0;
   }
   write_roomf(user->room,"~OL%s sonars :\n\n",get_visible_name(user).c_str());
   while (fgets(buffer,sizeof(buffer),fp) != 0)
   {
      write_room(user->room,buffer);
   }
   pclose(fp);
   return 0;
}
