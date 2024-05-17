#include "general_headers.h"
#include "move.h"
#include "loadsave_user.h"
#include "admin.h"
#include "globals.h"
#include "cmd_main.h"
#include "help.h"
#include "string_misc.h"
#include "account_reg.h"

int account_accept(UR_OBJECT user, char *inpstr);
int account_request(UR_OBJECT user, char *inpstr);
int account_reject(UR_OBJECT user, char *inpstr);

void account_reg_init()
{
   user_var_add_cust( "accreq", new IntObjectCreator(ACCREQ_GIVEN), USR_SAVE_ALWS);

   cmd_add("accreq",   LEV_ZER, GEN, &account_request);
   cmd_add("resident", LEV_THR, ADM, &account_accept );
   cmd_add("reject",   LEV_THR, ADM, &account_reject );
}

/*** A newbie is requesting an account. Get his email address off him so we
     can validate who he is before we promote him and let him loose as a
     proper user. ***/
int account_request(UR_OBJECT user, char *inpstr)
{
   IntGlobalVar *accreq   = (IntGlobalVar*)user->getVar("accreq");
   StrGlobalVar *color    = (StrGlobalVar*)user->getVar("Color");

   if (user->level>LEV_ZER)                  write_user(user,"This command is for new users only, you already have a full account.\n");
   else if (accreq->get() != ACCREQ_NONE) write_user(user,"You have already requested an account.\n");
   else if (words.word_count<2)                write_user(user,"Usage: accreq <an email address we can contact you on + any relevent info>\n");
   else
   {
      logStream << setLogfile( ACCOUNTLOG ) << "ACCOUNT REQUEST from " << user->name <<  ": " << inpstr << ".\n" << pod_send;

      pod_stringstream outputStream;
      outputStream << "user requested an account, info: " << inpstr << "\n";
      log_user(user, "System", (char*)outputStream.str().c_str() );


      write_levelf(LEV_FOU,1,NULL,"~OLSYSTEM:~RS %s has made a request for an account.\n",user->name);
      write_user(user,"Account request logged.\n");

      accreq->set(ACCREQ_ASKED);

      outputStream.str("");
      outputStream << "~OL~FW[wsys  ]~RS " << color->get() << user->name << " has requested residency on POD, with e-mail : " << inpstr << ".\n";
      write_level(LEV_THR,1,(char*)outputStream.str().c_str(), NULL );
      wrecord( (char*)outputStream.str().c_str() );
   }

   return 0;
}

int account_reject(UR_OBJECT user, char *inpstr)
{
   UR_OBJECT u;
   IntGlobalVar *accreq;

   if (words.word_count<3) write_user(user,"Usage: reject <user> <reason>\n");   
   else if ((u=get_user(words.word[1]))!=NULL) 
   {
      accreq   = (IntGlobalVar*)u->getVar("accreq");

      if(accreq->get() == ACCREQ_NONE)       write_userf(user,"~FG~OLUser %ss has not yet requested residency.\n",u->name);
      else if(accreq->get() == ACCREQ_GIVEN) write_userf(user,"~FG~OLUser %ss has already been granted residency.\n",u->name);
      else
      {
         accreq->set( ACCREQ_NONE );
         write_userf(u,"%s %s has rejected your request for residency because: %s.\n",getLevelName(user->level),user->name,remove_first(inpstr));
         write_userf(user,"~FG~OLYou have rejected residency to %s.\n",u->name);
         logStream << setLogfile( ACCOUNTLOG ) << "~OL~FR" << get_visible_name(user) << " has rejected " << u->name << "'s request for residency.\n" << pod_send;
         pod_stringstream outputStream;
         outputStream << "user account request got rejected, reason: " << remove_first(inpstr) << "\n";
         log_user(u,user->name, (char*)outputStream.str().c_str() );
      }
   }
   return 0;
}

int account_accept(UR_OBJECT user, char *inpstr)
{
   UR_OBJECT targetUser;
   RM_OBJECT room;
   pod_string name;
   IntGlobalVar *accreq;

   WriteRoomStream *roomStream = WriteRoomStream::getInstance();

   if (words.word_count<2)
   {
      write_user(user,"Usage: resident <user>\n");
      return 0;
   }

   targetUser = get_user(words.word[1]);

   if ( targetUser != 0 )
   {
      accreq   = (IntGlobalVar*)targetUser->getVar("accreq");

      if( accreq->get() == ACCREQ_NONE )
      {
         write_userf(user,"~FG~OLUser %ss has not yet requested residency.\n", targetUser->name);
      }
      else if( accreq->get() == ACCREQ_GIVEN )
      {
         write_userf(user,"~FG~OLUser %ss has already been granted residency.\n", targetUser->name);
      }
      else
      {
         targetUser->level++;
         accreq->set( ACCREQ_GIVEN );
         name = get_visible_name(user);
         write_userf( targetUser, "%s has approved your residency.\n", name.c_str() );
         write_userf(user,"~FG~OLYou have given residency to %s.\n",targetUser->name);


         if( globalSpecialRoomNames.getEntryRoomName()->get() == targetUser->room->name )
         {
            room = get_room( globalSpecialRoomNames.getMainRoomName()->get().c_str() );

            write_userf( targetUser, "\n~FT~OLA giant flipper grabs you and pulls you into a magical blue vortex!\n" );

            *roomStream  << setRoom( targetUser->room ) << addExcept( targetUser )
                         << "~FT~OLA giant flipper grabs " << get_visible_name( targetUser ) << " who is pulled into a magical blue vortex!\n"
                         << pod_send;

            *roomStream  << setRoom( room )
                         << "With a crackle of thunder, a new call sig is announced and " << get_visible_name( targetUser ) << " swims into the cove for the first time.\n"
                         << pod_send;

            move_user( targetUser, room, GO_MODE_SILENT );
         }
      }
      return 0;
   }

   targetUser = temp_user_spawn( user, words.word[1], "account_acc" );

   if( targetUser != 0 )
   {
      accreq   = (IntGlobalVar*)targetUser->getVar("accreq");

      if(accreq->get() == ACCREQ_NONE)
      {
         write_userf(user,"~FG~OLUser %s has not yet requested residency.\n",targetUser->name);
      }
      else if(accreq->get() == ACCREQ_GIVEN)
      {
         write_userf(user,"~FG~OLUser %s has already been granted residency.\n",targetUser->name);
      }
      else
      {
         targetUser->level++;
         accreq->set( ACCREQ_GIVEN );
         write_userf(user,"~FG~OLYou have given residency to %s.\n",targetUser->name);

         logStream << setLogfile( PROMOLOG ) << "~OL~FY" << name << " PROMOTED " << targetUser->name << " to rank: ~FW" << getLevelName(targetUser->level) << ".\n" << pod_send;

         save_user(targetUser);
      }
      temp_user_destroy(targetUser);
   }

   return 0;
}

