#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>
#include <sstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <setjmp.h>
#include "general_headers.h"
#include <vector>
#include "string_misc.h"
#include "shutdown.h"
#include "globals.h"
#include "loadsave_config.h"
#define NC 0
#define CONF_LINE_MAXLEN 81

#define SECT_NONE 0
#define SECT_INIT 1
#define SECT_ROOMS 2

#include <map>

int  setup_rooms();

typedef std::vector< BasicVar* , pod_alloc< BasicVar* >::Type > ConfigVarsVector;
StringsVector pluginStringsVector;

ConfigVarsVector configVars;

typedef std::map<pod_string, pod_string, std::less<pod_string>, pod_alloc< std::pair<pod_string, pod_string> >::Type > StringStringMap;
StringStringMap configStrings;



void addConfigVar( BasicVar *newVar )
{
   pod_string key;
   key = newVar->getName();

   configVars.push_back(newVar);
   if( !configStrings.count(key) )
   {
      std::cout << "-- BOOT : Warning, config option " << key << " unset, used default value.\n";
      logStream << setLogfile( BOOTLOG ) << noTime << "Warning, config option " << key << " unset, used default value.\n" << pod_send;
      newVar->init();
   }
   else
   {
      if( newVar->setFromString(configStrings[key]) == -1)
      {
         std::cerr << "          " << TALKER_NAME << ": " << "Error loading config for " << key << ".\n";
         logStream << setLogfile( BOOTLOG ) << noTime << "BOOT FAILURE: Error loading config for " << key << ".\n" << pod_send;
         exit(1);
      }
      configStrings.erase(key);
   }
}

void checkConfig()
{
   StringStringMap::iterator iterator;

   for ( iterator = configStrings.begin(); iterator != configStrings.end(); iterator++ )
   {
      std::cout << "Warning, config option " << (*iterator).first << "=" << (*iterator).second << " unused.\n";
      logStream << setLogfile( BOOTLOG ) << noTime
           << "Warning, config option " << (*iterator).first << "=" << (*iterator).second << " unused.\n" << pod_send;
   }

   configStrings.clear();
}

void parse_init_section( XmlTextReader *reader )
{
   bool hasRead = false;

   int initDepth = reader->Depth();

   pod_string name;
   pod_string value;

   hasRead = reader->Read();
   while( hasRead )
   {
      int type  = reader->NodeType();
      int depth = reader->Depth();
      if( type == XML_READER_TYPE_ELEMENT || type == XML_READER_TYPE_END_ELEMENT )
      {
         pod_string name = reader->Name();

         if( type == XML_READER_TYPE_ELEMENT )
         {
            if( name == "setting" )
            {
               try
               {
                  name = reader->GetAttribute( "name" );
               }
               catch (NoSuchAttribute &e)
               {
                  std::cerr << "          " << TALKER_NAME << ": " << "No 'name' attribute for element 'setting' found in configfile.\n";
                  logStream << setLogfile( BOOTLOG ) << noTime << "BOOT FAILURE: No 'name' attribute for element 'setting' found in configfile.\n" << pod_send;
                  exit(1);
               }

               try
               {
                  value = reader->GetAttribute( "value" );
               }
               catch (NoSuchAttribute &e)
               {
                  std::cerr << "          " << TALKER_NAME << ": " << "No 'valueame' attribute for element 'setting' found in configfile.\n";
                  logStream << setLogfile( BOOTLOG ) << noTime << "BOOT FAILURE: No 'value' attribute for element 'setting' found in configfile.\n" << pod_send;
                  exit(1);
               }

               if( name == "plugin" )
               {
                  pluginStringsVector.push_back( value );
               }
               else
               {
                  configStrings[name] = value;
               }
            }
         }
         else if( type == XML_READER_TYPE_END_ELEMENT )
         {
            if( depth == initDepth )
            {
               return;
            }
         }
      }

      hasRead = reader->Read();
   }
}

/*** Parse rooms section ***/
void parse_rooms_section( XmlTextReader *reader )
{
   RoomsVector::iterator roomNode;
   Room *room;
   int visible=1;

   bool hasRead = false;

   bool inRoomElement = false;

   int roomsDepth = reader->Depth();

   hasRead = reader->Read();
   while( hasRead )
   {
      int type  = reader->NodeType();
      int depth = reader->Depth();
      if( type == XML_READER_TYPE_ELEMENT || type == XML_READER_TYPE_END_ELEMENT )
      {
         pod_string name = reader->Name();

         if( type == XML_READER_TYPE_ELEMENT )
         {
            if( name == "room" )
            {
               pod_string name;
               pod_string shortcut;
               pod_string access;
               pod_string hidden;

               inRoomElement = true;
               try
               {
                  name = reader->GetAttribute( "name" );
               }
               catch (NoSuchAttribute &e)
               {
                  std::cerr << "          " << TALKER_NAME << ": " << "Required parameter name missing.\n";
                  logStream << setLogfile( BOOTLOG ) << noTime << "BOOT FAILURE: Required parameter name missing.\n" << pod_send;
                  exit(1);
               }

               if( name.size() > ROOM_NAME_LEN )
               {
                  std::cerr << "          " << TALKER_NAME << ": " << "Room name '" << name << "' too long.\n";
                  logStream << setLogfile( BOOTLOG ) << noTime << "BOOT FAILURE: Room name '" << name << "' too long.\n" << pod_send;
                  exit(1);
               }

               try
               {
                  shortcut = reader->GetAttribute( "shortcut" );
               }
               catch (NoSuchAttribute &e)
               {
                  std::cerr << "          " << TALKER_NAME << ": " << "Required parameter shortcut missing.\n";
                  logStream << setLogfile( BOOTLOG ) << noTime << "BOOT FAILURE: Required parameter shortcut missing.\n" << pod_send;
                  exit(1);
               }

               if ( shortcut.size() > ROOM_LABEL_LEN)
               {
                  std::cerr << "          " << TALKER_NAME << ": " << "Room label '" << shortcut << "' too long.\n";
                  logStream << setLogfile( BOOTLOG ) << noTime << "BOOT FAILURE: Room label '" << shortcut << "' too long.\n" << pod_send;
                  exit(1);
               }

               try
               {
                  access = reader->GetAttribute( "access" );
               }
               catch (NoSuchAttribute &e)
               {
                  std::cerr << "          " << TALKER_NAME << ": " << "Required parameter access missing.\n";
                  logStream << setLogfile( BOOTLOG ) << noTime << "BOOT FAILURE: Required parameter access missing.\n" << pod_send;
                  exit(1);
               }

               try
               {
                  hidden = reader->GetAttribute( "hidden" );
               }
               catch (NoSuchAttribute &e)
               {
                  std::cerr << "          " << TALKER_NAME << ": " << "Required parameter hidden missing.\n";
                  logStream << setLogfile( BOOTLOG ) << noTime << "BOOT FAILURE: Required parameter hidden missing.\n" << pod_send;
                  exit(1);
               }

               /* Check for duplicate label or name */
               for(roomNode=roomsList.begin();roomNode != roomsList.end();roomNode++)
               {
                  if ( shortcut == (*roomNode)->label )
                  {
                     std::cerr << "          " << TALKER_NAME << ": " << "Duplicate room label " << shortcut << ".\n";
                     logStream << setLogfile( BOOTLOG ) << noTime << "BOOT FAILURE: Duplicate room label " << shortcut << ".\n" << pod_send;
                     exit(1);
                  }
                  if ( name == (*roomNode)->name )
                  {
                     std::cerr << "          " << TALKER_NAME << ": " << "Duplicate room name " << name << ".\n";
                     logStream << setLogfile( BOOTLOG ) << noTime << "BOOT FAILURE: Duplicate room name " << name << ".\n" << pod_send;
                     exit(1);
                  }
               }

               if( hidden == "true" )
               {
                  visible = 0;
               }
               else if( hidden == "false" )
               {
                  visible = 1;
               }
               else
               {
                  std::cerr << "          " << TALKER_NAME << ": " << "Unknown hidden value '" << hidden << "'.\n";
                  logStream << setLogfile( BOOTLOG ) << noTime << "BOOT FAILURE: Unknown hidden value '" << hidden << "'.\n" << pod_send;
                  exit(1);
               }

               room = new Room( shortcut,name,visible );
               roomsList.push_back(room);

               if( access == "BOTH" )
               {
                  room->access = ROOM_PUBLIC;
               }
               else if ( access == "PUB" )
               {
                  room->access = ROOM_FIXED_PUBLIC;
               }
               else if ( access == "PRIV" )
               {
                  room->access = ROOM_FIXED_PRIVATE;
               }
               else
               {
                  std::cerr << "          " << TALKER_NAME << ": " << "Unknown room access type '" << access << "'.\n";
                  logStream << setLogfile( BOOTLOG ) << noTime << "BOOT FAILURE: Unknown room access type '" << access << "'.\n" << pod_send;
                  exit(1);
               }

            }
            else if( inRoomElement && name == "link" )
            {
               pod_string link;

               try
               {
                  link = reader->GetAttribute( "shortcut" );
               }
               catch (NoSuchAttribute &e)
               {
                  std::cerr << "          " << TALKER_NAME << ": " << "Link label '" << link << "' too long.\n";
                  logStream << setLogfile( BOOTLOG ) << noTime << "BOOT FAILURE: Link label '" << link << "' too long.\n" << pod_send;
                  exit(1);
               }

               room->addLinkLabel(link);
            }
         }
         else if( type == XML_READER_TYPE_END_ELEMENT )
         {
            if( inRoomElement && name == "room" )
            {
               inRoomElement = false;
            }
            else if( depth == roomsDepth )
            {
               return;
            }
         }
      }

      hasRead = reader->Read();
   }
}

int load_and_parse_config(char *confile)
{
   pod_string filename;
   pod_ostringstream filenameStream;

   filenameStream << DATAFILES << "/" << confile;
   filename = filenameStream.str();

   bool hasRead = false;

   bool inConfigElement = false;

   bool foundInit  = false;
   bool foundRooms = false;

   std::cout << "-- BOOT : Parsing config file \"" << filename << "\"...\n";

   XmlTextReader *reader = 0;

   try
   {
      reader = new XmlTextReader( filename );
   }
   catch (XmlError &e)
   {
      std::cerr << "          " << TALKER_NAME << ": " << "Can't open config file: " << filename << ".\n";
      logStream << setLogfile( BOOTLOG ) << noTime << "BOOT FAILURE: Can't open config file: " << filename << ".\n" << pod_send;
      exit(1);
   }

   hasRead = reader->Read();
   while( hasRead )
   {
      int type = reader->NodeType();
      if( type == XML_READER_TYPE_ELEMENT || type == XML_READER_TYPE_END_ELEMENT )
      {
         pod_string name = reader->Name();

         if( type == XML_READER_TYPE_ELEMENT )
         {
            if( name == "podconfig" )
            {
               inConfigElement = true;
            }
            else if( name == "init" &&  inConfigElement == true )
            {
               parse_init_section( reader);
               foundInit = true;
            }
            else if( name == "rooms" &&  inConfigElement == true )
            {
               parse_rooms_section( reader );
               foundRooms = true;
            }
         }
         else if( type == XML_READER_TYPE_END_ELEMENT )
         {
            if( name == "podconfig" && inConfigElement == true )
            {
               inConfigElement = false;
            }
         }

      }

      hasRead = reader->Read();
   }

   delete reader;


   if( !foundInit )
   {
      std::cerr << "          " << TALKER_NAME << ": " << "INIT section missing from config file.\n";
      logStream << setLogfile( BOOTLOG ) << noTime << "BOOT FAILURE: INIT section missing from config file.\n" << pod_send;
      exit(1);
   }

   if( !foundRooms )
   {
      std::cerr << "          " << TALKER_NAME << ": " << "ROOMS section missing from config file.\n";
      logStream << setLogfile( BOOTLOG ) << noTime << "BOOT FAILURE: ROOMS section missing from config file.\n" << pod_send;
      exit(1);
   }

   if (roomsList.size() == 0)
   {
      std::cerr << "          " << TALKER_NAME << ": " << "No rooms configured in config file.\n";
      logStream << setLogfile( BOOTLOG ) << noTime << "BOOT FAILURE: No rooms configured in config file.\n" << pod_send;
      exit(1);
   }

   setup_rooms();

   return 0;
}

int proc_show_configvars(UR_OBJECT user)
{
   ConfigVarsVector::iterator configVarNode;

   write_seperator_line(user,"configvars list");

   for(configVarNode=configVars.begin();configVarNode!=configVars.end();configVarNode++)
   {
      const pod_string &test =(*configVarNode)->toText();
      write_user(user,test.c_str());
   }

   write_seperator_line(user,NULL);
   write_user(user,"\n");

   return 0;
}

int setup_rooms()
{
   RoomsVector::iterator roomNode;

   for(roomNode=roomsList.begin();roomNode != roomsList.end();roomNode++)
   {
      (*roomNode)->setup_room_links();
      (*roomNode)->load_desc();
   }

   return 0;
}



