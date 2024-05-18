#include <string.h>
#include <stdlib.h>
#include "pod_string.h"
#include "color.h"

#define NUM_COLS       21

struct color_type
{
   /* ansi codes used to produce a color */
   char *telnet_code;
   /* Codes used in a string to produce the colors when prepended with a '~' */
   char *talker_code;
};

struct color_type col_codes[NUM_COLS] = 
                       {  /* Standard stuff */
                          { "\033[0m",  "RS" }, /* RESET */
                          { "\033[1m",  "OL" }, /* BOLD */
                          { "\033[4m",  "UL" }, /* UNDERLINE */
                          { "\033[5m",  "LI" }, /* BLINK */
                          { "\033[7m",  "RV" }, /* REVERSE */
                          /* Foreground color */
                          { "\033[30m", "FK" }, /* BLACK */
                          { "\033[31m", "FR" }, /* RED */
                          { "\033[32m", "FG" }, /* GREEN */
                          { "\033[33m", "FY" }, /* YELLOW/ORANGE */
                          { "\033[34m", "FB" }, /* BLUE */
                          { "\033[35m", "FM" }, /* MAGENTA */
                          { "\033[36m", "FT" }, /* TURQUIOSE */
                          { "\033[37m", "FW" }, /* WHITE */
                          /* Background color */
                          { "\033[40m", "BK" }, /* BLACK */
                          { "\033[41m", "BR" }, /* RED */
                          { "\033[42m", "BG" }, /* GREEN */
                          { "\033[43m", "BY" }, /* YELLOW/ORANGE */
                          { "\033[44m", "BB" }, /* BLUE */
                          { "\033[45m", "BM" }, /* MAGENTA */
                          { "\033[46m", "BT" }, /* TURQUIOSE */
                          { "\033[47m", "BW" }};/* WHITE */
                          /* Special colors */
//                          { "\033[2J",  "CL" }};/* Clear Screen */

int is_color( pod_string str, int pos )
{
   if( ( str.length() - pos ) < 3 || str[0 + pos] != '~' )
   {
      return 0;
   }
   for(int cnt=0; cnt < NUM_COLS; cnt++ )
   {
      if( !str.compare( 1 + pos , 2, col_codes[cnt].talker_code, 2 ) )
      {
         return 1;
      }
   }
   return 0;
}

char* decode_color(const char* col_code)
{   
   char* found_color_code=NULL;
   int cnt;

   for(cnt=0;cnt<NUM_COLS;cnt++)
   {
      if ( !strncmp(col_code,col_codes[cnt].talker_code,2))
      {
         found_color_code=col_codes[cnt].telnet_code;
         break;
      }
   }
   return found_color_code;
}

/*** Count the number of color commands in a string ***/
int color_com_count(pod_string str)
{
   int cnt = 0;
   pod_string::size_type pos = 0;
   pod_string::size_type length = str.length();

   while(pos < length)
   {
      if ( str[pos] == '~' )
      {
         pos++;
         for(int i=0;i<NUM_COLS;++i)
         {
            if( !str.compare(pos,2,col_codes[i].talker_code) )
            {
               cnt++;
               pos++;
               continue;
            }
         }
         continue;
      }
      pos++;
   }
   return cnt;
}


pod_string get_random_color()
{
   pod_string code;
   char *ptr;

   do
   {
      int pick = random()%NUM_COLS;
      ptr = col_codes[pick].talker_code;
   } while(!strcmp(ptr,"LI"));

   code = "~";
   code += ptr;
   return code;
} 
