#ifndef MAILMESSAGE_H
#define MAILMESSAGE_H

class MailMessage : public GenericMessage
{
   protected:
      bool   _cc;

   public:
      MailMessage();
      MailMessage( XmlTextReader *reader );
      MailMessage(pod_string from, pod_string body, bool cc);
      MailMessage(pod_string from, pod_string body, int time, bool cc);
      ~MailMessage();

      bool   getIsCC();

      void headerContentToXML( xmlTextWriterPtr ptr );
      
      void parseHeaderElement( XmlTextReader *reader );      
};

#endif /* !MAILMESSAGE_H */
