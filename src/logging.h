#ifndef LOGGING_H
#define LOGGING_H

#define DELOLDLOG    "users_removed"
#define CRASHLOG     "crashlog"
#define PROMOLOG     "promotelog"
#define BOOTLOG      "bootlog"
#define FILTERLOG    "filterlog"
#define ACCOUNTLOG   "accountlog"
#define SYSLOG       "syslog"
#define AUTORUNLOG   "autorunlog"
#define DEBUGLOG     "debuglog"

#define LOGFILES     "logfiles"

void setLoggingOn( bool val );
int  write_syslog(char *str, bool write_time);
void write_syslogf(char *str, bool write_time, ...) __attribute__ ((format (printf,1,3)));

void writeLogfile( pod_string filename, bool write_time, pod_string string );

#define pod_send '\0'

class WriteLogBuff : public pod_stringbuf
{
   public:
      WriteLogBuff();
      void reset();
      void setLogfile(pod_string logfile);
      void setWithTime(bool withTime);

   protected:
      pod_string _logfile;
      bool       _withTime;

      virtual int        overflow(int) ;
      virtual std::streamsize xsputn(const char* s, std::streamsize n);
};

class WriteLogStream : public std::ostream
{
   public:
      WriteLogStream();
      void setLogfile(pod_string logfile);
      void setWithTime(bool withTime);

   protected:
      WriteLogBuff _streambuffer;
};

struct _SetLogfile
{
   pod_string _logfile;
};

inline _SetLogfile setLogfile(pod_string __logfile)
{
   _SetLogfile __setLogfile;
   __setLogfile._logfile = __logfile;
   return __setLogfile;
}

struct _SetWithTime
{
   bool _withTime;
};

inline _SetWithTime setWithTime( bool __withTime)
{
   _SetWithTime __setWithTime;
   __setWithTime._withTime = __withTime;
   return __setWithTime;
}

std::ostream& noTime(std::ostream& os);

std::ostream& operator<<( std::ostream& __os, _SetLogfile __logfile);
std::ostream& operator<<( std::ostream& __os, _SetWithTime __withtime);

extern WriteLogStream logStream;

#endif /* !LOGGING_H */
