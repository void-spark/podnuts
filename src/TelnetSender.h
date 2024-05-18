#ifndef TELNETSENDER_H
#define TELNETSENDER_H

class TelnetSender
{   
   public:
      virtual ~TelnetSender() {};
      
      virtual void sendCommand( unsigned char cmd ) = 0;
      virtual void sendCommand( unsigned char cmd1, unsigned char cmd2 ) = 0;
};

#endif /* !TELNETSENDER_H */
