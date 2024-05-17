#ifndef MAILBOX_H
#define MAILBOX_H

#include "GenericMessageStore.h"
#include "MailMessage.h"
#include "pod_string.h"

class MailBox : public GenericMessageStore
{
   protected:
      int               _lastReadValue;

   public:
      MailBox(pod_string board_name) : GenericMessageStore(board_name), _lastReadValue(0)
      {
         _rootElementName = "mailbox";
      }
      ~MailBox() {};

      MailMessage *getMessageAndAdvance();
      MailMessage *getMessage(unsigned int number);
      int insertNewMessage(pod_string sender, pod_string body, bool cc);
      int getLastReadValue();
      void setLastReadValue(int val);
      void toXML( xmlTextWriterPtr ptr );
      void fromXml( XmlTextReader *reader );
      GenericMessage *createEmptyMessage();
};

#endif /* !MAILBOX_H */
