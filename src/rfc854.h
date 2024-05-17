#ifndef RFC854_H
#define RFC854_H

class RFC854
{
   public:
      static bool isUSASCIIGraphic( unsigned char c );
      static bool isUSASCIIControl( unsigned char c );
      static bool isUSASCIIUncovered( unsigned char c );

      static const unsigned char NUL;
      static const unsigned char LF;
      static const unsigned char CR;

      static const unsigned char BEL;
      static const unsigned char BS;
      static const unsigned char HT;
      static const unsigned char VT;
      static const unsigned char FF;

      static const unsigned char SE;
      static const unsigned char NOP;
      static const unsigned char DM;
      static const unsigned char BREAK;
      static const unsigned char IP;
      static const unsigned char AO;
      static const unsigned char AYT;
      static const unsigned char EC;
      static const unsigned char EL;
      static const unsigned char GA;
      static const unsigned char SB;
      static const unsigned char WILL;
      static const unsigned char WONT;
      static const unsigned char DO;
      static const unsigned char DONT;
      static const unsigned char IAC;
};

#endif /* !RFC854_H */
