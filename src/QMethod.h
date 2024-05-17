#ifndef QMETHOD_H
#define QMETHOD_H

/*
 * See rfc1143
 */

class QMethodData
{
   public:
      enum { NO, WANTNO, WANTYES, YES };
      enum { EMPTY, OPPOSITE };

      int us;
      int usq;
      int him;
      int himq;

      QMethodData();
};

class QMethod
{

/*
 *    There are two sides, we (us) and he (him).  We keep four
 *    variables:
 *
 *       us: state of option on our side (NO/WANTNO/WANTYES/YES)
 *       usq: a queue bit (EMPTY/OPPOSITE) if us is WANTNO or WANTYES
 *       him: state of option on his side
 *       himq: a queue bit if him is WANTNO or WANTYES
 */
   protected:
      QMethodData data[256];
      TelnetHandler *telnetHandler;
      TelnetSender  *telnetSender;
      bool debug;

   public:
      QMethod( TelnetHandler *telnetHandler, TelnetSender *telnetSender );
      virtual ~QMethod();

      bool isUsEnabled( unsigned char code );
      bool isHimEnabled( unsigned char code );

      void parseWill( unsigned char code );
      void parseDo( unsigned char code );

      void parseWont( unsigned char code );
      void parseDont( unsigned char code );

      void requestHimEnable( unsigned char code );
      void requestUsEnable( unsigned char code );

      void requestHimDisable( unsigned char code );
      void requestUsDisable( unsigned char code );

      void toXML( xmlTextWriterPtr ptr );
      void fromXML( XmlTextReader * reader );
      
   protected:
      void parseEnable( bool isWill, unsigned char code );
      void parseDisable( bool isWont, unsigned char code );
      void requestEnable( bool isHim, unsigned char code );
      void requestDisable( bool isHim, unsigned char code );

      void handleEnable( bool isHim, unsigned char code );
      void handleDisable( bool isHim, unsigned char code );
};

#endif /* !QMETHOD_H */
