#ifndef TELNETHANDLER_H
#define TELNETHANDLER_H

class TelnetHandler
{   
   public:
      virtual ~TelnetHandler() {};
      
      virtual void handleConnect() = 0;
      virtual void handleDisconnect() = 0;

      virtual void handleText( pod_string input ) = 0;

      virtual void doWeEnable( unsigned char option ) = 0;
      virtual void doWeDisable( unsigned char option ) = 0;
      virtual void doThemEnable( unsigned char option ) = 0;
      virtual void doThemDisable( unsigned char option ) = 0;

      virtual void handleSubNegotiation( unsigned char option, pod_string value ) = 0;

      virtual bool mayHeEnable( unsigned char code ) = 0;
      virtual bool mayWeEnable( unsigned char code ) = 0;

};

#endif /* !TELNETHANDLER_H */
