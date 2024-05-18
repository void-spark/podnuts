#ifndef SOCKET_FUNCS_H
#define SOCKET_FUNCS_H

#include <list>
#include <string>
#include <iostream>

#include "pod_alloc.h"
#include "pod_string.h"

#define MAX_SITE_LEN   80
#define LSOCK_ID_LEN   5

class PlainSocket;
typedef std::list<PlainSocket*, pod_alloc< PlainSocket* >::Type > SocketsList;

class ListenSocket;
typedef std::list<ListenSocket*, pod_alloc< PlainSocket* >::Type > LSocketsList;

int              sock_close_conn(PlainSocket *socket );

class SocketInterface;
class SockObjectCreator;

extern SocketInterface socketInterface;

class GenericSocket
{
   protected:
      int  _socket;
      int  _localPort;

   public:
      GenericSocket() : _socket(-1) , _localPort(-1)
      {}
      GenericSocket(int fd, int port) : _socket(fd) , _localPort(port)
      {}
      virtual ~GenericSocket()
      {}
      int getFileDescriptor() { return _socket; }
      void setFileDescriptor(int newFd) { _socket = newFd; }
      int getLocalPort() { return _localPort; }
};

class SocketInterface
{
   protected:
      SocketsList  plainSocketsList;
      LSocketsList listenSocketsList;

      fd_set _readMask;
      fd_set _writeMask;

      int checkAndHandleNewConnections();
      int setupReadMask();
      int setupWriteMask();
      int readCycle();
      int writeCycle();
      bool readMaskContains(GenericSocket *socket)
      {
         return FD_ISSET(socket->getFileDescriptor(),&_readMask);
      }
      bool writeMaskContains(GenericSocket *socket)
      {
         return FD_ISSET(socket->getFileDescriptor(),&_writeMask);
      }

   public:
      SocketInterface(){};
      int addPlainSocket( PlainSocket *node );
      int deletePlainSocket( PlainSocket *node ); /* remove from list and delete object (deleting flushes the queue) */
      int closeSocketHard( PlainSocket *socket ); /* call deletePlainSocket and then close the fd */
      int closeAllPlainSockets();
      pod_string getPortsString();

      int addListenSocket( ListenSocket *node );
      int initListenSockets();
      int closeListenSockets();
      int getListenPortNumber( pod_string id );
      int closeCycle();
      void procShow( std::ostream &output );
      void handleRequests();
};

class ListenSocket : public GenericSocket
{
   protected:
      pod_string               _id;  // LSOCK_ID_LEN
      SockObjectCreator*   _plainSocketCreator;

   public:
      ListenSocket( pod_string id, SockObjectCreator* plainSocketCreator, int port ) :GenericSocket(-1, port), _id(id),  _plainSocketCreator(plainSocketCreator)
      {
      }
      int create();
      pod_string getName();
      virtual ~ListenSocket();
      PlainSocket* AcceptConnection();
};

class PlainSocket : public GenericSocket
{
   public:
      /* flags a socket can have */
      const static int SOCKET_QUIT = 1<<0;

      time_t             connect_time;
      char               ip_num[MAX_SITE_LEN+1];

   protected:
      int                _peerPort;
      pod_string         _peerSite;
      pod_stringstream   _queueStringStream;
      int                _flags;

      ssize_t write_checked(const void *buf, size_t count);
      void constructorFunc( int fileDescriptor );

   public:
      PlainSocket( );
      PlainSocket( int fileDescriptor );
      PlainSocket( int fileDescriptor , pod_string peerSite );
      void connectIp( pod_string ip, int port );
      int getPeerPort();
      pod_string getPeerSite();
      void setPeerSite( pod_string );
      int flag_as_close_on_execv(bool value);
      bool isFlaggedAsCloseOnExecv();
      int flag_as_closed();
      bool isFlaggedForClosing();
      int write_queue(pod_string & str);
      std::ostream & getQueueStream();
      int flush_queue();
      int kill_queue();
      bool isQueueEmpty();
      int write(char *str);
      int write_len(char *str,size_t count);
      int handleInput();
      void resolveName();

      virtual void initializeSocket()          = 0;
      virtual int input_handler( pod_string input ) = 0;
      virtual int disconnect_handler()        = 0;

      virtual ~PlainSocket();
};

class SockObjectCreator
{
   public:
      virtual PlainSocket* getObjectInstance( int fileDescriptor ) = 0;
};

#endif /* !SOCKET_FUNCS_H */

