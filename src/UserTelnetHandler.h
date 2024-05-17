#ifndef USERTELNETHANDLER_H
#define USERTELNETHANDLER_H

class UserTelnetHandler : public TelnetHandler
{
   protected:
      TelnetSocket *telnetSocket;
      UR_OBJECT userPtr;
      bool debug;

   public:
      UserTelnetHandler();
      ~UserTelnetHandler();

      void setUserPtr(UR_OBJECT user);

      void handleConnect();
      void handleDisconnect();

      void handleText( pod_string input );

      void doWeEnable( unsigned char option );
      void doWeDisable( unsigned char option );
      void doThemEnable( unsigned char option );
      void doThemDisable( unsigned char option );

      void handleSubNegotiation( unsigned char option, pod_string value );

      bool mayHeEnable( unsigned char code );
      bool mayWeEnable( unsigned char code );

      void setSocket( TelnetSocket * socket );      
      
};

#endif /* !USERTELNETHANDLER_H */
