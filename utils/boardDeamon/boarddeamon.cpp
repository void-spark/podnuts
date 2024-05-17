#include <stdio.h>
#include <dirent.h>
#include <ctype.h>
#ifndef __FreeBSD__
#include <crypt.h>
#endif
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <set>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <ext/stdio_filebuf.h>
#include "../tinyxml/tinyxml.h"
#include "../dnsDeamon/errorClasses.h"

std::string boardFilesDir  =  "./";
std::string userFilesDir   =  "./";

class Connection
{
   public:
      Connection( ) : fileDescriptor(-1), stream(0), fdBuffer(0)
      {
      }

      int attach( int fileDescriptor )
      {
         this->fileDescriptor = fileDescriptor;
         fdBuffer = new fd_filebuf( fileDescriptor, std::ios::in|std::ios::out, true, 512 );
         stream   = new std::iostream( fdBuffer );
         if( stream->fail() )
         {
            close();
            throw IoError( "SocketStream is fail." );
         }
         if( stream->bad() )
         {
            close();
            throw IoError( "SocketStream is bad." );
         }
         if( stream->eof() )
         {
            close();
            throw IoError( "SocketStream is eof." );
         }
         return 0;
      }

      void close()
      {
         if(stream)
         {
            delete stream;
            stream = 0;
         }
         if(fdBuffer)
         {
            delete fdBuffer;
            fdBuffer = 0;
         }
         inputBuffer.clear();
      }

      int xmlSend( TiXmlDocument* outputDoc )
      {
         *stream << *outputDoc << '\0' << std::flush;
         return 0;
      }

      void doRead()
      {
         int         charsRead = 0;
         char        charBuffer[512];

         charsRead = read( fileDescriptor, charBuffer, sizeof(charBuffer) );
         if(charsRead == -1)
         {
            if( errno == EAGAIN || errno == EINTR )
            {
               return;
            }
            throw IoError( strerror(errno) );
         }
         if(charsRead == 0)
         {
            throw IoError( "read returned 0 bytes" );
         }
         inputBuffer.append( charBuffer, charsRead );
         return;
      }

      TiXmlDocument * xmlRead()
      {
         std::string::size_type firstZero = inputBuffer.find( '\0' );

         if( firstZero == std::string::npos )
         {
            throw FatalError( "Nothing in buffer on xmlRead()." );
         }

         std::istringstream inputStream( inputBuffer.substr(0,firstZero) );
         inputBuffer.erase( 0, firstZero + 1 );

         TiXmlDocument *inputDoc = new TiXmlDocument;
         inputStream >> *inputDoc;

         return inputDoc;
      }

      bool xmlBuffered()
      {
         std::string::size_type firstZero;
         if( inputBuffer.empty() )
         {
            return false;
         }

         firstZero = inputBuffer.find( '\0' );
         if( firstZero == std::string::npos )
         {
            return false;
         }
         return true;
      }

      int getFd()
      {
         return fileDescriptor;
      }

      protected:
         typedef  __gnu_cxx::stdio_filebuf<char> fd_filebuf;

         int            fileDescriptor;
         std::iostream *stream;
         fd_filebuf    *fdBuffer;
         std::string    inputBuffer;
};

int open_listen_socket(int port);
int accept_socket(int listenFd);
const std::string searchVar( std::istream *input, std::string key );
struct ltstr
{
   bool operator()(std::string s1, std::string s2) const
   {
      return s1.compare(s2) < 0;
   }
};

class DirectorySeeker
{
   public:

      typedef std::set<std::string, ltstr> StringSet;

      StringSet doSeek( std::string path )
      {
         DIR *dirp;
         struct dirent * dp;
         std::string fileName;
         StringSet mySet;

         dirp = opendir( path.c_str() );
         
         if( dirp == 0 )
         {
            std::string errString = "Error opening file '";
            errString += path;
            errString += "', cause: ";
            errString += strerror(errno);
            throw IoError( errString );
         }

         errno = 0;
         while	( ( dp = readdir(dirp) ) != 0 )
         {
            fileName = dp->d_name;
            if( checkEntry( fileName ) )
            {
               mySet.insert(fileName );
            }
	      }
         if (errno == 0)
         {
            closedir(dirp);
            return mySet;
         }
         closedir(dirp);
         std::string errString = "Error reading dir '";
         errString += path;
         errString += "', cause: ";
         errString += strerror(errno);
         errString += strerror(errno);
         throw IoError( errString );
      }

      virtual bool checkEntry( std::string fileName ) = 0;
};

class XMLSeeker : public DirectorySeeker
{
   public:
      bool checkEntry( std::string fileName )
      {
         std::string::size_type size = fileName.size();
         if( size < 4 )
         {
            return false;
         }
         if( fileName.substr(size-4,4).compare(".xml") == 0 )
         {
            return true;
         }
         return false;
      }
};

class BoardUser
{
   public:
      BoardUser( Connection *connection)
      {
         this->connection = connection;
         this->state = CONNECTED;
         this->level = 0;
      }

      ~BoardUser()
      {
         connection->close();
         delete connection;
      }

      Connection *getConnection()
      {
         return connection;
      }

      void parseXML( TiXmlDocument *inputDoc )
      {
         TiXmlDocument outputDoc;
         try
         {
            if( state == CONNECTED )
            {
               bool loginOk = checkLogin( inputDoc, userFilesDir );
               if( !loginOk )
               {
                  TiXmlElement rootNode("failure");
                  outputDoc.InsertEndChild(rootNode);
               }
               else
               {
                  state = VERIFIED;
                  TiXmlElement rootNode("boardslist");

                  XMLSeeker seeker;
                  XMLSeeker::StringSet xmlFiles = seeker.doSeek( boardFilesDir );

                  XMLSeeker::StringSet::iterator setIterator;

                  for( setIterator  = xmlFiles.begin();
                       setIterator != xmlFiles.end();
                       setIterator++ )
                  {
                     std::string fileName = *setIterator;
                     if( (fileName != "admnotes.xml") || level >= 3 )
                     {
                        TiXmlElement boardElement( fileName.substr( 0, fileName.size() - 4 ) );
                        rootNode.InsertEndChild( boardElement );
                     }
                  }
                  outputDoc.InsertEndChild(rootNode);
               }
            }
            else if( state == VERIFIED )
            {
               std::string boardFile = boardFilesDir;
               boardFile += getSelectedBoardName(inputDoc);
               boardFile += ".xml";

               outputDoc.LoadFile(boardFile.c_str());
            }
            connection->xmlSend(&outputDoc);

         }
         catch ( ParseError e )
         {
            throw e;
         }
         return;
      }
      

   protected:

      bool checkLogin( TiXmlDocument *inputDoc, std::string userFilesDir )
      {
         std::string userFileName   =  "";

         TiXmlElement *loginElement = inputDoc->FirstChildElement();
         if(loginElement == 0)
         {
            throw ParseError( "No login element found." );
         }
         const char *name = loginElement->Attribute("username");
         if(name == 0)
         {
            throw ParseError( "No username attribute found." );
         }
         std::string nameCopy = name;

         const char *password = loginElement->Attribute("password");
         if(password == 0)
         {
            throw ParseError( "No password attribute found." );
         }
         std::string passCopy = password;

         if(strlen(name) != 0)
         {
            char tempChar = nameCopy[0];
            tempChar = (char)toupper(tempChar);
            nameCopy[0] = tempChar;
         }

         userFileName = userFilesDir;
         userFileName += nameCopy;
         userFileName += ".D";
         std::ifstream userFileStream(userFileName.c_str());

         if( userFileStream.fail() || userFileStream.bad() )
         {
            std::cout << "Failed login, no such user for " << nameCopy << "." << std::endl;
            return false;
         }
         else
         {
            const std::string storedPassw = searchVar( &userFileStream, "PassWord" );
            const std::string storedLevel = searchVar( &userFileStream, "Level" );
            char *first_invalid = 0;
            const char *levelStrPtr = storedLevel.c_str();
            level = strtol( levelStrPtr, &first_invalid, 10 );
            if( first_invalid == levelStrPtr)
            {
               throw ParseError( "User level not a number." );
            }
            std::cout << level << std::endl;

            const std::string givenPassw  = crypt(passCopy.c_str(),"NU");

            if(storedPassw.compare(givenPassw ) == 0 && level >= 1 )
            {
               std::cout << "Succesfull login for " << nameCopy << "." << std::endl;
               return true;
            }
            else
            {
               std::cout << "Failed login for " << nameCopy << "." << std::endl;
               return false;
            }
         }
      }

      std::string getSelectedBoardName( TiXmlDocument *inputDoc )
      {
         std::string boardName;

         TiXmlElement *selectElement = inputDoc->FirstChildElement();
         if( selectElement == 0 )
         {
            throw ParseError( "No select element found." );
         }
         const char *name = selectElement->Attribute("boardname");
         if(name == 0)
         {
            throw ParseError( "No boardname attribute found." );
         }
         boardName = name;

         return boardName;
      }

      enum { CONNECTED,
             VERIFIED };

      Connection *connection;
      int state;
      int level;

};

int main(int argc, char *argv[])  
{
   typedef std::vector<BoardUser*> BoardUsersVector;

   BoardUsersVector users;
   bool        doFork         =  false;
   int         maxBadAccepts  =  50;
   int         maxInput       =  4000;
   int         listenPort     =  7310;
   int         listenFd       = -1;
   fd_set      readMask;
   struct      timeval timeout;
   int         badAcceptCount =  0;
   char        opt            = -1;
   int         newSocketFd    = -1;
   boardFilesDir  =  "./";
   userFilesDir   =  "./";

   TiXmlBase::SetCondenseWhiteSpace( false );

   std::cout << "BoardDeamon v1.0" << std::endl;

   while( ( opt = getopt(argc,argv, "fm:u:b:p:") ) != -1 )
   {
      switch(opt) 
      { 
         case 'u':
            userFilesDir = optarg; 
            break;
         case 'b':
            boardFilesDir = optarg;
            break;
         case 'f':
            doFork = true;
            break;
         case 'm':
            sscanf(optarg,"%d",&maxInput);
            break;
         case 'p':
            sscanf(optarg,"%d",&listenPort);
            break;
         case 'a':
            sscanf(optarg,"%d",&maxBadAccepts);
            break;             
         case '?':
         default:
            std::cout << "Usage: bdeamon [-f] -m maxInputSize -a maxBadAccepts -p port -u userfilesdir -b boardfilesdir" << std::endl;
            exit(1);
      }
   }
   if( boardFilesDir[boardFilesDir.size()-1] != '/' )
   {
      boardFilesDir += '/';
   }
   if( userFilesDir[userFilesDir.size()-1] != '/' )
   {
      userFilesDir += '/';
   }

   if (doFork)
   {
      if ( fork() )
      {
         exit(0);
      }
      std::cout << "Fork succesfull." << std::endl;
   }
   std::cout << "Board files reside in : '" << boardFilesDir << "'" << std::endl;
   std::cout << "User files reside in  : '" << userFilesDir << "'" << std::endl;
   std::cout << "Maximum input length  : " << maxInput << std::endl;
   std::cout << "Maximum bad accepts   : " << maxBadAccepts << std::endl;
   std::cout << "Listening port        : " << listenPort << std::endl;

   listenFd = open_listen_socket(listenPort);
   std::cout << "Listen port open." << std::endl;
   while( true )
   {
      FD_ZERO( &readMask );
      FD_SET( listenFd, &readMask );
      for (int cnt = 0; cnt < users.size(); cnt++)
      {
         FD_SET( users[cnt]->getConnection()->getFd(), &readMask);
      }

      timeout.tv_sec  = 10;
      timeout.tv_usec = 0;

      std::cout << "Polling..." << std::endl;
      int retval = select(FD_SETSIZE, &readMask, 0, 0, &timeout);
      std::cout << "Return." << std::endl;

      if( retval == -1 ) // error
      {
         if( errno != EINTR )
         {
            std::cerr << "Error in parent: " << strerror(errno) << "." << std::endl;
         }
         continue;
      }

      if( retval == 0 ) // timeout
      {
         std::cout << "Timeout." << std::endl;
         continue;
      }

      if (FD_ISSET(listenFd, &readMask))
      {
         std::cout << "Listen socket flagged." << std::endl;
         badAcceptCount = 0;
         newSocketFd = -1;
         while( newSocketFd == -1 )
         {
            std::cout << "Doing accept." << std::endl;
            newSocketFd = accept_socket(listenFd);
            if( newSocketFd == -1 )
            {
               if(badAcceptCount < maxBadAccepts)
               {
                  badAcceptCount++;
               }
               else
               {
                  std::cerr << "Too many failed accepts." << std::endl;
                  exit(13);
               }
            }
         }

         Connection *newConnection = new Connection;
         newConnection->attach( newSocketFd );
         BoardUser * newUser = new BoardUser( newConnection );
         users.push_back(newUser);

         std::cout << "New socket accepted." << std::endl;
      }
      for (int cnt = 0; cnt < users.size(); cnt++)
      {
         if ( FD_ISSET( users[cnt]->getConnection()->getFd(), &readMask ) )
         {
            std::cout << "Client socket " << cnt << " flagged." << std::endl;
            TiXmlDocument *inputDoc = 0;

            try
            {
               users[cnt]->getConnection()->doRead();

               if( users[cnt]->getConnection()->xmlBuffered() )
               {
                  inputDoc = users[cnt]->getConnection()->xmlRead();
                  std::cout << "Read xml : " << *inputDoc << std::endl;

                  users[cnt]->parseXML( inputDoc );
               }
            }
            catch( IoError e )
            {
               std::cerr << "Disconnected user: " << e.what() << std::endl;
               delete users[cnt];
               users.erase( users.begin() + cnt );
            }
            if( inputDoc != 0 )
            {
               delete inputDoc;
               inputDoc = 0;
            }
         }
      }
   }
}

const std::string searchVar( std::istream *input, std::string key )
{
   std::string var;
   std::string value;

   input->seekg (0, std::ios::beg);

   char inputChar = 0;

   while( var.compare(key) != 0 )
   {
      var = "";
      value = "";

      while( (inputChar = input->get()) != EOF && inputChar != ' ')
      {
         var += inputChar;
      }
      while( (inputChar = input->get()) != EOF && inputChar != '\n' )
      {
         value += inputChar;
      }
      if( input->eof() )
      {
         return 0;
      }
   }
   return value;
}

int open_listen_socket(int port)
{
   int socketFd;

   /* open new socket, inet domain, stream socket */
   socketFd = socket(AF_INET,SOCK_STREAM,0);
   if ( socketFd == -1 )
   {
      std::cerr << "Can't open listen socket on port " << port << ": " << strerror(errno) << "." << std::endl;
      exit(11);
   }

   /* allow reboots on port even with TIME_WAITS */
   int yes = 1;
   setsockopt(socketFd,SOL_SOCKET,SO_REUSEADDR,(char *)&yes,sizeof(yes));

   /* bind sockets and set up listen queues */
   struct sockaddr_in bindAddress;

   bindAddress.sin_family      = AF_INET;
   bindAddress.sin_addr.s_addr = INADDR_ANY;
   bindAddress.sin_port        = htons(port);

   int sockaddr_in_size = sizeof(struct sockaddr_in);
   if( bind(socketFd,(struct sockaddr *)&bindAddress,sockaddr_in_size) == -1 )
   {
      std::cerr << "Can't bind socket on port " << port << ": " << strerror(errno) << "." << std::endl;
      exit(12);
   }

   /* start listening on port */
   if( listen(socketFd,10) == -1 )
   {
      std::cerr << "Listen error on port " << port << ": " << strerror(errno) << "." << std::endl;
      exit(13);
   }

   return socketFd;
}

int accept_socket(int listenFd)
{
   int newSocket = accept(listenFd,0,0);

   if( newSocket == -1 )
   {
      std::cerr << "Error in accept on port " << listenFd << ": " << strerror(errno) << "." << std::endl;
      return -1;
   }

   return newSocket;
}

/*
 *  http://gcc.gnu.org/onlinedocs/libstdc++/ext/howto.html
 *  http://gcc.gnu.org/onlinedocs/libstdc++/17_intro/porting-howto.html
 *  ftp://people.redhat.com/jakub/gcc3
 */

