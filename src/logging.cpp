#include <stdarg.h>
#include "file_io.h"
#include "file_locations.h"
#include "pod_string.h"
#include "string_misc.h"
#include "logging.h"

#define BUFFER_SIZE 2049

bool loggingOn = true;

void setLoggingOn( bool val )
{
   loggingOn = val;
}

void writeLogfile( pod_string filename, bool write_time, pod_string string )
{
   pod_string path = LOGFILES;
   path += "/";
   path += filename;

   PodFile output(path);

   if ( !loggingOn || output.open_append() )
   {
      return;
   }
   if (!write_time)
   {
      output.fputs( string );
   }
   else
   {
      output.fputs( long_date(3) + ": " + string );
   }
   output.close();

   return;
}

void write_syslogf(char *str, bool write_time, ...)
{
   static char buffer[BUFFER_SIZE*2];
   va_list args;

   buffer[0]='\0';
   va_start(args,write_time);
   vsnprintf(buffer,BUFFER_SIZE*2,str,args);
   va_end(args);
   write_syslog(buffer, write_time);
}

int write_syslog(char *str, bool write_time)
{
   pod_string filename = LOGFILES;
   filename += "/";
   filename += SYSLOG;

   PodFile output(filename);

   if ( !loggingOn || output.open_append() )
   {
      return 0;
   }
   if (!write_time)
   {
      output.fputs( str );
   }
   else
   {
      output.fputs( long_date(3) + ": " + str );
   }
   output.close();

   return 0;
}

WriteLogBuff::WriteLogBuff() : pod_stringbuf( std::ios_base::out )
{
   _withTime = true;
}

void WriteLogBuff::setLogfile( pod_string logfile )
{
   _logfile = logfile;
}

void WriteLogBuff::setWithTime(bool withTime)
{
   _withTime = withTime;
}

int WriteLogBuff::overflow( int character )
{
   return pod_stringbuf::overflow(character);
}

void WriteLogBuff::reset()
{
   _logfile = "";
   _withTime = true;
}

std::streamsize WriteLogBuff::xsputn( const char* s, std::streamsize n )
{
   std::streamsize retval = 0;
   const char* nullChar = (const char*)memchr(s, '\0', n);
   const char* curChar = s;
   const char* endChar = s + n;


   if(!nullChar)
   {
      return pod_stringbuf::xsputn(s,n);
   }
   else
   {
      while(true)
      {
         int pieceLength =  nullChar - curChar;

         retval += pod_stringbuf::xsputn(curChar,pieceLength);
         retval += 1;


         writeLogfile( _logfile, _withTime, str() );

         reset();
         str("");
         if(retval == n)
         {
            break;
         }
         curChar = nullChar + 1;
         int remainderLength = endChar - curChar;
         nullChar = (const char*)memchr(curChar, '\0', remainderLength);
         if(!nullChar)
         {
            retval += pod_stringbuf::xsputn(curChar, remainderLength);
            break;
         }
      }
      return retval;
   }
}

WriteLogStream::WriteLogStream() : std::ostream(&_streambuffer), _streambuffer()
{
}

void WriteLogStream::setLogfile(pod_string logfile)
{
   _streambuffer.setLogfile(logfile);
}

void WriteLogStream::setWithTime(bool withTime)
{
   _streambuffer.setWithTime(withTime);
}

std::ostream& operator<<( std::ostream&  __os, _SetLogfile __logfile)
{
   WriteLogStream* writeLogStream = dynamic_cast< WriteLogStream* > ( &__os );
   writeLogStream->setLogfile(__logfile._logfile);

   return __os;
}

std::ostream& operator<<( std::ostream&  __os, _SetWithTime __withtime)
{
   WriteLogStream* writeLogStream = dynamic_cast< WriteLogStream* > ( &__os );
   writeLogStream->setWithTime(__withtime._withTime);

   return __os;
}

std::ostream& noTime(std::ostream& __os)
{
   WriteLogStream* writeLogStream = dynamic_cast< WriteLogStream* > ( &__os );
   writeLogStream->setWithTime(false);

   return __os;
}

WriteLogStream logStream;
