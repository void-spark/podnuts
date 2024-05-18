#include <iostream>
#include "pod_string.h"
#include "string_misc.h"
#include "CRTLineBuffer.h"

/* extract a \n terminated line from the input,
   pastes together non terminated pieces to form a string
   we can get non terminated pieces from character mode clients :
   "a" "b" "c" "/n" -> "abc\n"
   or from cut up tcp-ip packets :
   "abchfsk" "adad" "afafjkaf\n" -> "abchfskadadafafjkaf\n"
   also this function clips input at ARR_SIZE to prevent overflows,
   where the clipped portion gets reparsed as a next line of input :
   (say ARR_SIZE = 11)
   "abcde" "fg" "hijklm\n" -> "abcdefghij\n" "klm\n" */

CRTLineBuffer::CRTLineBuffer( )
{
   debug = false;
   hasLine = false;
}

// returns echo, if any
pod_string CRTLineBuffer::putData( pod_string input )
{
   int        inputSize   = input.size();
   int        pos         = 0;
   pod_string echo;

   if(debug) { std::cout << "Input size  : " << inputSize  << std::endl; }
   if(debug) { std::cout << "Input: '" << input << "'" << std::endl; }
   if(debug)
   {
      std::cout << "Input: '";
      for(int p = 0; p < input.size();p++)
      {
         std::cout  << "[" << (int)input[p] << "]";
      }
      std::cout  << "'" << std::endl;
   }

   if(debug) { std::cout << "Buffer size : " << buffer.size()  << std::endl; }

   for( pos = 0; pos < inputSize; pos++ )
   {
      // vt220 delete code = [ESC][3~
      if( input.substr( pos, 4 ) == "\33[3~" )
      {
         if(debug) { std::cout << "vt220 delete escape entered." << std::endl; }
         if( buffer.size() != 0 )
         {
            buffer.resize( buffer.size() - 1 );
            echo += "\b \b";
         }
         pos += 3;
         continue;
      }
   
      if( isBackOrDel( input[ pos ] ) )
      {
         if(debug) { std::cout << "Delete entered." << std::endl; }
         if( buffer.size() != 0 )
         {
            buffer.resize( buffer.size() - 1 );
            echo += "\b \b";
         }
         continue;
      }

      buffer.push_back( input[ pos ] );
      echo.push_back( input[ pos ] );
   }

   checkForLine();
   
   return echo;
}

void CRTLineBuffer::checkForLine()
{
   pod_string result;
   int        bufferSize  = buffer.size();
   int        pos         = 0;
   int        size        = 0;
   
   // Already gots us a line
   if( hasLine )
   {
      return;
   }

   if(debug) { std::cout << "Buffer size : " << bufferSize << std::endl; }

   for( pos = 0; pos < bufferSize; pos++ )
   {
      size = pos + 1;

      if( isNonPrinting( buffer[ pos ] ) ) /* End of line */
      {
         if(debug) { std::cout << "End of line." << std::endl; }
         result = buffer.substr(0,size);
         buffer.erase(0,size);
         terminate( result );
         line = result;
         hasLine = true;
         break;
      }

      if( size == ARR_SIZE-1 ) /* Overflow */
      {
         if(debug) { std::cout << "Overflow." << std::endl; }
         result = buffer.substr(0,size);
         buffer.erase(0,size);
         terminate( result );
         line = result;
         hasLine = true;
         break;
      }
   }
   
}

bool CRTLineBuffer::hasNextLine()
{
   return hasLine;
}

pod_string CRTLineBuffer::getLine( )
{
   if( !hasLine )
   {
      std::cerr << "Tried to get line when none available." << std::endl;   
      abort();
   }
   
   if(debug) { std::cout << "Line size  : " << line.size()  << std::endl; }
   if(debug) { std::cout << "Line: '" << line << "'" << std::endl; }
   if(debug)
   {
      std::cout << "Line: '";
      for(int p = 0; p < line.size();p++)
      {
         std::cout  << "[" << (int)line[p] << "]";
      }
      std::cout  << "'" << std::endl;
   }
   
   hasLine = false;
   pod_string result = line;

   checkForLine();
      
   return result;
}
