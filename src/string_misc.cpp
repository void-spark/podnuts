#include <sstream>
#include <iomanip>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <algorithm>
#include <sys/types.h>
#include <netinet/in.h>
#include "pod_string.h"
#include "time_utils.h"
#include "logging.h"
#include "color.h"
#include "StringLibrary.h"
#include "string_misc.h"

bool isNonPrintingOrWS( char c )
{
   return c < 33;
}

bool isNonPrinting( char c )
{
   return c < 32;
}

bool isBackOrDel( char c )
{
   return c == 8 || c == 127;
}

char *swear_words[]=
{
   "fuck",
   "shit",
   "cunt",
   "*"
};


pod_string intToString(int i) // convert int to string
{
   pod_ostringstream s;
   s << i;
   return s.str();
}

int stringToInt( pod_string str )
{
   pod_istringstream s(str);
   int i;
   s >> i;
   return i;
}

pod_string floatToString(float f)
{
   pod_ostringstream s;
   s << f;
   return s.str();
}

float stringToFloat( pod_string str)
{
   pod_istringstream s(str);
   float f;
   s >> f;
   return f;
}


pod_string gen_seperator_line(pod_string string)
{
   StringLibrary * stringLibrary = StringLibrary::getInstance();
   pod_string line_holder = stringLibrary->makeString("seperator_line");
   const unsigned int name_offset = 6;
   unsigned int out_pos = 0;
   unsigned int inp_pos;
   unsigned int inp_size = string.size();
   unsigned int cnt;

   if( !string.empty() )
   {
      for( cnt = 0; cnt < name_offset; cnt++ )
      {
         while( is_color( line_holder, out_pos ) )
         {
            out_pos += 3;
         }
         out_pos++;
      }
      for( inp_pos = 0; inp_pos < inp_size; inp_pos++)
      {
         while( is_color( line_holder, out_pos ) )
         {
            out_pos += 3;
         }
         line_holder[out_pos++] = string[inp_pos];
      }
   }
   return line_holder;
}

/*** Convert string to upper case ***/
void strToUpper(pod_string &str)
{
   transform (str.begin(),str.end(), str.begin(), toupper);

   return;
}

/*** Convert string to lower case ***/
void strToLower(pod_string &str)
{
   transform (str.begin(),str.end(), str.begin(), tolower);

   return;
}

void capitalize(pod_string &str)
{
   str[0]=toupper(str[0]);
}


/*** Convert string to lower case ***/
int strtolower(char *str)
{
   while(*str) 
   {  
      *str=tolower(*str);
      str++;
   }

   return 0;
}

/*** Returns 1 if string is a positive number ***/
int isNumber(char *str)
{
   while(*str) if (!isdigit(*str++)) return 0;
   return 1;
}

pod_string time2string(int show_seconds,time_t my_time)
{
   int secs,days,hours,mins/*,years*/;
   pod_string timeString;
   char my_str[80];

/*   years=my_time/31536000;*/
   days=(my_time/*%31536000*/)/86400;
   hours=(my_time%86400)/3600;
   mins=(my_time%3600)/60;
   secs=my_time%60;
   
   if(show_seconds)
   {
/*      if(years) sprintf(my_str,"%d years, %d days, %d hours, %d minutes, %d seconds",years,days,hours,mins,secs);*/
/*      else*/ if(days) sprintf(my_str,"%d days, %d hours, %d minutes, %d seconds",days,hours,mins,secs);
      else sprintf(my_str,"%d hours, %d minutes, %d seconds",hours,mins,secs);
   }
   else
   {
/*      if(years) sprintf(my_str,"%d yrs, %d days, %d hours, %d mins",years,days,hours,mins);*/
/*      else*/ if(days) sprintf(my_str,"%d days, %d hours, %d minutes",days,hours,mins);
      else sprintf(my_str,"%d hours, %d minutes",hours,mins);
   }
   
   timeString = my_str;
   return timeString;
}

/*** Return pos. of second word in inpstr ***/
char *remove_first(char *inpstr)
{
   char *pos=inpstr;
   while(*pos<33 && *pos) ++pos;
   while(*pos>32) ++pos;
   while(*pos<33 && *pos) ++pos;
   return pos;
}

const char *const_remove_first(const char *inpstr)
{
   const char *pos=inpstr;
   while(*pos<33 && *pos) ++pos;
   while(*pos>32) ++pos;
   while(*pos<33 && *pos) ++pos;
   return pos;
}

pod_string remove_first( pod_string input )
{
   pod_string::size_type pos = 0;
   pod_string::size_type len = input.size();

   while( pos < len &&  isNonPrintingOrWS( input[pos] ) )
   {
      pos++;
   }
   while( pos < len && !isNonPrintingOrWS( input[pos] ) )
   {
      pos++;
   }
   while( pos < len &&  isNonPrintingOrWS( input[pos] ) )
   {
      pos++;
   }

   return input.substr( pos, len - pos );
}

/*
 * Terminate string before first nonprinting char
 */
int terminate(pod_string &str)
{
   pod_string::size_type pos;

   for (pos=0;pos<str.size();pos++)
   {
      if ( isNonPrinting( str[pos] ) )
      {
         str = str.substr(0,pos);
         return 0;
      }
   }
   return 0;
}

int strxcmp(const char *s1, const char *s2)
{
   return strncmp(s1,s2,strlen(s2));
}

pod_string stripColorCodes( pod_string str )
{
   pod_string stripped;
   
   pod_string::size_type pos = 0;

   while( pos < str.size() )
   {
      if( is_color(str,pos) )
      {
         pos += 3;
      }
      else
      {
         stripped += str[pos];
         pos++;
      }
   }

   return stripped;      
}

pod_string color_com_strip( pod_string str )
{
   return stripColorCodes(str);
}

/* strip ANSI code, spooned from somewhere */
/*
static void str_ansi( char *dst, char *str, int max)
{
   int ch, ansi;
   char *tail;

   for (ansi = 0, tail = dst + max - 2; ch = *str; str++)
   {
      if (ch == '\n')
      {
         break;
      }
      else if (ch == 27)
      {
         ansi = 1;
      }
      else if (ansi)
      {
         if ((ch < '0' || ch > '9') && ch != ';' && ch != '[')
         ansi = 0;
      }
      else
      {
         *dst++ = ch;
         if (dst >= tail)
         {
            break;
         }
      }
   }
   *dst = '\0';
}
*/

pod_string str_ansi( pod_string str )
{
   pod_string dst;
   char ch;
   int ansi;
   unsigned int pos;

   for (ansi = 0,pos = 0; pos < str.size(); pos++)
   {
      ch = str[pos];
      if (ch == '\n' || ch == '\r')
      {
         break;
      }
      else if (ch == 27)
      {
         ansi = 1;
      }
      else if (ansi)
      {
         if ((ch < '0' || ch > '9') && ch != ';' && ch != '[')
         ansi = 0;
      }
      else
      {
         dst.push_back(ch);
      }
   }

   return dst;
}

int is_alpha_str(char *str)
{
   unsigned int i;

   for (i=0;i<strlen(str);++i) 
   {
      if (!isalpha(str[i])) 
      {
         return 0;
      }
   }

   return 1;
}

/*pod_string wordWrap( const pod_string input, int width)
{
   pod_ostringstream outputStream;
   int cnt = 0;
   const char *ptr = 0;

   for(ptr = input.c_str(); *ptr != '\0'; ptr++)
   {
      outputStream << *ptr;
      if (*ptr=='\n')
      {
         cnt = 0;
      }
      else
      {
         cnt++;
      }
      if (cnt == width)
      {
         outputStream << '\n';
         cnt=0;
      }
   }
   return outputStream.str();
}*/


/*
   "~FRtest~FWjehmm~FRbye~FG~FG"(27)
   
   t:0 -> 3
   e:1 -> 4
   s:2 -> 5
   t:3 -> 6
   j:4 -> 10
   e:5 -> 11
   h:6 -> 12
   m:7 -> 13
   m:8 -> 14
   b:9 -> 18
   y:10 -> 19
   e:11 -> 20
*/

int toColorIndex( pod_string str, int plainIndex )
{
   int colorIndex = 0;
   
   while( is_color( str, colorIndex) )
   {
      colorIndex += 3;
   }         
      
   for(int cnt = 0 ; cnt < plainIndex ; cnt++)
   {
      colorIndex++;
      while( is_color( str, colorIndex) )
      {
         colorIndex += 3;
      }         
   }
   return colorIndex;   
}

int toColorOffset( pod_string str, int plainOffset )
{
   if( plainOffset < 0 )
   {
      abort();
   }   
   
   if( plainOffset == 0 )
   {
      return 0;
   }
   
   return toColorIndex(str, plainOffset - 1 ) + 1;
}

/*   
   space,  
   form-feed  ('\f'),  newline  ('\n'),
   carriage  return ('\r'), 
   horizontal tab ('\t'), 
   and vertical tab ('\v').
*/
const char* wrapWhitespace = " \f\n\r\t\v";

pod_string::size_type findCutOffset( pod_string line, pod_string::size_type maxLineWidth )
{
   pod_string::size_type  lastSpace;

   // No wrapping needed   
   if ( line.size() <= maxLineWidth )
   {
      return line.size();
   }
   
   // Find last space, we wrap just behind the last space
   lastSpace = line.find_last_of( wrapWhitespace, maxLineWidth );   

   // Wrap just behind the last space, if there is one.
   if ( lastSpace != pod_string::npos && 
        lastSpace != maxLineWidth )
   {
      return lastSpace + 1;         
   }

   // There was no space we could wrap on, or the line after our
   // wrap length starts with a space. We wrap at the maximum length
   return maxLineWidth;
}

/* This function expects a line without newlines! */
pod_string wrapLine( const pod_string input, int maxLineWidth )
{
   pod_string::size_type  actualLineWidth;
   pod_string::size_type  actualLineWidthColors;
   
   pod_string buffer( input );
   pod_string strippedBuffer( stripColorCodes( input ) );
   pod_string output;
    
   if ( maxLineWidth <= 0) 
   {
      abort();   
   }

   while( !strippedBuffer.empty() )      
   { 
      actualLineWidth       = findCutOffset( strippedBuffer, maxLineWidth );
      actualLineWidthColors = toColorOffset( buffer, actualLineWidth );
      
      output += buffer.substr( 0, actualLineWidthColors );
 
      strippedBuffer.erase( 0, actualLineWidth );
      buffer.erase( 0, actualLineWidthColors );      

      if(!strippedBuffer.empty())
      {      
         output += "\n";
      }
   }
   
   return output;
}

/* splits input into its newline delimited parts, and feeds them to wrapLine
   newlines are stripped before feeding the lines to wrapLine, but are put 
   back in the output */
pod_string wordWrap( const pod_string input, int maxLineWidth)
{
   pod_string::size_type  start = 0;
   pod_string::size_type  end = input.find_first_of("\n\r",start);
   
   pod_string output;
   
   while( end != pod_string::npos )
   {
      output += wrapLine(input.substr(start,end-start),maxLineWidth);      
      output += input[end];
      start = end + 1;
      end = input.find_first_of("\n\r",start);
   }
   output += wrapLine(input.substr(start,end-start),maxLineWidth);
   
   return output;
}

pod_string longDateFromTime( time_t timeValue )
{
   struct tm *tm_struct;
   tm_struct = localtime(&timeValue);
   /* these two static because poor compy would have to recreate them everytime othwerwise :) */
   static char *month_names[12] = {
                               "January",
                               "February",
                               "March",
                               "April",
                               "May",
                               "June",
                               "July",
                               "August",
                               "September",
                               "October",
                               "November",
                               "December"
                            };

   static char *day_names[7] =    {
                               "Sunday",
                               "Monday",
                               "Tuesday",
                               "Wednesday",
                               "Thursday",
                               "Friday",
                               "Saturday"
                            };

   pod_ostringstream outputStream;

   int wday  = tm_struct->tm_wday;
   int mday  = tm_struct->tm_mday;
   int month = tm_struct->tm_mon;
   int year  = tm_struct->tm_year + 1900;
   int hour  = tm_struct->tm_hour;
   int min   = tm_struct->tm_min;

   // "Tuesday 12 August 2002 at 12:12"
   outputStream << day_names[wday] << " " << mday << " " << month_names[month] << " " << year << " at "
                << std::setfill('0') << std::setw(2) << hour << ":" << std::setw(2) << min;

   return outputStream.str();
}

/*** Generates several date strings, showing the current local time ***/
pod_string long_date(int which)
{
   struct tm *tm_struct;
   tm_struct = get_current_localtime();

   /* these two static because poor compy would have to recreate them everytime othwerwise :) */
   static char *month_names[12] = {
                               "January",
                               "February",
                               "March",
                               "April",
                               "May",
                               "June",
                               "July",
                               "August",
                               "September",
                               "October",
                               "November",
                               "December"
                            };  

   static char *day_names[7] =    {
                               "Sunday",
                               "Monday",
                               "Tuesday",
                               "Wednesday",
                               "Thursday",
                               "Friday",
                               "Saturday"
                            };

   pod_ostringstream outputStream;

   int wday  = tm_struct->tm_wday;
   int mday  = tm_struct->tm_mday;
   int month = tm_struct->tm_mon;
   int year  = 1900+tm_struct->tm_year;
   int hour  = tm_struct->tm_hour;
   int min   = tm_struct->tm_min;
   int sec   = tm_struct->tm_sec;

   // 1 : "on Tuesday 12 August 2002 at 12:12"
   // 2 : "Tuesday 12 August 2002 at 12:12"
   if ( which == 1 || which == 2  )
   {
      if      ( which == 1 )
      {
         outputStream << "on ";
      }
      outputStream << day_names[wday] << " " << mday << " " << month_names[month] << " " << year << " at "
                   << std::setfill('0') << std::setw(2) << hour << ":" << std::setw(2) << min;
   }
   // 3 : "31/11 12:12:01"
   else if ( which == 3 )
   {
      outputStream << std::setfill('0')
                   << std::setw(2) << mday    << "/"
                   << std::setw(2) << month+1 << " "
                   << std::setw(2) << hour    << ":"
                   << std::setw(2) << min     << ":"
                   << std::setw(2) << sec;
   }
   else
   {
      abort();
   }

   return outputStream.str();
}

/*** See if string contains any swearing ***/
int contains_swearing( pod_string str )
{
   int i;
   pod_string copy;

   copy = str;

   strToLower(copy);

   i=0;
   while( swear_words[i][0] != '*' )
   {
      if( copy.find(swear_words[i]) != pod_string::npos )
      {
         return 1;
      }
      ++i;
   }

   return 0;
}
