#include <ctype.h>
#include <stdlib.h>
#include <algorithm>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "general_headers.h"
#include "string_misc.h"
#include "loadsave_user.h"
#include "smail.h"
#include "color.h"
#include "curse.h"

struct curse_entry
{
   char* name;
   char* aft_str;
   int replace_char;
   int replace_char_percent;
   int mess_color;
   int bind_to_room;
   int reverse;
   char* bind_to_room_local_str;
   char* bind_to_room_room_str;
   char* room_string;
}            /*  name:     aft_str: rep_chr:rc%  MC BND RV Bind to room local str                                 Bind to room -room- string                              Curse str*/
curse_book[]={ { "ies",    "ies"    , 'e'  , 10 , 0, 0, 0, NULL,                                                  NULL,                                                  "%s giggles and waves a pec, putting a little spell on %s.\n"                                },
               { "shark",  "mmmmth" , '\0' , 0  , 0, 1, 0, "You turn a circle, trying to swim out of the room\n", "%s turns a circle, trying to swim out of the room\n", "%s Grrrclicks and chants some evil squeeks, turning %s into a onepec'ed toothless shark!\n" },
               { "color",  ""       , '\0' , 0  , 1, 0, 0, NULL,                                                  NULL,                                                  "%s blows a shiny bubble in %s direction, who suddenly lights up like a crazed discolight\n" },
               { "reverse",""       , '\0' , 0  , 0, 0, 1, NULL,                                                  NULL,                                                  "%s sonars up at %s's reflection in the water surface,\n a soft blue beam following it. When it touches the surface there's a sudden\n flash, and reflection and original are reversed!\n" },
               { "*",      NULL     , '\0' , 0  , 0, 0, 0, NULL,                                                  NULL, NULL} };

int rand_perct(int percentage)
{
   if( (random()%100) < percentage) return 1;
   return 0;
}

int jpc_chrcpy_mov(char *out_base, int *out_offset, char **in_ptr,int maxlen)
{
   if((*out_offset) < (maxlen-1)) 
   {
      *(out_base+(*out_offset)) = *(*in_ptr)++;
      (*out_offset)++;
   }
   else 
   {
      *(out_base+(*out_offset))='\0';
      return 0;
   }
   return 1;
}

int curse(UR_OBJECT user)
{
   UR_OBJECT u;
   int cnt=0,curse_num=0;;

   if (words.word_count<3) 
   {
      write_user(user,"Usage: curse <user> <type>\n");  
      write_user(user,"~FGPossible curses:\n\n");  
      while(strcmp(curse_book[curse_num].name,"*")) 
      {
         write_userf(user,"~OL~FB%s\n",curse_book[curse_num].name);
         curse_num++;
      }
      
      return 0;
   }

   while(strcmp(curse_book[cnt].name,"*"))
   {
      if(!strcmp(curse_book[cnt].name,words.word[2])) break;
      cnt++;
   }

   if(!strcmp(curse_book[cnt].name,"*")) 
   {
      write_user(user,"No such curse.\n");  
      return 0;
   }

   if ((u=get_user(words.word[1]))!=NULL) 
   {
      if (u==user)                     write_user(user,"Trying to curse yourself is the ninth sign of madness.\n");
      else if (u->level>=user->level)  write_user(user,"You cannot curse a user of equal or higher level than yourself.\n");
      else if (u->cursed>=user->level) write_userf(user,"%s is already cursed.\n",u->name);
      else
      {
	 u->cursed=user->level;
         strcpy(u->curse_name,words.word[2]);
         write_roomf(u->room,curse_book[cnt].room_string,user->name,u->name);
         write_userf(user,"~FR~OL%s now has a \"%s\" curse of level: ~RS~OL%s.\n",u->name,words.word[2],getLevelName(user->level));
	 write_user(u,"~FR~OLYou have been cursed!\n");
	 write_syslogf("%s cursed %s.\n",TRUE,user->name,u->name);
      }
      return 0;
   }
   /* User not logged on */
   if( !(u=temp_user_spawn(user,words.word[1],"curse")) ) return 0;
   else if (u->level>=user->level) write_user(user,"You cannot curse a user of equal or higher level than yourself.\n");
   else if (u->cursed>=user->level) write_userf(user,"%s is already cursed.\n",u->name);
   else
   {
      u->cursed=user->level;
      strcpy(u->curse_name,words.word[2]);
      save_user(u);
      write_userf(user,"~FR~OL%s given a \"%s\" curse of level: ~RS~OL%s.\n",u->name,words.word[2],getLevelName(user->level));
      send_mail(user,words.word[1],"~FR~OLYou have been cursed!\n",0);
      write_syslogf("%s cursed %s.\n",TRUE,user->name,u->name);
   }
   temp_user_destroy(u);
   return 0;
}

/*** Enchant the bastard now he's apologised and grovelled enough via email ***/
int remove_curse(UR_OBJECT user)
{
   UR_OBJECT u;

   if (words.word_count<2) 
   {
      write_user(user,"Usage: enchant <user>\n");  
      return 0;
   }
   if ((u=get_user(words.word[1]))!=NULL) 
   {
      if (u==user)                    write_user(user,"Trying to enchant yourself is the tenth sign of madness.\n");
      else if (!u->cursed)            write_userf(user,"%s is not cursed.\n",u->name);  
      else if (u->cursed>user->level) write_userf(user,"%s's curse is set to level %s, you do not have the power to remove it.\n",u->name,getLevelName(u->cursed));
      else
      {     
         write_userf(user,"~FG~OLYou remove %s's curse.\n",u->name);
         write_roomf(u->room,"%s enchants %s who is now no longer cursed.\n",user->name,u->name);
	 write_user(u,"~FG~OLYou have been enchanted!\n");
	 write_syslogf("%s enchanted %s.\n",TRUE,user->name,u->name);
	 u->cursed=0;
         u->curse_name[0]='\0';
      }
      return 0;
   }
/* User not logged on */
   if( !(u=temp_user_spawn(user,words.word[1],"demote")) ) return 0;
   else if (u->cursed>user->level) write_userf(user,"%s's curse is set to level %s, you do not have the power to remove it.\n",u->name,getLevelName(u->cursed));
   else
   {
      u->cursed=0;
      u->curse_name[0]='\0';
      save_user(u);
      write_userf(user,"~FG~OLYou remove %s's curse.\n",u->name);
      send_mail(user,words.word[1],"~FG~OLYou have been enchanted.\n",0);
      write_syslogf("%s enchanted %s.\n",TRUE,user->name,u->name);
   }
   temp_user_destroy(u);
   return 0;
}

pod_string append_each_word( pod_string &input, pod_string add )
{
   int endpos   = input.length();
   int addLength = add.length();

   for(int pos = 0; pos < endpos; pos++ )
   {
      // skip color codes:
      while( is_color(input, pos) )
      {
         pos += 3;
      }
      // if current char is alpha:
      if( isalpha( input[pos] ) )
      {
         // the position to check for being non alpha is the char after the current one:
         int nextpos = pos + 1;
         // except when the next char is part of a color code, then it is the char after the colorcode(s).
         while( is_color(input, nextpos) )
         {
            nextpos += 3;
         }

         // if the next char to check is beyond the end of the string, or non alpha.
         if( nextpos == endpos || !isalpha( input[nextpos] ) )
         {
            // insert before the char after the current pos:
            input.insert( pos + 1, add );
            // string got bigger, register that:
            endpos += addLength;
            pos     = (nextpos - 1) + addLength;
         }
      }
   }
   return input;
}

pod_string randomize_colors( pod_string &input )
{
   int endpos   = input.length();

   for(int pos = 0; pos < endpos; pos++ )
   {
      while( is_color(input, pos) )
      {
         pos += 3;
      }
      if( isalpha( input[pos] ) )
      {
         input.insert( pos, get_random_color() );
         endpos += 3;
         pos += 3;
      }
   }
   return input;
}

pod_string replace_random( pod_string &input, int percent, char replacement )
{
   int endpos   = input.length();

   for(int pos = 0; pos < endpos; pos++ )
   {
      while( is_color(input, pos) )
      {
         pos += 3;
      }
      if( isalpha( input[pos] ) )
      {
         if( rand_perct( percent ) )
         {
            input[pos] = replacement;
         }
      }
   }
   return input;
}

struct ColorStatus
{
   enum ColorType
   {
      UNSET,
      BLACK,
      RED,
      GREEN,
      YELLOW,
      BLUE,
      MAGENTA,
      TURQUIOSE,
      WHITE
   };

   bool underline;
   bool bold;
   bool blink;
   bool reverse;
   ColorType foreground;
   ColorType background;
};

typedef std::map<int, ColorStatus, std::less<int>, pod_alloc< std::pair<int, ColorStatus> >::Type > ColorMap;

ColorMap convert_colors( pod_string &input )
{
   ColorMap colors;
   struct ColorStatus cs;
   int endpos   = input.length();

   cs.underline = false;
   cs.bold = false;
   cs.blink = false;
   cs.reverse = false;
   cs.foreground = ColorStatus::UNSET;
   cs.background = ColorStatus::UNSET;

   colors[0] = cs;

   for(int pos = 0; pos < endpos; pos++ )
   {
      if( is_color(input, pos) )
      {
         do
         {
            if( input[ pos + 1 ] == 'O' && input[ pos + 2 ] == 'L' )
            {
                  cs.bold = true;
            }
            if( input[ pos + 1 ] == 'U' && input[ pos + 2 ] == 'L' )
            {
                  cs.underline = true;
            }
            if( input[ pos + 1 ] == 'L' && input[ pos + 2 ] == 'I' )
            {
                  cs.blink = true;
            }
            if( input[ pos + 1 ] == 'R' )
            {
               if( input[ pos + 2 ] == 'S' )
               {
                  cs.underline = false;
                  cs.bold = false;
                  cs.blink = false;
                  cs.reverse = false;
                  cs.foreground = ColorStatus::UNSET;
                  cs.background = ColorStatus::UNSET;
               }
               if( input[ pos + 2 ] == 'V' )
               {
                  cs.reverse = true;
               }
            }
            if( input[ pos + 1 ] == 'F' )
            {
               if( input[ pos + 2 ] == 'K' )
               {
                 cs.foreground = ColorStatus::BLACK;
               }
               if( input[ pos + 2 ] == 'R' )
               {
                 cs.foreground = ColorStatus::RED;
               }
               if( input[ pos + 2 ] == 'G' )
               {
                 cs.foreground = ColorStatus::GREEN;
               }
               if( input[ pos + 2 ] == 'Y' )
               {
                 cs.foreground = ColorStatus::YELLOW;
               }
               if( input[ pos + 2 ] == 'B' )
               {
                 cs.foreground = ColorStatus::BLUE;
               }
               if( input[ pos + 2 ] == 'M' )
               {
                 cs.foreground = ColorStatus::MAGENTA;
               }
               if( input[ pos + 2 ] == 'T' )
               {
                 cs.foreground = ColorStatus::TURQUIOSE;
               }
               if( input[ pos + 2 ] == 'W' )
               {
                 cs.foreground = ColorStatus::WHITE;
               }
            }
            if( input[ pos + 1 ] == 'B' )
            {
               if( input[ pos + 2 ] == 'K' )
               {
                 cs.background = ColorStatus::BLACK;
               }
               if( input[ pos + 2 ] == 'R' )
               {
                 cs.background = ColorStatus::RED;
               }
               if( input[ pos + 2 ] == 'G' )
               {
                 cs.background = ColorStatus::GREEN;
               }
               if( input[ pos + 2 ] == 'Y' )
               {
                 cs.background = ColorStatus::YELLOW;
               }
               if( input[ pos + 2 ] == 'B' )
               {
                 cs.background = ColorStatus::BLUE;
               }
               if( input[ pos + 2 ] == 'M' )
               {
                 cs.background = ColorStatus::MAGENTA;
               }
               if( input[ pos + 2 ] == 'T' )
               {
                 cs.background = ColorStatus::TURQUIOSE;
               }
               if( input[ pos + 2 ] == 'W' )
               {
                 cs.background = ColorStatus::WHITE;
               }
            }
            input.erase(pos,3);
            endpos -= 3;
         }
         while( is_color(input, pos) );
         colors[pos] = cs;
      }
   }

   return colors;
}

pod_string to_color_string( ColorStatus stat )
{
   pod_string color_string;

   if( stat.underline == true )
   {
      color_string += "~UL";
   }
   if( stat.bold == true )
   {
      color_string += "~OL";
   }
   if( stat.blink == true )
   {
      color_string += "~LI";
   }
   if( stat.reverse == true )
   {
      color_string += "~RV";
   }
   if( stat.foreground == ColorStatus::BLACK )
   {
      color_string += "~FK";
   }
   if( stat.foreground == ColorStatus::RED )
   {
      color_string += "~FR";
   }
   if( stat.foreground == ColorStatus::GREEN )
   {
      color_string += "~FG";
   }
   if( stat.foreground == ColorStatus::YELLOW )
   {
      color_string += "~FY";
   }
   if( stat.foreground == ColorStatus::BLUE )
   {
      color_string += "~FB";
   }
   if( stat.foreground == ColorStatus::MAGENTA )
   {
      color_string += "~FM";
   }
   if( stat.foreground == ColorStatus::TURQUIOSE )
   {
      color_string += "~FT";
   }
   if( stat.foreground == ColorStatus::WHITE )
   {
      color_string += "~FW";
   }
   if( stat.background == ColorStatus::BLACK )
   {
      color_string += "~BK";
   }
   if( stat.background == ColorStatus::RED )
   {
      color_string += "~BR";
   }
   if( stat.background == ColorStatus::GREEN )
   {
      color_string += "~BG";
   }
   if( stat.background == ColorStatus::YELLOW )
   {
      color_string += "~BY";
   }
   if( stat.background == ColorStatus::BLUE )
   {
      color_string += "~BB";
   }
   if( stat.background == ColorStatus::MAGENTA )
   {
      color_string += "~BM";
   }
   if( stat.background == ColorStatus::TURQUIOSE )
   {
      color_string += "~BT";
   }
   if( stat.background == ColorStatus::WHITE )
   {
      color_string += "~BW";
   }

   return color_string;
}

pod_string add_colors( pod_string &input , ColorMap colors )
{
   int pos = 0;
   int insertpos = 0;
   int endpos = input.length();
   ColorMap::iterator colorStatus;
   ColorMap::iterator prevColorStatus;

   colorStatus  = colors.find( pos );
   if( colorStatus != colors.end() )
   {
      struct ColorStatus &currStatus = (*colorStatus).second;

      pod_string color_string = to_color_string( currStatus );
      input.insert(insertpos,color_string);
      insertpos += color_string.length();
   }
   prevColorStatus  = colorStatus;

   for(; pos < endpos; pos++, insertpos++ )
   {
      colorStatus  = colors.find( pos );
      if( colorStatus != colors.end() )
      {
         struct ColorStatus &currStatus = (*colorStatus).second;
         struct ColorStatus &prevStatus = (*prevColorStatus).second;
         pod_string color_string;

         if( currStatus.underline == false && prevStatus.underline == true ||
             currStatus.bold      == false && prevStatus.bold      == true ||
             currStatus.blink     == false && prevStatus.blink     == true ||
             currStatus.reverse   == false && prevStatus.reverse   == true ||
             currStatus.foreground == ColorStatus::UNSET && prevStatus.foreground != ColorStatus::UNSET ||
             currStatus.background == ColorStatus::UNSET && prevStatus.background != ColorStatus::UNSET )
         {
            color_string += "~RS";
            color_string += to_color_string( currStatus );
         }
         else
         {
            if( currStatus.underline == true && prevStatus.underline == false )
            {
               color_string += "~UL";
            }
            if( currStatus.bold == true && prevStatus.bold == false )
            {
               color_string += "~OL";
            }
            if( currStatus.blink == true && prevStatus.blink == false )
            {
               color_string += "~LI";
            }
            if( currStatus.reverse == true && prevStatus.reverse == false )
            {
               color_string += "~RV";
            }
            if( currStatus.foreground == ColorStatus::BLACK && prevStatus.foreground != ColorStatus::BLACK )
            {
               color_string += "~FK";
            }
            if( currStatus.foreground == ColorStatus::RED && prevStatus.foreground != ColorStatus::RED )
            {
               color_string += "~FR";
            }
            if( currStatus.foreground == ColorStatus::GREEN && prevStatus.foreground != ColorStatus::GREEN )
            {
               color_string += "~FG";
            }
            if( currStatus.foreground == ColorStatus::YELLOW && prevStatus.foreground != ColorStatus::YELLOW )
            {
               color_string += "~FY";
            }
            if( currStatus.foreground == ColorStatus::BLUE && prevStatus.foreground != ColorStatus::BLUE )
            {
               color_string += "~FB";
            }
            if( currStatus.foreground == ColorStatus::MAGENTA && prevStatus.foreground != ColorStatus::MAGENTA )
            {
               color_string += "~FM";
            }
            if( currStatus.foreground == ColorStatus::TURQUIOSE && prevStatus.foreground != ColorStatus::TURQUIOSE )
            {
               color_string += "~FT";
            }
            if( currStatus.foreground == ColorStatus::WHITE && prevStatus.foreground != ColorStatus::WHITE )
            {
               color_string += "~FW";
            }
            if( currStatus.background == ColorStatus::BLACK && prevStatus.background != ColorStatus::BLACK )
            {
               color_string += "~BK";
            }
            if( currStatus.background == ColorStatus::RED && prevStatus.background != ColorStatus::RED )
            {
               color_string += "~BR";
            }
            if( currStatus.background == ColorStatus::GREEN && prevStatus.background != ColorStatus::GREEN )
            {
               color_string += "~BG";
            }
            if( currStatus.background == ColorStatus::YELLOW && prevStatus.background != ColorStatus::YELLOW )
            {
               color_string += "~BY";
            }
            if( currStatus.background == ColorStatus::BLUE && prevStatus.background != ColorStatus::BLUE )
            {
               color_string += "~BB";
            }
            if( currStatus.background == ColorStatus::MAGENTA && prevStatus.background != ColorStatus::MAGENTA )
            {
               color_string += "~BM";
            }
            if( currStatus.background == ColorStatus::TURQUIOSE && prevStatus.background != ColorStatus::TURQUIOSE )
            {
               color_string += "~BT";
            }
            if( currStatus.background == ColorStatus::WHITE && prevStatus.background != ColorStatus::WHITE )
            {
               color_string += "~BW";
            }
         }
         input.insert(insertpos,color_string);
         insertpos += color_string.length();

         prevColorStatus  = colorStatus;
      }
   }

   return input;
}

pod_string reverse_words( pod_string &input )
{
//   printf("input:%s\n",input.c_str());

   ColorMap colors = convert_colors( input );
   ColorMap newColors;


   int endpos   = input.length();

   ColorStatus lastStat = colors[0];

   for(int pos = 0; pos < endpos; pos++ )
   {

      if( isalpha( input[pos] ) )
      {
         int wordpos = pos+1;
         while( isalpha( input[wordpos] ) && wordpos < endpos )
         {
            wordpos++;
         }
         pod_string word = input.substr(pos,wordpos-pos);
//         printf("word: %s\n",word.c_str());
         std::reverse(word.begin(), word.end());
         input.replace(pos,wordpos-pos,word);

         ColorMap::iterator colorStatus = colors.find( pos );
         if( colorStatus != colors.end() )
         {
            lastStat = (*colorStatus).second;
         }
         for( int col_pos = pos+1; col_pos < wordpos; col_pos++)
         {
            colorStatus  = colors.find( col_pos );
            if( colorStatus != colors.end() )
            {
//               printf("found: (%d)%s\n",col_pos,to_color_string( (*colorStatus).second ).c_str());
//               printf("last: %s\n",to_color_string( lastStat ).c_str());
               newColors[pos+(wordpos-col_pos)] = lastStat;
               lastStat = (*colorStatus).second;
            }
         }
         newColors[pos] = lastStat;
         pos += (wordpos-pos);
         newColors[pos] = lastStat;
      }
      ColorMap::iterator colorStatus = colors.find( pos );
      if( colorStatus != colors.end() )
      {
//         printf("found- (%d)%s\n",pos,to_color_string( (*colorStatus).second ).c_str());
         newColors[pos] = (*colorStatus).second;
         lastStat = (*colorStatus).second;
      }
   }

   add_colors( input, newColors );

//   printf("output:%s\n",input.c_str());

   return input;
}


pod_string curse_rewrite(UR_OBJECT user,pod_string input)
{
   struct curse_entry *curse_ptr;
   int curse_num = 0;

   if( !user->cursed )
   {
      return input;
   }

   while( strcmp( curse_book[curse_num].name, "*" ) )
   {
      if( !strcmp( curse_book[curse_num].name, user->curse_name ) )
      {
         break;
      }
      curse_num++;
   }

   if( !strcmp( curse_book[curse_num].name, "*" ) )
   {
      return input;
   }

   curse_ptr = &curse_book[curse_num];

   append_each_word( input, curse_ptr->aft_str );
   if( curse_ptr->replace_char_percent != 0 && curse_ptr->replace_char != '\0' )
   {
      replace_random( input, curse_ptr->replace_char_percent , curse_ptr->replace_char );
   }
   if( curse_ptr->mess_color )
   {
      randomize_colors( input );
   }
   if( curse_ptr->reverse )
   {
      reverse_words( input );
   }

   return input;
}

struct curse_entry *get_curse(UR_OBJECT user)
{
   int curse_num=0;
   struct curse_entry *curse_ptr;

   if(!user->cursed) return NULL;

   while(strcmp(curse_book[curse_num].name,"*")) 
   {
      if(!strcmp(curse_book[curse_num].name,user->curse_name)) break;
      curse_num++;
   }   
   if(!strcmp(curse_book[curse_num].name,"*"))  return NULL;
   curse_ptr = &curse_book[curse_num];

   return curse_ptr;
}

int curse_is_bound(UR_OBJECT user)
{
   struct curse_entry *curse_ptr;

   if((curse_ptr = get_curse(user)) == NULL) return 0;

   if(!curse_ptr->bind_to_room) return 0;

   write_user(user,curse_ptr->bind_to_room_local_str);
   write_room_exceptf(user->room,user,curse_ptr->bind_to_room_room_str,get_visible_name(user).c_str());
   return 1;
}
