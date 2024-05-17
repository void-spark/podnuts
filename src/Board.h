#ifndef BOARD_H
#define BOARD_H

#include "GenericMessageStore.h"
#include "BoardMessage.h"

class Board : public GenericMessageStore
{
   public:

      Board(pod_string board_name) : GenericMessageStore(board_name)
      {
         _rootElementName = "messageboard";
      }
      BoardMessage *getMessageAndAdvance();
      int insertMessage(pod_string sender, pod_string body);

      ~Board() {};
      int deleteOldMessages(int maxMessageLife);
};


#endif /* !BOARD_H */
