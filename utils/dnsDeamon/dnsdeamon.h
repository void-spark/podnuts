#ifndef DNSDEAMON_H
#define DNSDEAMON_H

struct dns_request
{
   std::string                   id;
   std::string                   ip_addr;
};

class childProcess
{
   public:
      childProcess();
      void spawn();
      void run();
      void takeOffQueue();
      void processResult( std::iostream *socketStream );
      bool isIdle()
      {
         return !request;
      }
      int getReadFd()
      {
         return parentReadFd;
      }
      int getWriteFd()
      {
         return parentWriteFd;
      }

   protected:
      int                      childReadFd;
      std::istream            *childReadStream;
      std::filebuf            *childReadStreamBuffer;
      int                      childWriteFd;
      std::ostream            *childWriteStream;
      std::filebuf            *childWriteStreamBuffer;
      int                      parentReadFd;
      std::istream            *parentReadStream;
      std::filebuf            *parentReadStreamBuffer;
      int                      parentWriteFd;
      std::ostream            *parentWriteStream;
      std::filebuf            *parentWriteStreamBuffer;
      pid_t                    pid;
      struct dns_request      *request;
      pid_t                    parent_pid;
};

#endif /* !DNSDEAMON_H */
