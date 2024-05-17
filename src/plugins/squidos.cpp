#include "../levels.h"
#include "../socket_funcs.h"
#include "../GlobalVars.h"
#include "../Room.h"
#include "../user_objects.h"
#include "../speech_funcs.h"
#include "../wordfind.h"
#include "../help.h"
#include "../cmd_main.h"
#include "../podnuts.h"
#include "../pod_string.h"
#include "../StringLibrary.h"
#include "../gender.h"
#include "../string_misc.h"


int give (UR_OBJECT user,char *inpstr);
int eat  (UR_OBJECT user,char *inpstr);

extern "C" void plugin_init()
{
   StringLibrary::getInstance()->addFile("datafiles/squido.xml");

   user_var_add_cust("baggies",   new IntObjectCreator(0), USR_SAVE_ALWS);
   user_var_add_cust("squidos",   new IntObjectCreator(0), USR_SAVE_ALWS);

   cmd_add("give", LEV_THR, BOA, &give);
   cmd_add("eat",  LEV_ONE, BOA, &eat);
}

int give(UR_OBJECT user,char *inpstr)
{
   struct words_struct* words_ptr = &words;
   WriteUserStream *userStream = WriteUserStream::getInstance();
   WriteRoomStream *roomStream = WriteRoomStream::getInstance();
   StringLibrary * stringLibrary = StringLibrary::getInstance();

   if( words_ptr->word_count < 2 )
   {
      *userStream << addUser( user ) << "Give who, how many ?\n" << pod_send;
   }
   else if ( words_ptr->word_count < 3 )
   {
      *userStream << addUser( user ) << "Give how many ?\n" << pod_send;
   }
   else
   {
      UR_OBJECT u;
      
      if ( !( u = get_user(words_ptr->word[1])) )
      {
         *userStream << addUser( user ) << stringLibrary->makeString("notloggedon") << '\n' << pod_send;
      }
      else if (u==user)
      {
         *userStream << addUser( user ) << stringLibrary->makeString("give_self") << '\n' << pod_send;
      }
      else if (!(user->room==u->room))
      {
         *userStream << addUser( user ) << stringLibrary->makeString("give_not_in_room") << '\n' << pod_send;
      }
      else
      {
         IntGlobalVar *baggies   = (IntGlobalVar*)u->getVar("baggies");
         int val=0;
         char* first_invalid = (char*)(10); /*not NULL , can be anything else)*/

         val = strtol(words_ptr->word[2],&first_invalid,10);

         if(words_ptr->word[2] == first_invalid)
         {
            *userStream << addUser( user ) << "Please enter a nummeric amount to give.\n" << pod_send;
            return 0;
         }
         if( *first_invalid )
         {
            *userStream << addUser( user )
                       << "Eek!, no strange char's after the number :). ( " << words_ptr->word[2] << " )\n"
                       << pod_send;
            return 0;
         }
         if( val > 100 || val < 1)
         {
            *userStream << addUser( user ) << "Please enter an amount between 1 and 100.\n" << pod_send;
            return 0;
         }

         int oldAmount = baggies->get();

         pod_ostringstream count;
         count << val;
   
         stringLibrary->addTextTag( "name", user->name );
         stringLibrary->addTextTag( "target", u->name );
         stringLibrary->addTextTag( "count", count.str() );
                              
         *userStream << addUser( user ) << stringLibrary->makeString( "give" ) << '\n' << pod_send;

         *userStream << addUser( u ) << stringLibrary->makeString( "get" ) << '\n' << pod_send;
         
         *roomStream << setRoom( user->room ) << addExcept( user ) << addExcept( u )
                     << stringLibrary->makeString( "give_room" ) << '\n' << pod_send;

         baggies->set( oldAmount + val );         
      }
   }
   return 0;
}

int eat(UR_OBJECT user,char *inpstr)
{
   IntGlobalVar *baggies   = (IntGlobalVar*)user->getVar("baggies");
   IntGlobalVar *squidos   = (IntGlobalVar*)user->getVar("squidos");
   struct words_struct* words_ptr = &words;
   WriteUserStream *userStream = WriteUserStream::getInstance();
   WriteRoomStream *roomStream = WriteRoomStream::getInstance();
   StringLibrary * stringLibrary = StringLibrary::getInstance();

   if( words_ptr->word_count > 1 )
   {
      *userStream << addUser( user ) << "This command takes no arguments\n" << pod_send;
   }
   
   stringLibrary->addTextTag( "name", user->name );
   stringLibrary->addTextTag( "his", get_gender( user, "his") );
   
   int oldAmountBaggies = baggies->get();
   int oldAmountSquidos = squidos->get();   
   if( oldAmountBaggies == 0 && oldAmountSquidos == 0 )
   {
      *userStream << addUser( user ) << stringLibrary->makeString("out_of") << '\n' << pod_send;
   }
   else if( oldAmountSquidos == 0 )
   {
      *userStream << addUser( user )
                 << stringLibrary->makeString("pop_me") << '\n' << pod_send;

      *roomStream  << setRoom( user->room ) << addExcept( user )
                   << stringLibrary->makeString("pop_room") << '\n' << pod_send;

      baggies->set( oldAmountBaggies - 1 );
      squidos->set( 2 + random()%2 );
   }
   else if( oldAmountSquidos == 1 )
   {
      *userStream << addUser( user )
                 << stringLibrary->makeString("last_me") << '\n' << pod_send;

      *roomStream  << setRoom( user->room ) << addExcept( user )
                   << stringLibrary->makeString("last_room") << '\n' << pod_send;

      squidos->set( oldAmountSquidos - 1 );
   }
   else
   {
      *userStream << addUser( user )
                 << stringLibrary->makeString("eat_me") << '\n' << pod_send;

      *roomStream  << setRoom( user->room ) << addExcept( user )
                   << stringLibrary->makeString("eat_room") << '\n' << pod_send;

      squidos->set( oldAmountSquidos - 1 );
   }

   return 0;
}
