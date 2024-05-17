#ifndef ROOM_H
#define ROOM_H

#define ROOMFILES    "datafiles/roomfiles"
#define ROOM_NAME_LEN  16
#define ROOM_LABEL_LEN 5
#define ROOM_DESC_LEN  2000 /* 10 lines of 80 chars each + 10 nl */
#define TOPIC_LEN      65
#define MAX_LINKS      6

#include <stdio.h>


#ifndef PODNUTS_H
typedef struct user_struct* UR_OBJECT;
typedef struct Room* RM_OBJECT;
#endif

class Room
{
   public:
      char name[ROOM_NAME_LEN+1];
      char label[ROOM_LABEL_LEN+1];
      pod_string desc;
      char topic[TOPIC_LEN+1];
      int access;
      int mesg_cnt;
      int secret;
      char link_label[MAX_LINKS][ROOM_LABEL_LEN+1];
      Room *link[MAX_LINKS];
      int inked;

      BasicVar **data_ptr;

      Room(const pod_string theLabel, const pod_string theName, int visible);
      ~Room();
      int addLinkLabel(const pod_string theLabel);
      int setup_room_links();
      int load_desc();
      int PathTo(UR_OBJECT user);
      void toXML( xmlTextWriterPtr ptr );
      void fromXML( XmlTextReader * reader );
      BasicVar* getVar( pod_string name );

   private:
      bool visited;
      Room *previous;
};

#include <vector>

typedef std::vector<Room*, pod_alloc< Room* >::Type > RoomsVector;

extern RoomsVector roomsList;

int addRoomVarTempl( pod_string name,ObjectCreator *var );

#endif /* !ROOM_H */
