#ifndef GENERICMESSAGESTORE_H
#define GENERICMESSAGESTORE_H

#include <vector>
#include "GenericMessage.h"

typedef std::vector<GenericMessage*, pod_alloc< GenericMessage* >::Type > MessagesVector;

class GenericMessageStore
{
   protected:
      pod_string        _boardFileName;
      pod_string        _rootElementName;
      unsigned int      currentMessage;
      MessagesVector   messages;
      virtual GenericMessage *createEmptyMessage();

   public:
      const static int CREATE = 1;

      GenericMessageStore(pod_string board_name) : _boardFileName(board_name),  currentMessage(0)
      {
      }
      virtual ~GenericMessageStore();

      pod_string getFileName();
      int hasNextMessage();
      GenericMessage *getMessageAndAdvance();
      GenericMessage *getMessage(unsigned int number);
      void open(int flags);
      int save();
      int message_count();
      void advance();
      int insertMessage(GenericMessage *message);
      int wipeAllMessages();
      int wipeMessages(int *deleteMask);
      virtual void fromXml( XmlTextReader *reader );
      virtual void messagesContentToXML( xmlTextWriterPtr ptr );
      virtual void toXML( xmlTextWriterPtr ptr );
};

#endif /* !GENERICMESSAGESTORE_H */
