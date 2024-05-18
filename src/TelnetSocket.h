#ifndef TELNETSOCKET_H
#define TELNETSOCKET_H

class TelnetSocket : public PlainSocket, public TelnetSender
{
   protected:
      TelnetHandler *telnetHandler;
      
      QMethod qMethod;
      
      bool debug;

   public:
      TelnetSocket( int fileDescriptor , TelnetHandler *telnetHandler );
      TelnetSocket( int fileDescriptor , pod_string dns_name , TelnetHandler *telnetHandler );

      virtual void initializeSocket();
      virtual int input_handler(pod_string input);
      virtual int disconnect_handler();

      ~TelnetSocket();
// telnet

      void handleInput( pod_string input );

      bool isUsEnabled( unsigned char code );
      bool isHimEnabled( unsigned char code );

      void requestHimEnable( unsigned char code );
      void requestUsEnable( unsigned char code );

      void requestHimDisable( unsigned char code );
      void requestUsDisable( unsigned char code );      

      void sendCommand( unsigned char cmd );
      void sendCommand( unsigned char cmd1, unsigned char cmd2 );
                  
      TelnetHandler* getTelnetHandler();
      
      void toXML( xmlTextWriterPtr ptr );
      void fromXML( XmlTextReader * reader );
      
};

#endif /* !USERSOCKET_H */
