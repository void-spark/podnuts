#include <unistd.h>
#include "general_headers.h"
#include "./simple_types/vars.h"
#include "file_io.h"
#include "clones.h"
#include "more.h"
#include "softboot.h"
#include "help.h"
#include "color.h"
#include "StringLibrary.h"
#include "loadsave_user.h"
#include "string_misc.h"
#include "account_reg.h"
#include "tandem.h"
#include "telnet.h"
#include "dynamicVarsController.h"
#include "UserTelnetHandler.h"
#include "user_objects.h"

DynamicVarsController userVarsController;

int user_var_add_cust( pod_string name,ObjectCreator *var, int save_type )
{
   return userVarsController.addVarTempl( name, var, save_type );
}

void stringToXML( xmlTextWriterPtr ptr, pod_string name, pod_string val )
{
   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("   "));
   xmlTextWriterStartElement( ptr, reinterpret_cast<const xmlChar *>("item") );
   xmlTextWriterWriteAttribute( ptr, reinterpret_cast<const xmlChar *>("name"), reinterpret_cast<const xmlChar *>( name.c_str() ) );
   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>( val.c_str()  ));
   xmlTextWriterEndElement( ptr );
   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("\n"));
}

void intToXML( xmlTextWriterPtr ptr, pod_string name, int val )
{
   pod_string intString = intToString( val );

   stringToXML( ptr, name, intString );
}

void user_struct::toXMLSoft( xmlTextWriterPtr ptr )
{
   pod_string typeString = intToString( type );

   xmlTextWriterStartElement( ptr, reinterpret_cast<const xmlChar *>("user") );
   xmlTextWriterWriteAttribute( ptr, reinterpret_cast<const xmlChar *>("name"), reinterpret_cast<const xmlChar *>(name) );
   xmlTextWriterWriteAttribute( ptr, reinterpret_cast<const xmlChar *>("type"), reinterpret_cast<const xmlChar *>(typeString.c_str()) );
   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("\n"));

   if( type == CLONE_TYPE )
   {
      stringToXML( ptr, "roomname", room->name );
   }
   else
   {   
      stringToXML( ptr, "site", socket->getPeerSite() );
      intToXML( ptr, "socket", socket->getFileDescriptor() );
      
      xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("   "));
      xmlTextWriterStartElement( ptr, reinterpret_cast<const xmlChar *>("item"));
      xmlTextWriterWriteAttribute( ptr, reinterpret_cast<const xmlChar *>("name"), reinterpret_cast<const xmlChar *>( "qMethod" ) );
      socket->toXML( ptr );
      xmlTextWriterEndElement( ptr );
      xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("\n"));
      
      stringToXML( ptr, "roomname", room->name );
      if( invite_room != 0 )
      {
         stringToXML( ptr, "invite_roomname", invite_room->name );
      }
      if( idle_before_room != 0 )
      {
         stringToXML( ptr, "idle_before_room", idle_before_room->name );
      }

      userVarsController.toXML( data_ptr, USR_SAVE_SOFT, ptr );

      stringToXML( ptr, "inpstr_old", inpstr_old );
      stringToXML( ptr, "boardwrite", boardwrite );

      if( nextCommand )
      {
         stringToXML( ptr, "nextCommand", nextCommand->getName() );
      }
      if( follow_partner != 0 )
      {
         stringToXML( ptr, "follow_partner", follow_partner->name );
      }

      intToXML( ptr, "follow_mode", follow_mode );
      intToXML( ptr, "misc_op", misc_op );
      intToXML( ptr, "filepos", filepos );
      intToXML( ptr, "afk", afk );

   //      file.write_time_t("last_input"  ,user->last_input  );

      intToXML( ptr, "vis", vis );
      intToXML( ptr, "edit_line", edit_line );
      intToXML( ptr, "charcnt", charcnt );
      intToXML( ptr, "idle_status", idle_status );
      intToXML( ptr, "clone_hear", clone_hear );
      intToXML( ptr, "edit_op", edit_op );
      intToXML( ptr, "ignall", ignall );
      intToXML( ptr, "ignall_store", ignall_store );
      intToXML( ptr, "inedit", inedit );
      intToXML( ptr, "cloaked", cloaked );
      if( cloaked )
      {
         intToXML( ptr, "CloakStoreUserTime",  cloak_store_last_login );
         intToXML( ptr, "CloakStoreLastLogin", cloak_store_last_login_len );
         stringToXML( ptr, "CloakStoreLastSite",  cloak_store_last_site );      
      }
      if( malloc_start != 0 )
      {
         pod_string buffer( malloc_start, malloc_end - malloc_start );

         if( !buffer.empty() )
         {
            stringToXML( ptr, "user_malloc", buffer );
         }
      }
   }
         
   xmlTextWriterEndElement( ptr );
   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("\n"));
}

void user_struct::toXMLPerm( xmlTextWriterPtr ptr )
{
   if( type == CLONE_TYPE) 
   {
      return;
   }
   
   xmlTextWriterStartElement( ptr, reinterpret_cast<const xmlChar *>("user") );
   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("\n"));   

   intToXML( ptr, "UserTime",  last_login );
   intToXML( ptr, "LastLogin", last_login_len );
   stringToXML( ptr, "LastSite",  last_site );
   
   intToXML( ptr, "TotalActiveFirst",  total_active_first );
   intToXML( ptr, "TotalActiveTime",   total_active_time );
   intToXML( ptr, "TotalIdleTime",     total_idle_time );
   intToXML( ptr, "TotalAfkTime",      total_afk_time );
   intToXML( ptr, "TotalLogin",        total_login );

   intToXML( ptr, "AutoRead",  autoread );
   intToXML( ptr, "AutoFwd",   autofwd );
   intToXML( ptr, "CmdMode",   command_mode );
   intToXML( ptr, "Cursed",    cursed );
   intToXML( ptr, "HideEmail", hide_email );
   intToXML( ptr, "HideUrl",   hide_url );
   intToXML( ptr, "HelpMode",  help_mode );
   intToXML( ptr, "Jailed",    jailed );
   intToXML( ptr, "Level",     level );
   intToXML( ptr, "Muzzled",   muzzled );
   intToXML( ptr, "MailVrfy",  mail_verified );
   intToXML( ptr, "NoDelold",  no_delold );
   intToXML( ptr, "Prompt",    prompt );
   intToXML( ptr, "Tag",       tag );

   stringToXML( ptr, "Curse_name", curse_name );
   stringToXML( ptr, "PassWord",   pass );

   userVarsController.toXML( data_ptr, USR_SAVE_ALWS, ptr );

   xmlTextWriterEndElement( ptr );
   xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("\n"));

   return;
}

BasicVar* user_struct::getVar( pod_string name )
{
   return userVarsController.objGetVar( data_ptr, name );
}

int user_vars_proc(UR_OBJECT user)
{
   UR_OBJECT user_node;

   write_seperator_line(user,"user_vars info");

   for( user_node = user_first; user_node; user_node=user_node->next )
   {
      write_userf(user,"%-22s : %s\n","username",user_node->name );

      userVarsController.objPrintVar(user_node->data_ptr,user);

      write_seperator_line(user,NULL);
   }
   write_user(user,"\n");
   return 0;
}

int get_num_of_users()
{
   UR_OBJECT u;
   int num_users;

   num_users=0;

   for(u=user_first;u!=NULL;u=u->next)
   {
      if (is_clone(u));
      else if (!u->login) num_users++;
   }
   return num_users;
}

int get_num_of_unhidden_users()
{
   UR_OBJECT u;
   int num_users;

   num_users=0;

   for(u=user_first;u!=NULL;u=u->next)
   {
      if (is_clone(u) ||
          !u->vis ||
          u->cloaked );
      else if (!u->login) num_users++;
   }
   return num_users;
}

int get_num_of_unhidden_wizs()
{
   UR_OBJECT u;
   int num_users;

   num_users=0;

   for(u=user_first;u!=NULL;u=u->next)
   {
      if (is_clone(u) ||
          !u->vis ||
          u->cloaked ||
          u->level < LEV_THR);
      else if (!u->login) num_users++;
   }
   return num_users;
}

int get_num_of_logins()
{
   UR_OBJECT u;
   int num_users;

   num_users=0;

   for(u=user_first;u!=NULL;u=u->next)
   {
      if (is_clone(u));
      else if (u->login) num_users++;
   }
   return num_users;
}

int user_exists(UR_OBJECT user)
{
   char filename[80];
   FILE *fp;

   sprintf(filename,"%s/%s.D.xml",USERFILES,user->name);

   if (!(fp=fopen(filename,"r"))) return 0;
   fclose(fp);

   return 1;

}

UR_OBJECT temp_user_spawn(UR_OBJECT user,char* name,char* caller)
{
   StringLibrary * stringLibrary = StringLibrary::getInstance();
   UR_OBJECT u;

   if (!(u=create_user_adv(FALSE)))
   {
      write_syslogf("ERROR: Unable to create temporary user object in %s.\n",FALSE,caller);
      if(user) write_userf(user,"%s: unable to create temporary user object.\n",stringLibrary->makeString("syserror").c_str());
      return NULL;
   }
   temp_user_count++;
   strcpy(u->name,name);
   if (!load_user(u))
   {
      temp_user_destroy(u);
      if(!user) write_syslogf("ERROR: Unable to load user details in %s.\n",FALSE,caller);
      else write_user_crt(user,stringLibrary->makeString("nosuchuser").c_str());
      return NULL;
   }
   return u;
}

int temp_user_destroy(UR_OBJECT user)
{
   UR_OBJECT u;

   /* check if it's a temp user or an online user */
   for(u=user_first;u!=NULL;u=u->next)
   {
      if(u==user) return 0;
   }
   destruct_user(user);

   temp_user_count--;

   return 0;
}

/* returns user with name 'name' , or NULL if not found */
UR_OBJECT get_user_advanced( UR_OBJECT user, pod_string name, int verbosity, int context, int who )
{
   WriteUserStream * userStream    = WriteUserStream::getInstance();   
   StringLibrary   * stringLibrary = StringLibrary::getInstance();   
   user_struct     * loopUser      = 0;
   user_struct     * lastFoundUser = 0;
   int               usersfound    = 0;
   
   pod_string found_users_str;

   if( name.length() == 0 )
   {
      if( verbosity == GUA_SHOW_ERRORS && user != 0 )
      {
         *userStream << "No name given!\n" << pod_send;
      }
      return 0;
   }
   
   capitalize( name );
   
   for( loopUser  = user_first;
        loopUser != 0;
        loopUser  = loopUser->next )
   {
      bool cloaked = ( who     == GUA_NOT_CLOAKED ) && loopUser->cloaked;      
      bool notHere = ( context == GUA_ROOM )        && ( loopUser->room != user->room );

      if ( loopUser->login || is_clone( loopUser ) || cloaked || notHere )
      {
         continue;
      }
      
      if ( !strncmp( loopUser->name, name.c_str(), name.length() ) )
      {
         if ( name.length() == strlen( loopUser->name ) )
         {
            return loopUser;
         }
         usersfound++;
         lastFoundUser = loopUser;
         if ( usersfound > 1 )
         {
            found_users_str += ", ";
         }
         found_users_str += loopUser->name;
      }
   }

   if( usersfound == 1 )
   {
      return lastFoundUser;
   }
      
   if( verbosity == GUA_SHOW_ERRORS && user != 0 )
   {   
      if( usersfound == 0 )
      {
         if( context == GUA_ROOM )
         {
            *userStream << addUser( user ) << "There is no user in this room matching that name.\n" << pod_send;
         }
         else
         {
            *userStream << addUser( user ) << stringLibrary->makeString("notloggedon") << '\n' << pod_send;
         }
      }
      else
      {
         *userStream << addUser( user ) << "There is more than one match for that name: " << found_users_str << "\n" << pod_send;
      }
   }
   
   return 0;
}

/* Find user with name exactly the same as var. 'name',  else return null --- Crandonkphin 1999 */
UR_OBJECT get_user_exactmatch(char *name)
{
   UR_OBJECT u;

   if(!name[0]) return 0;

   name[0]=toupper(name[0]);
   for(u=user_first;u!=NULL;u=u->next)
   {
      if (u->login || is_clone(u))
      {
         continue;
      }
      if (!strcmp(u->name,name))
      {
         return u;
      }
   }
   return 0;
}

/* Find user with name or part of name the same as var. 'name',  return 0 and show a warning if no good match */
UR_OBJECT get_user_and_check( UR_OBJECT user, pod_string name )
{
   return get_user_advanced( user, name, GUA_SHOW_ERRORS, GUA_TALKER, GUA_ALL );
}

/* Find user with name or part of name the same as var. 'name',  else return 0 */
UR_OBJECT get_user( pod_string name )
{
   return get_user_advanced( 0, name, GUA_SILENT, GUA_TALKER, GUA_ALL );
}

/* things that are the same for new users and initialized
   user structures */
void gen_init_user(UR_OBJECT user)
{
   userVarsController.initObj( user->data_ptr );
   user->autofwd=0;
   user->autoread=0;
   user->boardwrite[0]='\0';
   user->charcnt=0;
   user->clone_hear=CLONE_HEAR_ALL;
   user->command_mode=0;
   user->cursed=0;
   user->curse_name[0]='\0';
   user->edit_op=0;
   user->edit_line=0;
   user->hide_email=0;
   user->hide_url=0;
   user->ignall=0;
   user->ignall_store=0;
   user->inpstr_old[0]='\0';
   user->jailed=LEV_MIN;
   user->last_input=time(0);
   user->last_site[0]='\0';
   user->last_login=time(0);
   user->last_login_len=0;
   user->cloak_store_last_site[0]='\0';
   user->cloak_store_last_login=0;
   user->cloak_store_last_login_len=0;   
   user->level=0;
   user->mail_verified=0;
   user->malloc_start=NULL;
   user->malloc_end=NULL;
   user->misc_op=MISC_NONE;
   user->nextCommand=0;
   user->muzzled=0;
   user->no_delold=0;
   user->help_mode=HELP_INDEX;
   user->owner=NULL;
   user->prompt=prompt_def.get();
   user->tag=0;
   user->total_login=0;
   user->total_active_time=0;
   user->total_idle_time=0;
   user->total_afk_time=0;
   user->total_active_first=0;
   user->idle_status=0;
   user->inedit=0;
}


/* initialize a new unitialized user structure */
/* needs to clean everything, set to empty values */
void init_user(UR_OBJECT user)
{
   gen_init_user(user);

   user->type=USER_TYPE;
   user->socket=NULL;
   user->name[0]='\0';
   user->pass[0]='\0';
   user->filepos=0;
   user->room=NULL;
   user->invite_room=NULL;
   user->idle_before_room=NULL;
   user->follow_partner=NULL;
   user->follow_mode=FOLLOW_MODE_NONE;
   user->login=0;
   user->attempts=0;
   user->remoteEchoOn=0;
   user->cloaked=0;
   user->vis=1;
   user->afk=0;
}

/* generate a brand new user structure, set to first logon defaults
   the struct being cleaned is already initialized but can also already
   contain a loaded user , struct is still is unused tho)  */
void new_init_user(UR_OBJECT user)
{
   gen_init_user(user);

   StrGlobalVar *in_phrase   = (StrGlobalVar*)user->getVar("in_phrase");
   StrGlobalVar *out_phrase  = (StrGlobalVar*)user->getVar("out_phrase");
   StrGlobalVar *desc        = (StrGlobalVar*)user->getVar("desc");
   IntGlobalVar *accreq      = (IntGlobalVar*)user->getVar("accreq");
   StrGlobalVar *prefroom    = (StrGlobalVar*)user->getVar("PrefRoom");
   StrGlobalVar *color       = (StrGlobalVar*)user->getVar("Color");
   IntGlobalVar *color_on    = (IntGlobalVar*)user->getVar("ColorOn");

   in_phrase->set("enters");
   out_phrase->set("goes");
   desc->set("hasn't used .desc yet");
   accreq->set(ACCREQ_NONE);
   prefroom->set( globalSpecialRoomNames.getMainRoomName()->get() );

   color->set( default_color.get() );
   color_on->set( color_on_def.get() );
   user->total_active_first=(int)time(0);
}

UR_OBJECT create_user()
{
   return create_user_adv(TRUE);
}

/*** Construct user/clone object ***/
UR_OBJECT create_user_adv(int link)
{
   UR_OBJECT user;

   user=new user_struct;

   /* dynamic data part */
   userVarsController.createObj( user->data_ptr );

/* Append object into linked list. */
   if(link)
   {
      if (user_first==NULL)
      {
         user_first=user;
         user->prev=NULL;
      }
      else
      {
         user_last->next=user;  user->prev=user_last;
      }
      user->next=NULL;
      user_last=user;
   }

   /* initialise user structure */
   init_user(user);

   return user;
}

/*** Destruct an object. ***/
int destruct_user(UR_OBJECT user)
{
   int linked=FALSE;
   UR_OBJECT u;

   /* check if it's a temp user (not in linked list) or an online user */
   for(u=user_first;u!=NULL;u=u->next)
   {
      if(u==user)
      {
         linked=TRUE;
         break;
      }
   }

   /* Remove from linked list */
   if(linked)
   {
      if (user==curr_user)
      {
         curr_user_destructed=TRUE;
         curr_user=NULL;
      }

      if (user==user_first)
      {
         user_first=user->next;
         if (user==user_last) user_last=NULL;
         else user_first->prev=NULL;
      }
      else
      {
         user->prev->next=user->next;
         if (user==user_last)
         {
            user_last=user->prev;
            user_last->next=NULL;
         }
         else user->next->prev=user->prev;
      }
   }

   userVarsController.deleteObj( user->data_ptr );

   delete user;
   return 0;
}

int purge_user_files(char* u_name)
{
   char filename[80];
   sprintf(filename,"%s/%s.D.xml",USERFILES,u_name);
   unlink_checked(filename,"purge_user_files");
   sprintf(filename,"%s/%s.M",USERFILES,u_name);
   unlink(filename);
   sprintf(filename,"%s/%s.P",USERFILES,u_name);
   unlink(filename);
   sprintf(filename,"%s/%s.A",USERFILES,u_name);
   unlink(filename);
   sprintf(filename,"%s/%s.R",USERFILES,u_name);
   unlink(filename);
   return 0;
}

void user_struct::fromXMLPerm( XmlTextReader *reader )
{
   enum
   {
         UNSET,
         AUTOFWD_ITEM,
         AUTOREAD_ITEM,
         CHARECHO_ITEM,
         CMDMODE_ITEM,
         CURSED_ITEM,
         CURSE_NAME_ITEM,
         HELPMODE_ITEM,
         HIDEEMAIL_ITEM,
         HIDEURL_ITEM,
         JAILED_ITEM,
         LASTLOGIN_ITEM,
         LASTSITE_ITEM,
         LEVEL_ITEM,
         MAILVRFY_ITEM,
         MUZZLED_ITEM,
         NODELOLD_ITEM,
         PASSWORD_ITEM,
         PROMPT_ITEM,
         TOTAL_ACTIVE_TIME_ITEM,
         TOTAL_IDLE_TIME_ITEM,
         TOTAL_AFK_TIME_ITEM,
         TOTAL_ACTIVE_FIRST_ITEM,
         TOTAL_LOGIN_ITEM,
         TAG_ITEM,
         USERTIME_ITEM
   };
      
   int        state          = UNSET;
   bool       hasRead        = true;
   int        type           = 0;
   pod_string name;
   bool       inUserElement  = false;
   pod_string site_name;
   pod_string itemName;
   pod_string itemValue;
   

   while( hasRead )
   {
      type = reader->NodeType();
   
      if( type == XML_READER_TYPE_ELEMENT || type == XML_READER_TYPE_END_ELEMENT )
      {
         name = reader->Name();

         if( type == XML_READER_TYPE_ELEMENT )
         {
            if( name == "user" )
            {
               inUserElement = true;
            }
            else if( inUserElement == true && name == "item" )
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
               
               if( itemName == "AutoFwd" )
               {
                  state = AUTOFWD_ITEM;
               }
               else if( itemName == "AutoRead" )
               {
                  state = AUTOREAD_ITEM;
               }
               else if( itemName == "CharEcho" )
               {
                  state = CHARECHO_ITEM;
               }
               else if( itemName == "CmdMode" )
               {
                  state = CMDMODE_ITEM;
               }
               else if( itemName == "Cursed" )
               {
                  state = CURSED_ITEM;
               }
               else if( itemName == "Curse_name" )
               {
                  state = CURSE_NAME_ITEM;
               }
               else if( itemName == "HelpMode" )
               {
                  state = HELPMODE_ITEM;
               }
               else if( itemName == "HideEmail" )
               {
                  state = HIDEEMAIL_ITEM;
               }
               else if( itemName == "HideUrl" )
               {
                  state = HIDEURL_ITEM;
               }
               else if( itemName == "Jailed" )
               {
                  state = JAILED_ITEM;
               }
               else if( itemName == "LastLogin" )
               {
                  state = LASTLOGIN_ITEM;
               }
               else if( itemName == "LastSite" )
               {
                  state = LASTSITE_ITEM;
               }
               else if( itemName == "Level" )
               {
                  state = LEVEL_ITEM;
               }
               else if( itemName == "MailVrfy" )
               {
                  state = MAILVRFY_ITEM;
               }
               else if( itemName == "Muzzled" )
               {
                  state = MUZZLED_ITEM;
               }
               else if( itemName == "NoDelold" )
               {
                  state = NODELOLD_ITEM;
               }
               else if( itemName == "PassWord" )
               {
                  state = PASSWORD_ITEM;
               }
               else if( itemName == "Prompt" )
               {
                  state = PROMPT_ITEM;
               }
               else if( itemName == "TotalActiveTime" )
               {
                  state = TOTAL_ACTIVE_TIME_ITEM;
               }
               else if( itemName == "TotalIdleTime" )
               {
                  state = TOTAL_IDLE_TIME_ITEM;
               }
               else if( itemName == "TotalAfkTime" )
               {
                  state = TOTAL_AFK_TIME_ITEM;
               }
               else if( itemName == "TotalActiveFirst" )
               {
                  state = TOTAL_ACTIVE_FIRST_ITEM;
               }
               else if( itemName == "TotalLogin" )
               {
                  state = TOTAL_LOGIN_ITEM;
               }
               else if( itemName == "Tag" )
               {
                  state = TAG_ITEM;
               }
               else if( itemName == "UserTime" )
               {
                  state = USERTIME_ITEM;
               }
               else
               {
                  userVarsController.fromXML( reader, data_ptr );
               }
            }
         }
         else if( type == XML_READER_TYPE_END_ELEMENT )
         {
            if( name == "user" && inUserElement == true )
            {
               inUserElement = false;
               return;
            }
            else if( name == "item" && inUserElement == true)
            {
               state = UNSET;
            }
         }
      }
      else if( type == XML_READER_TYPE_TEXT )
      {
         itemValue = reader->Value();

         if( state == AUTOFWD_ITEM )
         {
            autofwd = stringToInt( itemValue );
         }
         else if( state == AUTOREAD_ITEM )
         {
            autoread = stringToInt( itemValue );
         }
         else if( state == CMDMODE_ITEM )
         {
            command_mode = stringToInt( itemValue );
         }
         else if( state == CURSED_ITEM )
         {
            cursed = stringToInt( itemValue );
         }
         else if( state == CURSE_NAME_ITEM )
         {
            if( itemValue.length() <= 79 )
            {
               strcpy( curse_name, itemValue.c_str() );
            }
         }
         else if( state == HELPMODE_ITEM )
         {
            help_mode = stringToInt( itemValue );
         }
         else if( state == HIDEEMAIL_ITEM )
         {
            hide_email = stringToInt( itemValue );
         }
         else if( state == HIDEURL_ITEM )
         {
            hide_url = stringToInt( itemValue );
         }
         else if( state == JAILED_ITEM )
         {
            jailed = stringToInt( itemValue );
         }
         else if( state == LASTLOGIN_ITEM )
         {
            last_login_len = (time_t)stringToInt( itemValue );
         }
         else if( state == LASTSITE_ITEM )
         {
            if( itemValue.length() <= MAX_SITE_LEN )
            {
               strcpy( last_site, itemValue.c_str() );
            }
         }
         else if( state == LEVEL_ITEM )
         {
            level = stringToInt( itemValue );
         }
         else if( state == MAILVRFY_ITEM )
         {
            mail_verified = stringToInt( itemValue );
         }
         else if( state == MUZZLED_ITEM )
         {
            muzzled = stringToInt( itemValue );
         }
         else if( state == NODELOLD_ITEM )
         {
            no_delold = stringToInt( itemValue );
         }
         else if( state == PASSWORD_ITEM )
         {
            if( itemValue.length() <= PASS_LEN + 5 )
            {
               strcpy( pass, itemValue.c_str() );
            }
         }
         else if( state == PROMPT_ITEM )
         {
            prompt = stringToInt( itemValue );
         }
         else if( state == TOTAL_ACTIVE_TIME_ITEM )
         {
            total_active_time = (time_t)stringToInt( itemValue );
         }
         else if( state == TOTAL_IDLE_TIME_ITEM )
         {
            total_idle_time = (time_t)stringToInt( itemValue );
         }
         else if( state == TOTAL_AFK_TIME_ITEM )
         {
            total_afk_time = (time_t)stringToInt( itemValue );
         }
         else if( state == TOTAL_ACTIVE_FIRST_ITEM )
         {
            total_active_first = (time_t)stringToInt( itemValue );
         }
         else if( state == TOTAL_LOGIN_ITEM )
         {
            total_login = (time_t)stringToInt( itemValue );
         }
         else if( state == TAG_ITEM )
         {
            tag = stringToInt( itemValue );
         }
         else if( state == USERTIME_ITEM )
         {
            last_login = (time_t)stringToInt( itemValue );
         }
      }

      hasRead = reader->Read();
   }

}

void user_struct::fromXMLSoft( XmlTextReader *reader )
{
   enum
   {
         UNSET,
         SITE_ITEM,
         SOCKET_ITEM,
         INPSTR_OLD_ITEM,
         NEXTCOMMAND_ITEM,
         BOARDWRITE_ITEM,
         ROOMNAME_ITEM,
         INVITE_ROOMNAME_ITEM,
         IDLE_BEFOREROOM_ITEM,
         FOLLOW_MODE_ITEM,
         FOLLOW_PARTNER_ITEM,
         MISC_OP_ITEM,
         FILEPOS_ITEM,
         AFK_ITEM,
         VIS_ITEM,
         IGNALL_ITEM,
         EDIT_LINE_ITEM,
         CHARCNT_ITEM,
         IDLE_STATUS_ITEM,
         IGNALL_STORE_ITEM,
         CLONE_HEAR_ITEM,
         EDIT_OP_ITEM,
         INEDIT_ITEM,
         CLOAKED_ITEM,
         TYPE_ITEM,
         LAST_INPUT_ITEM,
         USER_MALLOC_ITEM,
         CLOAK_STORE_LAST_LOGIN_ITEM,
         CLOAK_STORE_LAST_LOGIN_LEN_ITEM,
         CLOAK_STORE_LAST_SITE_ITEM
   };
   int        state          = UNSET;
   bool       hasRead        = true;
   int        type           = 0;
   pod_string name;
   bool       inUserElement  = false;
   pod_string site_name;
   pod_string itemName;
   pod_string itemValue;
   

   while( hasRead )
   {
      type = reader->NodeType();
   
      if( type == XML_READER_TYPE_ELEMENT || type == XML_READER_TYPE_END_ELEMENT )
      {
         name = reader->Name();

         if( type == XML_READER_TYPE_ELEMENT )
         {
            if( name == "user" )
            {
               inUserElement = true;
            }
            else if( inUserElement == true && name == "item" )
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

               if( itemName == "site" )
               {
                  state = SITE_ITEM;
               }
               else if( itemName == "socket" )
               {
                  state = SOCKET_ITEM;
               }
               else if( itemName == "qMethod" )
               {
                  socket->fromXML( reader );
               }               
               else if( itemName == "inpstr_old" )
               {
                  state = INPSTR_OLD_ITEM;
               }
               else if( itemName == "nextCommand" )
               {
                  state = NEXTCOMMAND_ITEM;
               }
               else if( itemName == "boardwrite" )
               {
                  state = BOARDWRITE_ITEM;
               }
               else if( itemName == "roomname" )
               {
                  state = ROOMNAME_ITEM;
               }
               else if( itemName == "invite_roomname" )
               {
                  state = INVITE_ROOMNAME_ITEM;
               }
               else if( itemName == "idle_before_room" )
               {
                  state = IDLE_BEFOREROOM_ITEM;
               }
               else if( itemName == "follow_mode" )
               {
                  state = FOLLOW_MODE_ITEM;
               }
               else if( itemName == "follow_partner" )
               {
                  state = FOLLOW_PARTNER_ITEM;
               }
               else if( itemName == "misc_op" )
               {
                  state = MISC_OP_ITEM;
               }
               else if( itemName == "filepos" )
               {
                  state = FILEPOS_ITEM;
               }
               else if( itemName == "afk" )
               {
                  state = AFK_ITEM;
               }
               else if( itemName == "vis" )
               {
                  state = VIS_ITEM;
               }
               else if( itemName == "ignall" )
               {
                  state = IGNALL_ITEM;
               }
               else if( itemName == "edit_line" )
               {
                  state = EDIT_LINE_ITEM;
               }
               else if( itemName == "charcnt" )
               {
                  state = CHARCNT_ITEM;
               }
               else if( itemName == "idle_status" )
               {
                  state = IDLE_STATUS_ITEM;
               }
               else if( itemName == "ignall_store" )
               {
                  state = IGNALL_STORE_ITEM;
               }
               else if( itemName == "clone_hear" )
               {
                  state = CLONE_HEAR_ITEM;
               }
               else if( itemName == "edit_op" )
               {
                  state = EDIT_OP_ITEM;
               }
               else if( itemName == "inedit" )
               {
                  state = INEDIT_ITEM;
               }
               else if( itemName == "cloaked" )
               {
                  state = CLOAKED_ITEM;
               }
               else if( itemName == "type" )
               {
                  state = TYPE_ITEM;
               }
               else if( itemName == "last_input" )
               {
                  state = LAST_INPUT_ITEM;
               }
               else if( itemName == "user_malloc" )
               {
                  state = USER_MALLOC_ITEM;
               }
               else if( itemName == "CloakStoreUserTime" )
               {
                  state = CLOAK_STORE_LAST_LOGIN_ITEM;
               }
               else if( itemName == "CloakStoreLastLogin" )
               {
                  state = CLOAK_STORE_LAST_LOGIN_LEN_ITEM;
               }
               else if( itemName == "CloakStoreLastSite" )
               {
                  state = CLOAK_STORE_LAST_SITE_ITEM;
               }
               else
               {
                  userVarsController.fromXML( reader, data_ptr );
               }
            }
         }
         else if( type == XML_READER_TYPE_END_ELEMENT )
         {
            if( name == "user" && inUserElement == true )
            {
               inUserElement = false;
               return;
            }
            else if( name == "item" && inUserElement == true)
            {
               state = UNSET;
            }
         }
      }
      else if( type == XML_READER_TYPE_TEXT )
      {
         itemValue = reader->Value();

         if( state == SITE_ITEM )
         {
            std::cout << "site item chars : " << itemValue << std::endl;
            site_name = itemValue;
         }
         else if( state == SOCKET_ITEM )
         {
            std::cout << "socket item chars : " << itemValue << std::endl;
            int old_socket_fd;

            old_socket_fd = stringToInt( itemValue );
            
            UserTelnetHandler *telnetHandler = new UserTelnetHandler();
            telnetHandler->setUserPtr( this );

            socket = new TelnetSocket( old_socket_fd, site_name, telnetHandler );
   
            telnetHandler->setSocket( socket );

            if( socket == 0 )
            {
               write_syslogf("*** BOOT failed softboot, recreating user socket failed\n",FALSE);
               return;
            }
            socketInterface.addPlainSocket( socket );
            socket->flag_as_close_on_execv( true );
         }
         else if(state == INPSTR_OLD_ITEM)
         {
            if( itemValue.length() <= INPSTR_OLD_LEN )
            {
               strcpy( inpstr_old, itemValue.c_str() );
            }
         }
         else if(state == NEXTCOMMAND_ITEM)
         {
            nextCommand = get_cmd( itemValue );
         }
         else if(state == BOARDWRITE_ITEM)
         {
            if( itemValue.length() <= 29 )
            {
               strcpy( boardwrite, itemValue.c_str() );
            }
         }
         else if(state == ROOMNAME_ITEM)
         {
            if( !itemValue.empty() )
            {
               room = get_room( itemValue.c_str() );
            }
            else
            {
               room = 0;
            }
         }
         else if(state == INVITE_ROOMNAME_ITEM)
         {
            if( !itemValue.empty() )
            {
               invite_room = get_room( itemValue.c_str() );
            }
            else
            {
               invite_room = 0;
            }
         }
         else if(state == IDLE_BEFOREROOM_ITEM)
         {
            if( !itemValue.empty() )
            {
               idle_before_room = get_room( itemValue.c_str() );
            }
            else
            {
               idle_before_room = 0;
            }
         }
         else if(state == FOLLOW_MODE_ITEM)
         {
            follow_mode = stringToInt( itemValue );
         }
         else if(state == FOLLOW_PARTNER_ITEM)
         {
            set_user_link (&follow_partner,itemValue );
         }
         else if(state == MISC_OP_ITEM)
         {
            misc_op = stringToInt( itemValue );
         }
         else if(state == FILEPOS_ITEM)
         {
            filepos = stringToInt( itemValue );
         }
         else if(state == AFK_ITEM)
         {
            afk = stringToInt( itemValue );
         }
         else if(state == VIS_ITEM)
         {
            vis = stringToInt( itemValue );
         }
         else if(state == IGNALL_ITEM)
         {
            ignall = stringToInt( itemValue );
         }
         else if(state == EDIT_LINE_ITEM)
         {
            edit_line = stringToInt( itemValue );
         }
         else if(state == CHARCNT_ITEM)
         {
            charcnt = stringToInt( itemValue );
         }
         else if(state == IDLE_STATUS_ITEM)
         {
            idle_status = stringToInt( itemValue );
         }
         else if(state == IGNALL_STORE_ITEM)
         {
            ignall_store = stringToInt( itemValue );
         }
         else if(state == CLONE_HEAR_ITEM)
         {
            clone_hear = stringToInt( itemValue );
         }
         else if(state == EDIT_OP_ITEM)
         {
            edit_op = stringToInt( itemValue );
         }
         else if(state == INEDIT_ITEM)
         {
            inedit = stringToInt( itemValue );
         }
         else if(state == CLOAKED_ITEM)
         {
            cloaked = stringToInt( itemValue );
         }
         else if(state == TYPE_ITEM)
         {
            type = stringToInt( itemValue );
         }
         else if(state == LAST_INPUT_ITEM)
         {
            last_input = stringToInt( itemValue );
            #warning time_t, not int!
         }
         else if(state == CLOAK_STORE_LAST_LOGIN_ITEM)
         {
            cloak_store_last_login = stringToInt( itemValue );
         }
         else if(state == CLOAK_STORE_LAST_LOGIN_LEN_ITEM)
         {
            cloak_store_last_login_len = stringToInt( itemValue );
         }
         else if(state == CLOAK_STORE_LAST_SITE_ITEM)
         {
            strcpy( cloak_store_last_site, itemValue.c_str() );
         }
         else if(state == USER_MALLOC_ITEM)
         {
            if(malloc_start == 0)
            {
               malloc_start = (char *)xalloc(MAX_LINES*81,"load_malloc");
               if( malloc_start == 0 )
               {
                  write_syslog("ERROR: Failed to allocate memory in load_malloc_node().\n",0);
                  return;
               }
               malloc_end = malloc_start;
            }
            memcpy( malloc_end, itemValue.data(), itemValue.length() );
            malloc_end += itemValue.length();
         }
      }

      hasRead = reader->Read();
   }
}

bool user_struct::load( )
{
   XmlTextReader * reader = 0;
   pod_string filename = USERFILES;
   filename += "/";
   filename += name;
   filename += ".D.xml";   
   
   try
   {
      reader = new XmlTextReader( filename );   
   }
   catch (XmlError &e)
   {
      std::cerr << "Unable to open " << filename << std::endl;
      return false;
   }

   fromXMLPerm( reader );
   
   delete reader;   
      
   return true;   
}

void user_struct::save( )
{
   pod_string fileName = USERFILES;
   fileName += "/";
   fileName += name;
   fileName += ".D.xml";

   if( type==CLONE_TYPE)
   {
      return;
   }

   xmlTextWriterPtr ptr = xmlNewTextWriterFilename( fileName.c_str(),  0);
   xmlTextWriterStartDocument( ptr, NULL, NULL, NULL );
   toXMLPerm(ptr);
   xmlTextWriterEndDocument( ptr );
   xmlFreeTextWriter( ptr );    

   return;   
}

