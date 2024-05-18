#include <iostream>
#include "pod_string.h"
#include "logging.h"
#include "stdio.h"
#include "file_io.h"

pod_string PodFile::getFilename()
{
   return _filename;
}

PodFile::PodFile(pod_string filename)
{
   _filename = filename;
   _isOpen   = false;
}

PodFile::~PodFile()
{
   if(_isOpen)
   {
      write_syslogf("Error: tried to delete open file \"%s\".\n",true,_filename.c_str());
      abort();
   }
}

std::iostream &PodFile::getStream()
{
   return _stream;
}

int PodFile::open_write()
{
   if(_isOpen)
   {
      write_syslogf("Error: tried to open already open file \"%s\".\n",true,_filename.c_str());
      return 1;
   }

   _stream.open(_filename.c_str(),std::ios_base::out);

   if( _stream.fail() )
   {
      write_syslogf("Error while opening file: \"%s\" for writing.\n",true,_filename.c_str());
      return 1;
   }

   _isOpen=true;
   return 0;
}

int PodFile::open_append()
{
   if(_isOpen)
   {
      write_syslogf("Error: tried to open already open file \"%s\".\n",true,_filename.c_str());
      return 1;
   }

   _stream.open(_filename.c_str(),std::ios_base::out|std::ios_base::app);
   if( _stream.fail() )
   {
      write_syslogf("Error while opening file: \"%s\" for writing.\n",true,_filename.c_str());
      return 1;
   }

   _isOpen=true;
   return 0;
}

int PodFile::open_read()
{
   if(_isOpen)
   {
      write_syslogf("Error: tried to open already open file \"%s\".\n",true,_filename.c_str());
      return 1;
   }

   _stream.open(_filename.c_str(),std::ios_base::in);
   if( _stream.fail() )
   {
      write_syslogf("Error while opening file: \"%s\" for reading.\n",true,_filename.c_str());
      return 1;
   }

   _isOpen=true;
   return 0;
}

int PodFile::close()
{
   if(!_isOpen)
   {
      write_syslogf("Error: tried to close unopen file \"%s\".\n",true,_filename.c_str());
      return 1;
   }
   _stream.close();

   _isOpen=false;
   return 0;
}

void PodFile::toNextLine()
{
   while( (_stream.get() != '\n') && !_stream.eof() );
}

void PodFile::skipSpaceAndTabs()
{
   char in;

   while( (in = _stream.peek()) == ' ' || in == '\t' )
   {
      _stream.get();
      if(_stream.eof())
      {
         break;
      }
   }
}

int PodFile::setPosition( unsigned int pos)
{
   _stream.seekg(pos, std::ios::beg);

   return 0;
}

unsigned int PodFile::getPosition()
{
   return _stream.tellg();
}

/*
 * Reads a line from the file, up to '\n' or eof.
 * A delemiting '\n' is included when found.
 */
pod_string PodFile::fgets()
{
   char nextChar;
   pod_stringbuf buffer(std::ios_base::in|std::ios_base::out);

   if( !_stream.get(buffer) )
   { // weird, if nothing to get, the stream is set to fail (Empty line + crt 'd do that, get doesn't read the delim btw.!).
      if( _stream.fail() )
      {
         _stream.clear();
      }
      else if( _stream.bad() )
      {
         abort();
      }
   }

   // get delim
   _stream.get(nextChar);

   if( _stream.eof() )
   {
      return buffer.str();
   }

   return (buffer.str() + nextChar);
}

/*
 * Reads a line from the file, up to '\n' or eof.
 * A delemiting '\n' is discarded when found.
 */
pod_string PodFile::readLine()
{
   char nextChar;
   pod_stringbuf buffer(std::ios_base::in|std::ios_base::out);

   if( !_stream.get( buffer ) )
   { // weird, if nothing to get, the stream is set to fail (Empty line + crt 'd do that, get doesn't read the delim btw.!).
      if( _stream.fail() )
      {
         _stream.clear();
      }
      else if( _stream.bad() )
      {
         abort();
      }
   }

   // read in terminating newline, if there is none this sets eof
   _stream.get(nextChar);
   // peek past the newline, if there's nothign this was the last line and we set eof as well
   if( _stream.peek() == EOF )
   {
      _stream.get(nextChar);
   }
   return buffer.str();
}

void PodFile::writeLine( pod_string line )
{
   _stream << line << std::endl;
}

void PodFile::fputs( pod_string string )
{
   _stream << string;
}
