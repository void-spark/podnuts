#ifndef USERTELNETSOCKOBJECTCREATOR_H
#define USERTELNETSOCKOBJECTCREATOR_H

class UserTelnetSockObjectCreator : public SockObjectCreator
{
   public:
      PlainSocket* getObjectInstance( int fileDescriptor );
};

#endif /* !USERTELNETSOCKOBJECTCREATOR_H */
