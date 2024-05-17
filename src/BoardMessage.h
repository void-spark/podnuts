#ifndef BOARDMESSAGE_H
#define BOARDMESSAGE_H

class BoardMessage : public GenericMessage
{
   public:
      BoardMessage( XmlTextReader *reader ) : GenericMessage( reader ) {}
      BoardMessage(pod_string from, pod_string body) : GenericMessage(from, body) {}
      BoardMessage(pod_string from, pod_string body, int time) : GenericMessage(from, body, time) {}
      ~BoardMessage() {}
};

#endif /* !BOARDMESSAGE_H */
