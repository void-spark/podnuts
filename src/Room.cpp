#include <stdlib.h>
#include <stdarg.h>
#include <deque>
#include <stack>
#include "general_headers.h"
#include "string_misc.h"
#include "StringLibrary.h"
#include "dynamicVarsController.h"
#include "Room.h"

RoomsVector roomsList;

DynamicVarsController roomsVarsController;

int addRoomVarTempl( pod_string name,ObjectCreator *var )
{
   return roomsVarsController.addVarTempl( name, var, 0 );
}

void Room::toXML( xmlTextWriterPtr ptr )
{
   xmlTextWriterStartElement( ptr, reinterpret_cast<const xmlChar *>("room") );
   xmlTextWriterWriteAttribute( ptr, reinterpret_cast<const xmlChar *>("name"), reinterpret_cast<const xmlChar *>(name) );
   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("\n"));

   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("   "));

   xmlTextWriterStartElement( ptr, reinterpret_cast<const xmlChar *>("item") );
   xmlTextWriterWriteAttribute( ptr, reinterpret_cast<const xmlChar *>("name"), reinterpret_cast<const xmlChar *>("topic") );
   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>( topic ));
   xmlTextWriterEndElement( ptr );
   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("\n"));

   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("   "));

   pod_string accessString = intToString( access );

   xmlTextWriterStartElement( ptr, reinterpret_cast<const xmlChar *>("item") );
   xmlTextWriterWriteAttribute( ptr, reinterpret_cast<const xmlChar *>("name"), reinterpret_cast<const xmlChar *>("access") );
   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>( accessString.c_str() ));
   xmlTextWriterEndElement( ptr );
   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("\n"));

   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("   "));

   pod_string inkedString = intToString( inked );

   xmlTextWriterStartElement( ptr, reinterpret_cast<const xmlChar *>("item") );
   xmlTextWriterWriteAttribute( ptr, reinterpret_cast<const xmlChar *>("name"), reinterpret_cast<const xmlChar *>("inked") );
   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>( inkedString.c_str() ));
   xmlTextWriterEndElement( ptr );
   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("\n"));

   roomsVarsController.toXML( data_ptr, 0, ptr );

   xmlTextWriterEndElement( ptr );
   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("\n"));
}
            
BasicVar* Room::getVar( pod_string name )
{
   return roomsVarsController.objGetVar( data_ptr, name );
}

Room::Room(const pod_string theLabel, const pod_string theName, int visible)
{
   int i;
   strcpy(label, theLabel.c_str());
   strcpy(name,  theName.c_str());
   topic[0]='\0';
   access=-1;
   inked=0;
   mesg_cnt=0;
   secret=0;
   for(i=0;i<MAX_LINKS;++i)
   {
      link_label[i][0]='\0';  
      link[i]=NULL;
   }

   if(!visible)
   {
      secret = 1;
   }
   else
   {
      secret = 0;
   }

   roomsVarsController.createObj( data_ptr );
   roomsVarsController.initObj( data_ptr );
}

Room::~Room()
{
   roomsVarsController.deleteObj( data_ptr );
}

int Room::addLinkLabel(const pod_string theLabel)
{
   int i;

   if (!strcmp(theLabel.c_str(), label))
   {
      std::cerr << "          " << TALKER_NAME << ": " << "Room " << name << " has a link to itself.\n";
      logStream << setLogfile( SYSLOG ) << noTime << "FATAL: Room " << name << " has a link to itself.\n" << pod_send;
      exit(1);
   }

   for(i=0;i<MAX_LINKS;++i)
   {
      if(link_label[i][0] == '\0')
      {
         strcpy(link_label[i], theLabel.c_str());
         return 0;
      }
   }
   std::cerr << "          " << TALKER_NAME << ": " << "Too many links for room " << name << ".\n";
   logStream << setLogfile( SYSLOG ) << noTime << "FATAL: Too many links for room " << name << ".\n" << pod_send;
   exit(1);
}

int Room::setup_room_links()
{
   RoomsVector::iterator roomNode;
   int cnt;

   for (cnt = 0; cnt < MAX_LINKS; ++cnt)
   {
      if (!link_label[cnt][0]) break;

      for(roomNode=roomsList.begin();roomNode != roomsList.end();roomNode++)
      {
         if ( !strcmp(name,(*roomNode)->name) ) continue;
         if (!strcmp(link_label[cnt], (*roomNode)->label))
         {
            link[cnt] = (*roomNode);
            break;
         }
      }
      if (link[cnt] == NULL)
      {
         std::cerr << "          " << TALKER_NAME << ": " << "Room " << name << " has undefined link label '" << link_label[cnt] << "'.\n";
         logStream << setLogfile( SYSLOG ) << noTime << "FATAL: Room " << name << " has undefined link label '" << link_label[cnt] << "'.\n" << pod_send;
         exit(1);

      }
   }

   return 0;
}

int Room::PathTo(UR_OBJECT user)
{
   StringLibrary * stringLibrary = StringLibrary::getInstance();
   RoomsVector::iterator roomNode;
   typedef std::deque<Room*, pod_alloc< Room* >::Type > RoomDeque;
   typedef std::stack<Room*, RoomDeque > RoomStack;
   RoomDeque roomDeque;
   RoomStack pathStack;
   Room *currentRoom,*targetRoom;
   int link_cnt;

   if (words.word_count<2)
   {
      write_user(user,"Usage .route <room name>\n");
      return 0;
   }
   else if (!(targetRoom=get_room(words.word[1])))
   {
      write_user_crt(user,stringLibrary->makeString("nosuchroom").c_str());
      return 0;
   }

   if(this==targetRoom)
   {
      write_user(user,"Already there!\n");
      return 0;
   }

   for(roomNode=roomsList.begin();roomNode != roomsList.end();roomNode++)
   {
      (*roomNode)->visited=false;
      (*roomNode)->previous=0;
   }

   roomDeque.push_back(this);
   this->visited=true;
   while(!roomDeque.empty())
   {
      currentRoom=roomDeque.front();
      roomDeque.pop_front();
      for(link_cnt=0;link_cnt < MAX_LINKS && currentRoom->link[link_cnt];link_cnt++)
      {
         if( currentRoom->link[link_cnt]->visited ||
             !has_room_access(user,currentRoom->link[link_cnt]) )
         {
            continue;
         }
         if(currentRoom->link[link_cnt] == targetRoom)
         {
            pathStack.push(targetRoom);
            while(currentRoom != this)
            {
               pathStack.push(currentRoom);
               currentRoom = currentRoom->previous;
            }
            pathStack.push(currentRoom);
            write_user(user,"~OLRoute:~RS ");
            while(pathStack.size() > 1)
            {
               write_userf(user,"%s -> ",pathStack.top()->name);
               pathStack.pop();
            }
            write_userf(user,"%s\n",pathStack.top()->name);
            pathStack.pop();
            return 0;
         }
         roomDeque.push_back(currentRoom->link[link_cnt]);
         currentRoom->link[link_cnt]->visited=true;
         currentRoom->link[link_cnt]->previous=currentRoom;
      }
   }
   write_user(user,"No route to target\n");
   return 0;
}

int Room::load_desc()
{
   FILE *fp;
   char filename[80];
   char c;
   int cnt;

   sprintf(filename, "%s/%s.R", ROOMFILES, name);
   if (!(fp = fopen(filename, "r")))
   {
      std::cerr <<  "          " << TALKER_NAME << ": Can't open description file for room " << name << ".\n";
      logStream << setLogfile( BOOTLOG ) << noTime << "ERROR: Couldn't open description file for room " << name << ".\n" << pod_send;
      desc = "";
      return 1;
   }

   cnt = 0;
   c = getc(fp);
   desc = "";

   while (!feof(fp))
   {
      if (cnt == ROOM_DESC_LEN)
      {
         std::cerr <<  "          " << TALKER_NAME << ": Description too long for room " << name << ".\n";
         logStream << setLogfile( BOOTLOG ) << noTime << "ERROR: Description too long for room " << name << ".\n" << pod_send;
         desc = "System error!";
         break;
      }
      desc += c;
      c = getc(fp);
      ++cnt;
   }
   fclose(fp);

   return 0;
}

void Room::fromXML( XmlTextReader * reader  )
{
   enum
   {
      UNSET,
      TOPIC_ITEM,
      ACCESS_ITEM,
      INKED_ITEM,
   };

   int        state         = UNSET;
   bool       hasRead       = true;
   bool       inRoomElement = false;
   int        type          = 0;
   pod_string name;
   pod_string itemName;
   pod_string itemValue;
   
   while( hasRead )
   {
      type = reader->NodeType();
      
      if( type == XML_READER_TYPE_ELEMENT ||
          type == XML_READER_TYPE_END_ELEMENT )
      {
         name = reader->Name();

         if( type == XML_READER_TYPE_ELEMENT )
         {
            if( name == "room" )
            {
               inRoomElement = true;
//                  xmlChar *roomName = xmlTextReaderGetAttribute( reader, (xmlChar*)"name" );
//                  xmlFree( roomName );
            }
            else if( inRoomElement == true && name == "item" )
            {
               try
               {
                  itemName = reader->GetAttribute( "name" );
               }
               catch (NoSuchAttribute &e)
               {
                  std::cerr << "item with no name!" << std::endl;
                  abort();
               }

               if( itemName == "topic" )
               {
                  state = TOPIC_ITEM;
               }
               else if( itemName == "access" )
               {
                  state = ACCESS_ITEM;
               }
               else if( itemName == "inked" )
               {
                  state = INKED_ITEM;
               }
               else
               {
                  roomsVarsController.fromXML( reader, data_ptr );
               }
            }
         }
         else if( type == XML_READER_TYPE_END_ELEMENT )
         {
            if( name == "room" && inRoomElement == true )
            {
               inRoomElement = false;
               return;
            }
            else if( name == "item" && inRoomElement == true)
            {
               state = UNSET;
            }
         }
      }
      else if( type == XML_READER_TYPE_TEXT )
      {
         itemValue = reader->Value();

         if(state == TOPIC_ITEM)
         {
            if( itemValue.length() <= TOPIC_LEN )
            {
               strcpy(topic, itemValue.c_str());
            }
         }
         else if(state == ACCESS_ITEM)
         {
            access = stringToInt( itemValue );
         }
         else if(state == INKED_ITEM)
         {
            inked = stringToInt( itemValue );
         }
      }

      hasRead = reader->Read();
   }   

}
