#include <string.h>
#include <iomanip>
#include <iostream>
#include "general_headers.h"
#include "string_misc.h"
#include "smail.h"
#include "ignore.h"
#include "StringLibrary.h"
#include "loadsave_user.h"
#include "time_utils.h"
#include "examine_stat.h"

extern char *noyes2[];
extern char *who_user[];
extern char *offon[];

typedef std::map<pod_string, pod_string, std::less<pod_string>, pod_alloc< std::pair<pod_string, pod_string> >::Type > AttributesMap;

class UserExaminer
{
   protected:
      enum { ONLINE,
            OFFLINE,
            CLOAKED };

      int        poutySlitStatus;
      UR_OBJECT  poutySlit;
      UR_OBJECT  prodder;
      pod_string poutySlitName;

   public:
      void showLastLogin( UR_OBJECT user );
      void examine( UR_OBJECT user );
      void status( UR_OBJECT user );
      UserExaminer();

   protected:
      void findSlitProddableDolphin( );
      void releaseSlitProddableDolphin( );
      void cloakSwitchSlitProddableDolphin( );
      void handleField( AttributesMap attributesMap, std::ostream &userStream );
      void parseXmlfile( pod_string filename, std::ostream &userStream );
};

int last_login(UR_OBJECT user)
{
   UserExaminer examiner;
   examiner.showLastLogin( user );
   return 0;
}

int examine(UR_OBJECT user)
{
   UserExaminer examiner;
   examiner.examine( user );
   return 0;
}

int ustatus(UR_OBJECT user)
{
   UserExaminer examiner;
   examiner.status( user );
   return 0;
}

UserExaminer::UserExaminer()
{
   poutySlitStatus = 0;
   poutySlit       = 0;
   prodder         = 0;
   poutySlitName   = "";
}

void UserExaminer::showLastLogin( UR_OBJECT user )
{
   if (words.word_count<2)
   {
      write_user(user,"Last who?\n");
      return;
   }

   prodder       = user;
   poutySlitName = words.word[1];

   findSlitProddableDolphin( );

   if( poutySlit == 0 )
   {
      return;
   }

   int timelen = (int)(time(0) - poutySlit->last_login);
   int days    = timelen / 86400;
   int hours   = ( timelen % 86400 ) / 3600;
   int mins    = ( timelen % 3600 ) / 60;

   WriteUserStream *userStream = WriteUserStream::getInstance();

   if (poutySlitStatus != ONLINE)
   {
      *userStream << addUser( user )
                 << "\n~OL" << poutySlit->name << " was last logged in: ~FG" << ctime( (time_t*) &poutySlit->last_login )
                 << "~OLWhich was: ~FG" << days << " ~FWdays,~FG " << hours << " ~FWhours, and ~FG" << mins << " ~FWminutes ago\n\n"
                 << pod_send;
   }
   else
   {
      *userStream << addUser( user )
                 << "\n~OL" << poutySlit->name << " has been logged in since: ~FG" << ctime( (time_t*) &poutySlit->last_login )
                 << pod_send;
   }

   releaseSlitProddableDolphin();

   return;
}

void UserExaminer::findSlitProddableDolphin( )
{
   StringLibrary * stringLibrary = StringLibrary::getInstance();
   int load_check;

   #warning not neat
   poutySlit = get_user_exactmatch( (char*)poutySlitName.c_str() );

   if( poutySlit == 0 )
   {
      poutySlitStatus = OFFLINE;
   }
   else if(poutySlit->cloaked && !(poutySlit->level <= prodder->level))
   {
      poutySlitStatus = CLOAKED;
   }
   else
   {
      poutySlitStatus = ONLINE;
   }

   if ( poutySlitStatus == OFFLINE )
   {
      poutySlit = create_user();

      if ( poutySlit == 0 )
      {
         write_userf( prodder, "%s: unable to create temporary user object.\n",stringLibrary->makeString("syserror").c_str() );
         write_syslog("ERROR: Unable to create temporary user object in getSlitProddableDolphin().\n",1);
         return;
      }
      strcpy( poutySlit->name, poutySlitName.c_str() );
      load_check = load_user( poutySlit );

      if (!load_check)
      {
         write_userf( prodder, "%s\n",stringLibrary->makeString("nosuchuser").c_str() );
         destruct_user( poutySlit );
         poutySlit = 0;
         return;
      }
   }

   if( poutySlitStatus == CLOAKED )
   {
      cloakSwitchSlitProddableDolphin();
   }
}

void UserExaminer::releaseSlitProddableDolphin( )
{
   if( poutySlitStatus == CLOAKED )
   {
      cloakSwitchSlitProddableDolphin();
   }

   if (poutySlitStatus == OFFLINE )
   {
      destruct_user( poutySlit );
   }
}

void UserExaminer::cloakSwitchSlitProddableDolphin( )
{
   int  last_login;
   int  last_login_len;
   char last_site[MAX_SITE_LEN+1];

   // store actual values into temp
   last_login       = poutySlit->cloak_store_last_login;
   last_login_len   = poutySlit->cloak_store_last_login_len;
   strcpy( last_site, poutySlit->cloak_store_last_site );

   // put back cloaked values
   poutySlit->cloak_store_last_login       = poutySlit->last_login;
   poutySlit->cloak_store_last_login_len   = poutySlit->last_login_len;
   strcpy( poutySlit->cloak_store_last_site, poutySlit->last_site );

   // restore actual values
   poutySlit->last_login       = last_login;
   poutySlit->last_login_len   = last_login_len;
   strcpy( poutySlit->last_site, last_site );
}

void UserExaminer::handleField( AttributesMap attributesMap, std::ostream &userStream )
{
   AttributesMap::iterator iterator;

   iterator = attributesMap.find( "name" );
   if(iterator == attributesMap.end())
   {
      logStream << setLogfile( SYSLOG ) << "Field with no name.\n" << pod_send;
      return;
   }
   pod_string fieldName = iterator->second;

   iterator = attributesMap.find( "width" );
   if(iterator != attributesMap.end())
   {
      userStream << std::setw( stringToInt(iterator->second) );
   }

   std::ios_base::fmtflags flags = userStream.flags();

   iterator = attributesMap.find( "align" );
   if(iterator != attributesMap.end())
   {
      if(iterator->second == "left")
      {
         userStream << setiosflags(std::ios_base::left);
      }
      if(iterator->second == "right")
      {
         userStream << setiosflags(std::ios_base::right);
      }
   }

   if( fieldName == "name" )
   {
      userStream << poutySlit->name;
   }
   else if( fieldName == "desc" )
   {
      StrGlobalVar *desc = (StrGlobalVar*)poutySlit->getVar("desc");
      userStream << desc->get();
   }
   else if( fieldName == "species" )
   {
      StrGlobalVar *species = (StrGlobalVar*)poutySlit->getVar("species");
      userStream << species->get();
   }
   else if( fieldName == "enter_phrase" )
   {
      StrGlobalVar *enter_phrase  = (StrGlobalVar*)poutySlit->getVar("enter_phrase");
      userStream << enter_phrase->get();
   }
   else if( fieldName == "leave_phrase" )
   {
      StrGlobalVar *leave_phrase  = (StrGlobalVar*)poutySlit->getVar("leave_phrase");
      userStream << leave_phrase->get();
   }
   else if( fieldName == "in_phrase" )
   {
      StrGlobalVar *in_phrase = (StrGlobalVar*)poutySlit->getVar("in_phrase");
      userStream << in_phrase->get();
   }
   else if( fieldName == "out_phrase" )
   {
      StrGlobalVar *out_phrase = (StrGlobalVar*)poutySlit->getVar("out_phrase");
      userStream << out_phrase->get();
   }
   else if( fieldName == "bday" )
   {
      StrGlobalVar *bday = (StrGlobalVar*)poutySlit->getVar("Squeeze_out_of_a_tight_slit_day");
      userStream << bday->get();
   }
   else if( fieldName == "Age" )
   {
      StrGlobalVar *age = (StrGlobalVar*)poutySlit->getVar("Age");
      userStream << age->get();
   }
   else if( fieldName == "Noise" )
   {
      StrGlobalVar *noise = (StrGlobalVar*)poutySlit->getVar("Noise");
      if( noise->get().length() != 0 )
      {
         userStream << noise->get();
      }
      else
      {
         userStream << "#NONE";
      }
   }
   else if( fieldName == "PrefRoom" )
   {
      StrGlobalVar *prefroom = (StrGlobalVar*)poutySlit->getVar("PrefRoom");
      userStream << prefroom->get();
   }
   else if( fieldName == "Icq" )
   {
      StrGlobalVar *icq = (StrGlobalVar*)poutySlit->getVar("Icq");
      userStream << icq->get();
   }
   else if( fieldName == "Gender" )
   {
      StrGlobalVar *gend_desc = (StrGlobalVar*)poutySlit->getVar("Gender");
      userStream << gend_desc->get();
   }
   else if( fieldName == "GendChoice" )
   {
      StrGlobalVar *gend_choice = (StrGlobalVar*)poutySlit->getVar("GendChoice");
      userStream << gend_choice->get();
   }
   else if( fieldName == "ColorOn" )
   {
      IntGlobalVar *color_on = (IntGlobalVar*)poutySlit->getVar("ColorOn");
      userStream << offon[color_on->get()];
   }
   else if( fieldName == "Prompt" )
   {
      userStream << offon[poutySlit->prompt];
   }
   else if( fieldName == "Autoread" )
   {
      userStream << offon[poutySlit->autoread];
   }
   else if( fieldName == "Visible" )
   {
      userStream << noyes2[poutySlit->vis];
   }
   else if( fieldName == "HideEmail" )
   {
      userStream << noyes2[poutySlit->hide_email];
   }
   else if( fieldName == "HideUrl" )
   {
      userStream << noyes2[poutySlit->hide_url];
   }
   else if( fieldName == "Email" )
   {
      StrGlobalVar *email = (StrGlobalVar*)poutySlit->getVar("Email");
      if( poutySlit->hide_email && (prodder->level < LEV_FOU) && (prodder != poutySlit) )
      {
         userStream << "<email address only viewable by wizards>";
      }
      else
      {
         userStream << email->get();
      }
   }
   else if( fieldName == "Url" )
   {
      StrGlobalVar *url = (StrGlobalVar*)poutySlit->getVar("Url");
      if( poutySlit->hide_url && (prodder->level < LEV_FOU) && (prodder != poutySlit) )
      {
         userStream << "<homepage url only viewable by wizards>";
      }
      else
      {
         userStream << url->get();
      }
   }
   else if( fieldName == "Level" )
   {
      userStream << getLevelName(poutySlit->level);
   }
   else if( fieldName == "Mode" )
   {
      if( poutySlit->command_mode == 1 )
      {
         userStream << "COMMAND";
      }
      else
      {
         userStream << "SPEECH";
      }
   }
   else if( fieldName == "Arrested" )
   {
      userStream << noyes2[poutySlit->level == LEV_MIN];
   }
   else if( fieldName == "Newmail" )
   {
      userStream << noyes2[has_unread_mail(poutySlit)];
   }
   else if( fieldName == "Muzzled" )
   {
      userStream << noyes2[(poutySlit->muzzled>0)];
   }
   else if( fieldName == "Ignoring" )
   {
      userStream << ignore_stat(poutySlit);
   }
   else if( fieldName == "Invited" )
   {
      if( poutySlit->invite_room )
      {
         userStream << poutySlit->invite_room->name;
      }
      else
      {
         userStream << "<nowhere>";
      }
   }
   else if( fieldName == "OnlineMinutes" )
   {
      userStream << (int)(time(0) - poutySlit->last_login)/60;
   }
   else if( fieldName == "Room" )
   {
      if(poutySlit->room == 0 )
      {
         userStream << "(null)";
      }
      else
      {
         if(poutySlit->room->secret)
         {
            userStream << "<unavailable>";
         }
         else
         {
            userStream << poutySlit->room->name;
         }
      }
   }
   else if( fieldName == "Timezone" )
   {
      IntGlobalVar *timezone = (IntGlobalVar*)poutySlit->getVar("timezone");
      if( timezone->get() >= 0 )
      {
         userStream << "UTC +" << timezone->get();
      }
      else
      {
         userStream << "UTC " << timezone->get();
      }
   }
   else if( fieldName == "Usertime" )
   {
      IntGlobalVar *timezone = (IntGlobalVar*)poutySlit->getVar("timezone");

      struct tm *tm_struct;
      tm_struct = get_current_utctime();
      int localMin  = tm_struct->tm_min;
      int localHour = (tm_struct->tm_hour + timezone->get() + 24) % 24;

      userStream << std::setw(2) << localHour << ":" << std::setw(2) << localMin;
   }
   else if( fieldName == "Site" )
   {
      if( poutySlit->socket == 0 )
      {
         userStream << "(offline)";
      }
      else
      {
         userStream << poutySlit->socket->getPeerSite();
      }
   }
   else if( fieldName == "Port" )
   {
      if( poutySlit->socket == 0 )
      {
         userStream << "(offline)";
      }
      else
      {
         userStream << poutySlit->socket->getPeerPort();
      }
   }
   else if( fieldName == "LastSite" )
   {
      userStream << poutySlit->last_site;
   }
   else if( fieldName == "TotalLogin" )
   {
      userStream << time2string(FALSE,poutySlit->total_login);
   }
   else if( fieldName == "TotalActive" )
   {
      userStream << time2string(FALSE,poutySlit->total_active_time);
   }
   else if( fieldName == "TotalIdle" )
   {
      userStream << time2string(FALSE,poutySlit->total_idle_time);
   }
   else if( fieldName == "Since" )
   {
      pod_string since_time = ctime(&poutySlit->total_active_first);
      since_time.erase( since_time.length() - 1, 1 );
      userStream << since_time;
   }
   else if( fieldName == "LastLogin" )
   {
      pod_string last_login_time = ctime((time_t*)&poutySlit->last_login);
      last_login_time.erase( last_login_time.length() - 1, 1 );
      userStream << last_login_time;
   }
   else if( fieldName == "LastLoginAgo" )
   {
      userStream << time2string(FALSE,(int)(time(0) - poutySlit->last_login));
   }
   else if( fieldName == "LastLoginLen" )
   {
      userStream << poutySlit->last_login_len/3600 << " hours, " << (poutySlit->last_login_len%3600)/60 << " minutes";
   }
   else if( fieldName == "Idle" )
   {
      userStream << get_idle_mins(poutySlit);
   }
   else if( fieldName == "AfkMessage" )
   {
      StrGlobalVar *afk_mesg    = (StrGlobalVar*)poutySlit->getVar("afk_mesg");

      if ( afk_mesg->get().length() > 0 )
      {
         userStream << afk_mesg->get();
      }
      else
      {
         userStream << "<unset>";
      }
   }
   else if( fieldName == "Profile" )
   {
      pod_string filename;

      filename = USERFILES;
      filename += "/";
      filename += poutySlit->name;
      filename += ".P";

      PodFile file( filename );

      if( file.open_read() != 0 )
      {
         userStream << "No Profile.\n";
      }
      else
      {
         pod_string buffer;

         buffer = file.fgets();
         while( buffer.length() != 0 )
         {
            userStream << buffer;
            buffer = file.fgets();
         }

         file.close();
      }
   }
// games
   else if( fieldName == "nerf_win" )
   {
      IntGlobalVar *nerf_win = (IntGlobalVar*)poutySlit->getVar("nerf_win");
      userStream << nerf_win->get();
   }
   else if( fieldName == "nerf_lose" )
   {
      IntGlobalVar *nerf_lose = (IntGlobalVar*)poutySlit->getVar("nerf_lose");
      userStream << nerf_lose->get();
   }
   else if( fieldName == "nerflife" )
   {
      IntGlobalVar *nerflife = (IntGlobalVar*)poutySlit->getVar("nerflife");
      userStream << nerflife->get();
   }
   else if( fieldName == "nerfcharge" )
   {
      IntGlobalVar *nerfcharge = (IntGlobalVar*)poutySlit->getVar("nerfcharge");
      userStream << nerfcharge->get();
   }
   else if( fieldName == "baggies" )
   {
      IntGlobalVar *baggies = (IntGlobalVar*)poutySlit->getVar("baggies");
      userStream << baggies->get();
   }
   else if( fieldName == "squidos" )
   {
      IntGlobalVar *squidos = (IntGlobalVar*)poutySlit->getVar("squidos");
      userStream << squidos->get();
   }
   else if( fieldName == "Width" )
   {
      IntGlobalVar *width = (IntGlobalVar*)poutySlit->getVar("Width");
      userStream << width->get();
   }   
   else if( fieldName == "Height" )
   {
      IntGlobalVar *height = (IntGlobalVar*)poutySlit->getVar("Height");
      userStream << height->get();
   }   
// !games
   else
   {
      logStream << setLogfile( SYSLOG ) << "Field with unrecognized name: " << fieldName << ".\n" << pod_send;
   }

   userStream.flags ( flags );
}

/*
 * This function should be called when the reader is pointed at an element.
 * It will then parse the content of this element, up to and including it's
 * closing element tag, ignoring anything it finds.
 */
void jumpOver( XmlTextReader *reader )
{
   bool ret       = 0;
   int type       = 0;
   int depth      = 0;
   int startDepth = 0;

   startDepth = reader->Depth();
      
   type = reader->NodeType();

   if(type != XML_READER_TYPE_ELEMENT)
   {
      logStream << setLogfile( SYSLOG ) << "jumpOver should only be called from an element.\n" << pod_send;
      return;
   }

   while ( true )
   {
      ret = reader->Read();

      if( !ret )
      {
         logStream << setLogfile( SYSLOG ) << "jumpOver failed parsing before finding closing element.\n" << pod_send;
         return;
      }

      type = reader->NodeType();
      depth = reader->Depth();

      if( depth == startDepth && type == XML_READER_TYPE_END_ELEMENT )
      {
         break;
      }
   }
}

class ParseError: XmlError
{
   public:
      ParseError( pod_string errorMsg );
};

ParseError::ParseError( pod_string errorMsg ) : XmlError( errorMsg )
{
}

void UserExaminer::parseXmlfile( pod_string filename, std::ostream &userStream )
{
   bool inDocumentElement = false;
   bool madeChoice = false;
   bool renderContent = false;
   bool ret = 0;
   int type = 0;

   XmlTextReader *reader = 0;
   
   try
   {
      reader = new XmlTextReader( filename );   
   }
   catch (XmlError &e)
   {
      logStream << setLogfile( SYSLOG ) << "Failed to construct XmlTextReader for file: " << filename << std::endl;
      return;
   }

   while ( true )
   {
      ret = reader->Read();
      if( !ret )
      {
         break;
      }

      type = reader->NodeType();

      pod_string name = reader->Name();

      if( type == XML_READER_TYPE_ELEMENT )
      {
         if( name == "document" )
         {
            inDocumentElement = true;
            renderContent = true;
         }
         else if( inDocumentElement )
         {
            if( name == "choose" )
            {
               madeChoice = false;
               renderContent = false;
            }
            else if( name == "when" )
            {
               bool ignore = true;

               if( madeChoice != true )
               {
                  try
                  {
                     pod_string status = reader->GetAttribute( "status" );
                     if(status == "online")
                     {
                        if( poutySlitStatus == ONLINE )
                        {
                           ignore = false;
                        }
                     }
                     else if(status == "authorized")
                     {
                        if ( poutySlit == prodder || prodder->level >= LEV_THR )
                        {
                           ignore = false;
                        }
                     }
                     else if(status == "afk")
                     {
                        if ( poutySlit->afk )
                        {
                           ignore = false;
                        }
                     }
                     else
                     {
                        logStream << setLogfile( SYSLOG ) << "Unrecognized status: " << status << ".\n" << pod_send;
                     }
                  }
                  catch (NoSuchAttribute &e)
                  {
                     logStream << setLogfile( SYSLOG ) << "Element when with no status attribute.\n" << pod_send;
                  }
               }

               if( ignore )
               {
                  jumpOver(reader);
                  renderContent = false;
               }
               else
               {
                  madeChoice = true;
                  renderContent = true;
               }
            }
            else if( name == "otherwise" )
            {
               if(madeChoice == true)
               {
                  jumpOver(reader);
                  renderContent = false;
               }
               else
               {
                  madeChoice = true;
                  renderContent = true;
               }
            }
            else if( renderContent )
            {
               if( name == "hr" )
               {
                  pod_string caption;

                  try
                  {
                     caption = reader->GetAttribute( "caption" );
                  }
                  catch (NoSuchAttribute &e)
                  {
                     caption = "";
                  }

                  userStream << gen_seperator_line( (char*)caption.c_str() );
               }
               if( name == "field" )
               {
                  AttributesMap attributesMap;

                  int attributeCount = reader->AttributeCount();
                  for(int cnt = 0; cnt < attributeCount; cnt ++)
                  {
                     reader->MoveToAttribute( cnt );

                     attributesMap[ reader->Name() ] = reader->Value();
                  }
                  handleField( attributesMap, userStream );
               }
            }
         }
      }
      else if( type == XML_READER_TYPE_END_ELEMENT )
      {
         if( name == "document" && inDocumentElement == true )
         {
            inDocumentElement = false;
            renderContent = false;
         }
         else if( name == "hr" )
         {
         }
         else if( name == "field" )
         {
         }
         else if( name == "choose" )
         {
            renderContent = true;
            madeChoice = true;
         }
         else if( name == "when"  )
         {
            renderContent = false;
         }
         else if( name == "otherwise" )
         {
            renderContent = false;
         }
      }
      else if( type == XML_READER_TYPE_TEXT ||
               type == XML_READER_TYPE_SIGNIFICANT_WHITESPACE )
      {
         if( renderContent )
         {
            userStream << reader->Value();
         }
      }
   }

   if(ret != 0)
   {
      logStream << setLogfile( SYSLOG ) << "Error parsing file: " << filename << ".\n" << pod_send;
   }

   delete reader;
}

void UserExaminer::status(UR_OBJECT user)
{
   prodder = user;

   if ( words.word_count < 2 )
   {
      poutySlitName = prodder->name;
   }
   else
   {
      poutySlitName = words.word[1];
   }

   findSlitProddableDolphin( );

   if( poutySlit == 0 )
   {
      return;
   }

   pod_string filename = "datafiles/ustatus.xml";

   WriteUserStream *userStream = WriteUserStream::getInstance();

   *userStream << addUser( user );

   parseXmlfile( filename, *userStream );

   *userStream << pod_send;

   if ( poutySlitStatus == ONLINE )
   {
      if( poutySlit != user && !poutySlit->ignall )
      {
         write_userf(poutySlit,"~OL%s click-trains and focuses a narrow echo-pulse at you.\n",get_visible_name(user).c_str());
      }
   }

   releaseSlitProddableDolphin();

   return;
}

void UserExaminer::examine(UR_OBJECT user)
{
   prodder = user;

   if ( words.word_count < 2 )
   {
      poutySlitName = prodder->name;
   }
   else
   {
      poutySlitName = words.word[1];
   }

   findSlitProddableDolphin( );

   if( poutySlit == 0 )
   {
      return;
   }

   pod_string filename = "datafiles/examine.xml";

   WriteUserStream *userStream = WriteUserStream::getInstance();

   *userStream << addUser( user );

   parseXmlfile( filename, *userStream );

   *userStream << pod_send;

   if ( poutySlitStatus == ONLINE )
   {
      if( (poutySlit != user) && !poutySlit->ignall )
      {
         write_userf(poutySlit,"~OL%s has just scanned you!\n",get_visible_name(user).c_str());
      }
   }

   releaseSlitProddableDolphin();

   return;
}
