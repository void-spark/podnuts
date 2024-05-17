#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "../general_headers.h"
#include "../logging.h"
#include "../string_misc.h"
#include "../pod_string.h"
#include "../help.h"
#include "../more.h"
#include "logviewer_subsystem.h"

int view_log(UR_OBJECT user, char *inpstr);

extern "C" void plugin_init()
{
   cmd_add("viewlog",   LEV_THR, ADM, &view_log);
}

//////////////////TAIL/////////////////////
/* Stolen from tail from gnu textutils */

#ifndef MIN
# define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif

ssize_t safe_read (int desc, void *ptr, size_t len)
{
   ssize_t n_chars;
   if (len <= 0)
   {
      return len;
   }

   do
   {
      n_chars = read (desc, ptr, len);
   }
   while (n_chars < 0 && errno == EINTR);

   return n_chars;
}

static void xwrite (FILE *output, char *const buffer, size_t n_bytes)
{
   if (n_bytes > 0 && fwrite (buffer, 1, n_bytes, output) == 0)
   {
      write_syslogf("Error in xwrite: '%s'.\n",TRUE,strerror(errno));
      abort();
   }
}

/* Read and output N_BYTES of file PRETTY_FILENAME starting at the current
   position in FD.  If N_BYTES is COPY_TO_EOF, then copy until end of file.
   Return the number of bytes read from the file.  */

static long dump_remainder(FILE *output, int fd, off_t n_bytes)
{
   char buffer[BUFSIZ];
   int bytes_read;
   long n_written;
   off_t n_remaining = n_bytes;

   n_written = 0;
   while (1)
   {
      long n = MIN (n_remaining, (off_t) BUFSIZ);
      bytes_read = safe_read (fd, buffer, n);
      if (bytes_read <= 0)
      {
         break;
      }
      xwrite (output, buffer, bytes_read);
      n_remaining -= bytes_read;
      n_written += bytes_read;
   }
   if (bytes_read == -1)
   {
      write_syslogf("Error in dump_remainder: '%s'.\n",TRUE,strerror(errno));
      abort();
   }

   return n_written;
}

   /* Print the last N_LINES lines from the end of file FD.
   Go backward through the file, reading `BUFSIZ' bytes at a time (except
   probably the first), until we hit the start of the file or have
   read NUMBER newlines.
   FILE_LENGTH is the length of the file (one more than the offset of the
   last byte of the file).
   Return 0 if successful, 1 if an error occurred.  */
   /* mods: no file_length param,
      returns 0 if plenty lines, or lines in file if not enough or eactly enough */

static int file_lines(int fd, long int n_lines, FILE* output)
{
   char buffer[BUFSIZ];
   int bytes_read;
   int i;                        /* Index into `buffer' for scanning.  */
   off_t file_length = lseek (fd, (off_t) 0, SEEK_END);
   off_t pos = file_length;
   int asked_lines = n_lines;

   if (n_lines == 0)
   {
      return 0;
   }

   /* Set `bytes_read' to the size of the last, probably partial, buffer;
       0 < `bytes_read' <= `BUFSIZ'.  */
   bytes_read = pos % BUFSIZ;

   if (bytes_read == 0)
   {
      bytes_read = BUFSIZ;
   }
   /* Make `pos' a multiple of `BUFSIZ' (0 if the file is short), so that all
      reads will be on block boundaries, which might increase efficiency.  */
   pos -= bytes_read;
   /* FIXME: check lseek return value  */
   lseek (fd, pos, SEEK_SET);
   bytes_read = safe_read (fd, buffer, bytes_read);
   if (bytes_read == -1)
   {
      write_syslogf("Error in file_lines: '%s'.\n",TRUE,strerror(errno));
      abort();
   }

   /* Count the incomplete line on files that don't end with a newline.  */
   if (bytes_read && buffer[bytes_read - 1] != '\n')
   {
      --n_lines;
   }

   do
   {
      /* Scan backward, counting the newlines in this bufferfull.  */
      for (i = bytes_read - 1; i >= 0; i--)
      {
         /* Have we counted the requested number of newlines yet?  */
         if (buffer[i] == '\n' && n_lines-- == 0)
         {
            /* If this newline wasn't the last character in the buffer,
               print the text after it.  */
            if (i != bytes_read - 1)
            {
               xwrite (output, &buffer[i + 1], bytes_read - (i + 1));
            }
            dump_remainder (output, fd, file_length - (pos + bytes_read));
            return 0;
         }
      }
      /* Not enough newlines in that bufferfull.  */
      if (pos == 0)
      {
         /* Not enough lines in the file; print the entire file.  */
         /* FIXME: check lseek return value  */
         lseek (fd, (off_t) 0, SEEK_SET);
         dump_remainder (output, fd, file_length);
         return asked_lines - n_lines;
      }
      pos -= BUFSIZ;

      /* FIXME: check lseek return value  */
      lseek (fd, pos, SEEK_SET);
   }
   while ((bytes_read = safe_read (fd, buffer, BUFSIZ)) > 0);

   if (bytes_read == -1)
   {
      write_syslogf("Error in file_lines: '%s'.\n",TRUE,strerror(errno));
      abort();
   }

   write_syslogf("Something weird in file_lines.\n",TRUE);
   abort();

   return 0;
}
//////////////////!TAIL/////////////////////

typedef std::map<pod_string, log_type, std::less<pod_string>,  pod_alloc< std::pair<pod_string, log_type> >::Type > LogFilesMap;

int showLogfile(UR_OBJECT user, log_type & logInfo)
{
   write_userf(user, "\n~OL~FR***~RS~OL %s Log ~FR***\n\n", logInfo.show_name);

   if( more(user, NULL, logInfo.filename) )
   {
      user->misc_op = MISC_MORE;
   }
   else
   {
      write_userf(user,"The %s log is empty\n",logInfo.show_name_lwr);
   }

   return 0;
}

int showLogfileLines(UR_OBJECT user, log_type & logInfo, unsigned int lines)
{
   unsigned int cnt;
   int logfile_fd;
   FILE *outputFile;
   pod_string tempFileName;

   tempFileName =  "logfiles/showLog.";
   tempFileName += user->name;
   tempFileName += ".tempfile";

   if ( ( logfile_fd = open( logInfo.filename, O_RDONLY) ) == -1 )
   {
      write_userf(user, "The %s log is empty\n", logInfo.show_name_lwr );
      return 0;
   }

   if ( !( outputFile = fopen(tempFileName.c_str(),"w") ) )
   {
      write_userf(user,"System error! contact admin, file was:'%s'\n",tempFileName.c_str() );
      return 0;
   }

   cnt = file_lines( logfile_fd, lines, outputFile );
   fclose( outputFile );
   close(logfile_fd);

   if (cnt == 0)
   {
      write_userf(user,"\n~BB*** %s Log (last %d lines) ***\n\n",logInfo.show_name,lines);
      if (more(user,NULL,tempFileName.c_str() ) == 1)
      {
         user->misc_op=MISC_MORE;
      }
   }
   else if (cnt < lines)
   {
      unlink_checked(tempFileName.c_str() ,"view_log");
      write_userf(user,"There are only %d lines in the log.\n",cnt);
   }
   else // if (cnt == lines)
   {
      unlink_checked(tempFileName.c_str() ,"view_log");
      write_userf(user,"\n~OL~FR***~RS~OL %s Log ~FR***\n\n",logInfo.show_name);
      if( more(user,NULL,logInfo.filename) == 1)
      {
         user->misc_op=MISC_MORE;
      }
   }
   return 0;
}

int view_log(UR_OBJECT user, char *inpstr)
{
   int logfile_fd;
   LogFilesMap logFiles;
   LogFilesMap::iterator iterator;
   struct words_struct* words_ptr = &words;

   logFiles.insert( LogFilesMap::value_type("system",  log_type( "system",    "System",    LOGFILES "/" SYSLOG     ) ) );
   logFiles.insert( LogFilesMap::value_type("filter",  log_type( "filter",    "Filter",    LOGFILES "/" FILTERLOG  ) ) );
   logFiles.insert( LogFilesMap::value_type("account", log_type( "account",   "Account",   LOGFILES "/" ACCOUNTLOG ) ) );
   logFiles.insert( LogFilesMap::value_type("boot",    log_type( "boot",      "Boot",      LOGFILES "/" BOOTLOG    ) ) );
   logFiles.insert( LogFilesMap::value_type("promo",   log_type( "promotion", "Promotion", LOGFILES "/" PROMOLOG   ) ) );
   logFiles.insert( LogFilesMap::value_type("delold",  log_type( "delold",    "Delold",    LOGFILES "/" DELOLDLOG  ) ) );
   logFiles.insert( LogFilesMap::value_type("crash",   log_type( "crash",     "Crash",     LOGFILES "/" CRASHLOG   ) ) );
   logFiles.insert( LogFilesMap::value_type("autorun", log_type( "autorun",   "Autorun",   LOGFILES "/" AUTORUNLOG ) ) );

   if (words_ptr->word_count >= 2)
   {
      if( logFiles.count( words_ptr->word[1] ) )
      {
         log_type & logFile = logFiles[words_ptr->word[1]];

         if ( words_ptr->word_count == 2 )
         {
            showLogfile( user, logFile );
            return 0;
         }
         else if( isNumber(words_ptr->word[2]) )
         {
            showLogfileLines(user, logFile, atoi(words_ptr->word[2]) );
            return 0;
         }
         else if ( user->level == LEV_FIV && !strcmp(words_ptr->word[2],"-c") )
         {
            if ( ( logfile_fd = open( logFile.filename, O_RDONLY) ) == -1)
            {
               write_userf(user,"The %s log is empty\n",logFile.show_name_lwr);
               return 0;
            }
            close(logfile_fd);

            if (!strcmp(words_ptr->word[1],"account"))
            {
               write_user(user,"\nERROR: you may not clear that log.\n");
            }
            else
            {
               unlink_checked(logFile.filename,"view_log");
               write_userf(user,"\n%s log cleared\n",logFile.show_name);
            }
            return 0;
         }
      }
   }

   write_userf(user,"\nUsage: view <filename> [<#lines%s>]\n",(user->level == LEV_FIV) ? "/-c":"");
   write_user(user,"Filenames: ");

   for ( iterator = logFiles.begin(); iterator != logFiles.end(); iterator++ )
   {
      if( iterator != logFiles.begin() )
      {
         write_user(user,",");
      }
      write_user( user, (*iterator).first.c_str() );
   }
   write_user(user,"\n\n");

   return 0;
}
