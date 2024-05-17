#include <string>
#include <string.h>
#include "general_headers.h"
#include "string_misc.h"
#include "cmd_main.h"
#include "macro.h"
#include "pod_string.h"
#include "parse.h"

typedef std::vector<pod_string, pod_alloc< pod_string >::Type > StringsVector;

StringsVector split_string( const pod_string& input )
{
   StringsVector strings;
   pod_string::size_type head = 0;
   pod_string::size_type searchPos = 0;
   pod_string::size_type separatorPos = 0;

   while ( true )
   {
      separatorPos = input.find( INPUT_SEP, searchPos );

      if( separatorPos == pod_string::npos )
      {
         strings.push_back( input.substr(head, input.size() - head) );
         break;
      }
      if ( input[ separatorPos + 1 ] == INPUT_SEP )
      {
         searchPos = separatorPos + 2;
         continue;
      }
      strings.push_back( input.substr(head, separatorPos - head) );
      searchPos = separatorPos + 1;
      head = searchPos;
   }

   return strings;
}


/* returns a string with all the double seperators "\\" replaced with
   single ones "\" */
pod_string fix_separators( pod_string input )
{
   pod_string output;
   pod_string::size_type pos = 0;

   while( input[ pos ] )
   {
      if ( input[ pos ] == INPUT_SEP && input[ pos + 1 ] == INPUT_SEP )
      {
         pos++;
      }
      output += input[ pos++ ];
   }

   return output;
}


void pod_expand(UR_OBJECT user, pod_string input )
{
   words_struct *inputWords;

   inputWords = wordfind( input.c_str() );
   if( inputWords->word_count )
   { 
      if( user->macrolist.macroExpand( user ) ||
          system_macrolist.macroExpand( user ) ||
          ( (user->level >= MINLEVELACTIONS) ? system_actionlist.macroExpand( user ) : 0 ) )
      {
      
      }
      else
      {
         exec_com( user, fix_separators( input ) );
      }
   }
}

void parse( UR_OBJECT user, pod_string input )
{
   StringsVector inputLines = split_string( input );
   StringsVector::size_type size = inputLines.size();   

   for( StringsVector::size_type cnt = 0; cnt < size; cnt++ )
   {
      pod_expand( user, inputLines[cnt] );
      if( curr_user_destructed )
      {
         break;
      }
   }
}


