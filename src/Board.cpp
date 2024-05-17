#include <sstream>
#include <string>
#include "XmlParsingError.h"
#include "libxml_glue.h"
#include "string_misc.h"
#include "Board.h"

int Board::insertMessage(pod_string sender, pod_string body)
{
   pod_string wrappedBody = wordWrap( body, 80 );

   BoardMessage *message = new BoardMessage( sender,wrappedBody );
   return GenericMessageStore::insertMessage( message );
}

BoardMessage *Board::getMessageAndAdvance()
{
   return (BoardMessage*)GenericMessageStore::getMessageAndAdvance();
}

int Board::deleteOldMessages(int maxMessageLife)
{
   const int numberOfSecondsInDay = 60*60*24;
   int old_cnt=0;

   int size = messages.size();
   for(int i = (size-1); i >= 0; i--)
   {
      if ((int)time(0) - messages[i]->getTime() >= maxMessageLife*numberOfSecondsInDay)
      {
         delete messages[i];
         messages.erase(messages.begin()+i);
         old_cnt++;
      }
   }

   if (old_cnt == size)
   {
      wipeAllMessages();
   }
   else
   {
      save();
   }
   return old_cnt;
}

