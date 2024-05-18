#ifndef CRTLINEBUFFER_H
#define CRTLINEBUFFER_H

#include <iostream>

class CRTLineBuffer
{
   protected:
      bool       debug;
      bool       hasLine;
      pod_string buffer;
      pod_string line;

      void checkForLine();
   public:
      CRTLineBuffer( );

      pod_string putData( pod_string input );
      bool hasNextLine();
      pod_string getLine( );

};

#endif /* !CRTLINEBUFFER_H */
