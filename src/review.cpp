#include <sstream>
#include <string>
#include <string.h>
#include "general_headers.h"
#include "StringLibrary.h"
#include "help.h"
#include "cmd_main.h"
#include "string_misc.h"
#include "globals.h"
#include "xalloc.h"
#include "./simple_types/vars.h"
#include "review.h"

#define REVIEW_LINES   30
#define REVIEW_LEN     400

class ReviewVar : public BasicVar
{
   protected:
      int  line;
      pod_string buff[REVIEW_LINES];

   public:
      ReviewVar(pod_string name);
      ReviewVar(pod_string name, int flag);
      void   toXML( xmlTextWriterPtr ptr );
      void   fromXML( XmlTextReader * reader );
      int    init();
      pod_string toText();
      int    record(char *str);
      int    clear();
      int    review(UR_OBJECT user, char *rev_name);

      virtual ~ReviewVar(){};
};

class ReviewObjectCreator : public ObjectCreator
{
   public:
      BasicVar *getObjectInstance(pod_string name);
};

ReviewVar wrevbuff("wrev",EVERY_BOOT);
ReviewVar shoutbuff("shout",EVERY_BOOT);

BasicVar* ReviewObjectCreator::getObjectInstance(pod_string name)
{
   return new ReviewVar(name);
};

ReviewVar::ReviewVar(pod_string name) : BasicVar(name,0)
{
   int i;

   for(i=0;i<REVIEW_LINES;++i) buff[i]="";
   line=0;
}

ReviewVar::ReviewVar(pod_string name, int flag) : BasicVar(name, flag)
{
   int i;

   for(i=0;i<REVIEW_LINES;++i) buff[i]="";
   line=0;
}
   
int ReviewVar::init()
{
   int i;

   for(i=0;i<REVIEW_LINES;++i) buff[i]="";
   line=0;

   return 0;
}

void ReviewVar::toXML( xmlTextWriterPtr ptr )
{
   pod_string lineString = intToString( line );

   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("\n"));
   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("      "));
   xmlTextWriterStartElement( ptr, reinterpret_cast<const xmlChar *>("item") );
   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>( lineString.c_str() ));
   xmlTextWriterEndElement( ptr );
   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("\n"));

   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("      "));
   xmlTextWriterStartElement( ptr, reinterpret_cast<const xmlChar *>("array") );
   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("\n"));
   for( int i = 0; i < REVIEW_LINES; i++)
   {
      if( !buff[i].empty() )
      {
         xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("         "));
         xmlTextWriterStartElement( ptr, reinterpret_cast<const xmlChar *>("val") );
         xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>( buff[i].c_str() ));
         xmlTextWriterEndElement( ptr );
         xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("\n"));
      }
   }
   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("      "));
   xmlTextWriterEndElement( ptr );
   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("\n"));
   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("   "));

}

pod_string ReviewVar::toText()
{
   int i;
   pod_ostringstream outputStream;

   outputStream.width(22);
   outputStream.setf(std::ios::left);
   outputStream << name.c_str() << " :" << std::endl;
   for(i=0;i < REVIEW_LINES;i++)
   {
      if(!buff[i].empty())
      {
         outputStream.width(22);
         outputStream.setf(std::ios::left);
         outputStream << i << " : \"" << "-" /*<< buff[i]*/ << "\"" << std::endl;
      }
   }
   return outputStream.str();
}

int ReviewVar::record(char *str)
{
   pod_string temp_buff;

   temp_buff = str;
   temp_buff = temp_buff.substr(0,REVIEW_LEN-1);
   terminate(temp_buff);

   buff[line] = temp_buff;
  
   line=(line+1)%REVIEW_LINES;
   return 0;
}

int ReviewVar::clear()
{
   int c;
  
   for(c=0;c<REVIEW_LINES;++c) buff[c]="";
   line=0;
   return 0;
}

int ReviewVar::review(UR_OBJECT user, char *rev_name)
{
   int i,curLine,cnt;

   cnt=0;
   for(i=0;i<REVIEW_LINES;++i) 
   {
      curLine=(line+i)%REVIEW_LINES;
      if (!buff[curLine].empty())
      {
         cnt++;
         if (cnt==1) 
         {
            write_userf(user,"\n~BB~FG*** %s ***\n\n",rev_name);
         }
         write_userf(user,"%s\n",buff[curLine].c_str());
      }
   }
   if (!cnt) write_user(user,"Review buffer is empty.\n");
   else write_user(user,"\n~BB~FG*** End ***\n\n");

   return 0;
}


/*** Records tells and pemotes sent to the user. ***/
int record_tell(UR_OBJECT user,char *str)
{
  return ((ReviewVar*)(user->getVar("rev")))->record(str);
}

/*** Record speech and emotes in the room. ***/
int record(RM_OBJECT rm, char *str)
{
  return ( (ReviewVar*)( rm->getVar("rev") ) )->record(str);
}

/*** Record wizard speech and emotes in the room. (Wizzes only) added by Vaghn 1999 ***/
int wrecord(char *str)
{
  return wrevbuff.record(str);
}

/*** Records shouts and shemotes sent over the talker. ***/
int record_shout(char *str)
{
  return shoutbuff.record(str);
}

/*** Clear the tell buffer of a user ***/
int clear_tells(UR_OBJECT user)
{
   int retval = ((ReviewVar*)(user->getVar("rev")))->clear();
   write_user(user,"\nYour tells have now been cleared.\n\n");

   return retval;
}

/*** Clear the review buffer of a room ***/
int clear_revbuff(RM_OBJECT rm)
{ 
   return ( (ReviewVar*)( rm->getVar("rev") ) )->clear();
}

/*** Clear the wizard review buffer added by Vaghn 1999 ***/
int wclear_revbuff(UR_OBJECT user,char *inpstr)
{ 
   wrevbuff.clear(); 
   write_levelf(LEV_THR,1,user,"\nThe wizard buffer has now been cleared by %s.\n\n",user->name);
   write_userf(user,"\nYou have cleared the wizard buffer.\n\n");
   return 0;
}

/*** Clear the shout buffer of the talker ***/
int clear_shouts(UR_OBJECT user,char *inpstr)
{ 
   shoutbuff.clear(); 
   write_roomf(NULL,"\nShout buffer has now been cleared by %s.\n\n",user->name);
   return 0;
}

/*** Clear the review buffer ***/
int revclr(UR_OBJECT user)
{
   clear_revbuff(user->room);
   write_user(user,"Review buffer cleared.\n");
   write_room_exceptf(user->room,user,"%s has cleared the review buffer.\n",get_visible_name(user).c_str());

   return 0;
}

/*** view a rooms review buffer ***/
int review(UR_OBJECT user,char *inpstr)
{
   StringLibrary * stringLibrary = StringLibrary::getInstance();
   RM_OBJECT rm=user->room;
   char rev_name[81];

   if (words.word_count<2) rm=user->room;
   else 
   {
      if ((rm=get_room(words.word[1]))==NULL) 
      {
         write_userf(user,"%s\n",stringLibrary->makeString("nosuchroom").c_str());
         return 0;
      }
      if (user->level<LEV_FOU)
      {
         write_user(user,"Sorry but you can only review that room which you are in.\n");
         return 0;
      }
      if (!has_room_access(user,rm)) 
      {
         write_user(user,"That room is currently private, you cannot review the conversation.\n");
         return 0;
      }
   }

   sprintf(rev_name,"Review buffer for the %s",rm->name);

   ( (ReviewVar*)( rm->getVar("rev") ) )->review(user,rev_name);

   return 0;
}

/*** Show recorded tells and pemotes ***/
int revtell(UR_OBJECT user,char *inpstr)
{ 
  return ((ReviewVar*)(user->getVar("rev")))->review(user,"Your revtell buffer");
}

/*** See review of wizard conversation ***/
int wreview(UR_OBJECT user,char *inpstr)
{ 
  return wrevbuff.review(user,"Wizard review buffer"); 
}

/*** See review of shouts ***/
int revshout(UR_OBJECT user,char *inpstr)
{ 
  return shoutbuff.review(user,"Shout review buffer"); 
}

/*** register our global buffers ***/
void revbuffs_init()
{
   glob_add_var_cust(&wrevbuff);
   glob_add_var_cust(&shoutbuff);
   user_var_add_cust( "rev", new ReviewObjectCreator(), USR_SAVE_SOFT );
   addRoomVarTempl( "rev", new ReviewObjectCreator() );

   cmd_add("review",   LEV_ONE, SPCH, &review , '<');
   cmd_add("revtell",  LEV_TWO, SPCH, &revtell);
   cmd_add("revshout", LEV_TWO, SPCH, &revshout);
   cmd_add("ctell",    LEV_TWO, SPCH, &clear_tells);
   cmd_add("cshout",   LEV_FOU, SPCH, &clear_shouts);
   cmd_add("cbuff",    LEV_TWO, SPCH, &revclr);
   cmd_add("wreview",  LEV_THR, SPCH, &wreview );
   cmd_add("wcbuff",   LEV_THR, SPCH, &wclear_revbuff);
}

void ReviewVar::fromXML( XmlTextReader * reader )
{
   enum
   {
      UNSET,
      LINE_ITEM,
      LINE_ITEM_DONE,
      LINES_ARRAY,
      LINE_VAL,
      DONE
   };
   
   bool       hasRead       = true;
   bool       inItemElement = false;
   int        ourItemDepth  = -1;
   int        type          = 0;
   int        depth         = 0;
   int        subDepth      = 0;
   int        state         = UNSET;
   int        arrIndex      = 0;
   pod_string name;
   pod_string itemValue;
      

   while( hasRead )
   {
      type     = reader->NodeType();
      depth    = reader->Depth();
      subDepth = depth - ourItemDepth;
      
      if( type == XML_READER_TYPE_ELEMENT || type == XML_READER_TYPE_END_ELEMENT )
      {
         name = reader->Name();

         if( type == XML_READER_TYPE_ELEMENT )
         {
            if( inItemElement == false )
            {
               if( name == "item" )
               {
                  inItemElement = true;
                  ourItemDepth = depth;
               }
            }            
            else if( subDepth == 1 )
            {
               if( name == "item" && state == UNSET)
               {
                  state = LINE_ITEM;
               }
               else if( name == "array" && state == LINE_ITEM_DONE )
               {
                  state = LINES_ARRAY;
               }
            }
            else if( subDepth == 2 )
            {
               if( name == "val" && state == LINES_ARRAY )
               {
                  state = LINE_VAL;
               }
            }
         }
         else if( type == XML_READER_TYPE_END_ELEMENT && inItemElement )
         {
            if( depth == ourItemDepth && name == "item" )
            {
               inItemElement = false;
               ourItemDepth = -1;
               return;
            }
            else if( subDepth == 1 )
            {
               if( name == "item" && state == LINE_ITEM )
               {
                  state = LINE_ITEM_DONE;
               }
               else if( name == "array" && state == LINES_ARRAY )
               {
                  state = DONE;
               }
            }
            else if( subDepth == 2 )
            {
               if( name == "val" && state == LINE_VAL )
               {
                  state = LINES_ARRAY;
               }
            }
         }
      }
      else if( type == XML_READER_TYPE_TEXT && inItemElement )
      {
         itemValue = reader->Value();

         if( state == LINE_ITEM )
         {
            line = stringToInt( itemValue );
         }
         else if( state == LINE_VAL )
         {
            if( arrIndex == REVIEW_LINES )
            {
               write_syslogf("*** BOOT error: Review buffer can contain only %i lines.\n",FALSE,REVIEW_LINES);
            }
            else if( itemValue.length() > REVIEW_LEN )
            {
               write_syslogf("*** BOOT error: Review buffer strings can only be %i characters.\n",FALSE,REVIEW_LEN);
            }
            else
            {
               buff[arrIndex] = itemValue;
               arrIndex++;
            }
         }
      }

      hasRead = reader->Read();
   }
}

