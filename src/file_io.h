#ifndef FILE_IO_H
#define FILE_IO_H

#include <fstream>
#include "pod_string.h"

class PodFile
{
   public:
      PodFile(pod_string filename);
      ~PodFile();

      pod_string   getFilename();

      pod_string   readLine();
      void         writeLine(pod_string line);

      pod_string   fgets();
      void         fputs(pod_string string);

      int          setPosition( unsigned int pos);
      unsigned int getPosition();

      int      open_write();
      int      open_read();
      int      open_append();
      int      close();
      std::iostream &getStream();

      int     eof()
      {
         return _stream.eof();
      };
      void toNextLine();

   protected:
      pod_string         _filename;
      std::fstream       _stream;
      bool               _isOpen;

      void skipSpaceAndTabs();
};
#endif /* !FILE_IO_H */
