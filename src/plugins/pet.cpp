#include <iostream>
#include "../levels.h"
#include "../socket_funcs.h"
#include "../GlobalVars.h"
#include "../Room.h"
#include "../user_objects.h"
#include "../speech_funcs.h"
#include "../StringLibrary.h"
#include "../help.h"
#include "../wordfind.h"
#include "../cmd_main.h"
#include "../podnuts.h"
#include "../pod_string.h"
#include "../gender.h"
#include "../string_misc.h"


//      writeus(user," ~CW- ~FGYour very own pet ~CR%s ~FGnamed~CB: [~FT%s~CB]\n",pet[user->pet.type],user->pet.name[0]?user->pet.name:pet[user->pet.type]);

char* pet[] = { "starfish", "otter", "doggie", "glowfish", "crab", "catfish" };

const int Pets = 6;
const int MaxPet = 10;                 

int pet_stuff(UR_OBJECT user,char *inpstr);
int give_pet(UR_OBJECT user,char *inpstr);

extern "C" void plugin_init()
{
   user_var_add_cust("pet_type",    new IntObjectCreator( 0 ),             USR_SAVE_ALWS);
   user_var_add_cust("pet_name",    new StrObjectCreator( 13, "" ),        USR_SAVE_ALWS);
   user_var_add_cust("pet_alias",   new StrArrObjectCreator( MaxPet, "" ), USR_SAVE_ALWS);
   user_var_add_cust("pet_trigger", new StrArrObjectCreator( MaxPet, "" ), USR_SAVE_ALWS);   

   cmd_add("pet", LEV_ONE, BOA, &pet_stuff);
   cmd_add("give_pet", LEV_THR, BOA, &give_pet);
}

pod_string intToString(int i) // convert int to string
{
   pod_ostringstream s;
   s << i;
   return s.str();
}

pod_string StringConvert( pod_string trigger, pod_string str )
{
   pod_string output;
   pod_string current_var;
   pod_string::size_type input_pos  = 0;
   int var_cnt;

   if( trigger.find( '$' ) == pod_string::npos )
   {
      return( trigger );
   }

   words_struct *inputWords = wordfind( str.c_str() );
   if (inputWords->word_count == 0)
   {
      return( trigger );
   }

   while ( input_pos < trigger.size() )
   {
      for ( var_cnt = 0 ; var_cnt < inputWords->word_count ; var_cnt++ )
      {
         current_var = "$";
         current_var += intToString( var_cnt + 1 );

         if( input_pos <= trigger.size() - current_var.size() )
         {
            if ( !trigger.compare( input_pos, current_var.size(), current_var ) )
            {
               output += inputWords->word[ var_cnt ];
               input_pos += current_var.size();

               break;
            }
         }
      }
      if( var_cnt == inputWords->word_count )
      {
         output += trigger[input_pos];
         input_pos++;
      }
   }

   return output;
}

int pet_stuff( UR_OBJECT user, char *inpstr )
{
   struct words_struct* words_ptr = &words;
   WriteUserStream *userStream = WriteUserStream::getInstance();
   WriteRoomStream *roomStream = WriteRoomStream::getInstance();
   pod_string name = get_visible_name(user);
   IntGlobalVar  *pet_type    = (IntGlobalVar*)user->getVar("pet_type");
   StrGlobalVar  *pet_name    = (StrGlobalVar*)user->getVar("pet_name");
   strArrGlobVar *pet_alias   = (strArrGlobVar*)user->getVar("pet_alias");
   strArrGlobVar *pet_trigger = (strArrGlobVar*)user->getVar("pet_trigger");

   if ( user->level == LEV_MIN )
   {
      *userStream << addUser( user )
                 << "~OLYou are in ~FR" << globalSpecialRoomNames.getJailRoomName()->get()
                 << "~FW....you can have no pets in here.\n" << pod_send;
   }
   else if ( pet_type->get() == 0 )
   {
      *userStream << addUser( user )
                 << "You don't have a pet.\n" << pod_send;
   }
   else if( words_ptr->word_count < 2 )
   {
      *userStream << addUser( user )
                 << "See ~FW~OL'~RS.help pet~FW~OL'~RS for info on this command.\n" << pod_send;
   }
   else if ( !strxcmp(words_ptr->word[1],"-name") )
   {
      if ( words_ptr->word_count < 3 )
      {
         *userStream << addUser( user )
                    << "Name your pet to what?\n" << pod_send;
         return 0;
      }
      if ( color_com_strip( words_ptr->word[2] ).size() > 12 )
      {
         *userStream << addUser( user )
                    << "Sorry, that pet name is too long.\n" << pod_send;
         return 0;
      }
      pet_name->set( color_com_strip( words_ptr->word[2] ) );

      *userStream << addUser( user )
                 << "You name your pet " << pet_name->get() << ".\n" << pod_send;

      *roomStream  << setRoom( user->room ) << addExcept( user )
                  << name << " names " << get_gender(user,"his")
                  << " pet " << pet_name->get() << ".\n" << pod_send;
   }
   else if ( !strxcmp(words_ptr->word[1],"-list") )
   {
      *userStream << addUser( user )
                 << gen_seperator_line(" pet triggers ") << "\n" << pod_send;
                 
      bool emptyList = true;
      for (int cnt = 0; cnt < MaxPet; cnt++)
      {
         if ( pet_alias->get(cnt).empty() )
         {
            continue;
         }
         *userStream << addUser( user )
                    << pet_alias->get(cnt) << " = " << pet_trigger->get(cnt) << "\n" << pod_send;
         emptyList = false;
      }
      if( emptyList )
      {
         *userStream << addUser( user )
                    << "You don't have any pet triggers set.\n" << pod_send;
      }

      *userStream << addUser( user )
                 << gen_seperator_line("") << "\n" << pod_send;
   }
   else if ( !strxcmp(words_ptr->word[1],"-trade") )
   {
      if ( words_ptr->word_count < 3 )
      {
         for(int cnt = 0; cnt < Pets; cnt++ )
         {
            *userStream << addUser( user )
                       << "Pet #" << cnt+1 << " =  " << pet[cnt] << "\n" << pod_send;
         }
         *userStream << addUser( user )
                    << "You've gotta specify what your trading it for.\n"
                    << "To trade, type '.pet #', replacing # with your choice.\n" << pod_send;
         return 0;
      }
      int pick = atoi( words_ptr->word[2] );
      if ( pick < 1 || pick > Pets )
      {
         *userStream << addUser( user )
                    << "There are only " << Pets << " different types of pets.. read the helpfile.\n" << pod_send;
         return 0;
      }

      pet_alias->init();
      pet_trigger->init();
      pet_name->set("");
      pet_type->set( pick );
      
      *userStream << addUser( user )
                 << "You trade your pet in for a " << pet[ pet_type->get() - 1 ] << ".\n" << pod_send;
   }
   else if ( !strxcmp(words_ptr->word[1],"-train") )
   {
      pod_string alias;
      pod_string trigger;
      if ( words_ptr->word_count < 5 || strxcmp( words_ptr->word[3], "=" ) )
      {
         *userStream << addUser( user )
                    << "See ~OL'~RS.help pet~OL'~RS for info on this command.\n" << pod_send;
         return 0;
      }
      
      alias = color_com_strip( words_ptr->word[2] );
      if ( alias.size() > 15 )
      {
         *userStream << addUser( user )
                    << "That alias name would be too long.\n" << pod_send;
         return 0;
      }

      inpstr = remove_first(inpstr);
      inpstr = remove_first(inpstr);
      trigger = remove_first(inpstr);      
      if ( trigger.size() > 75 )
      {
         *userStream << addUser( user )
                    << "That trigger would be too long.\n" << pod_send;
         return 0;
      }
      
      for (int cnt = 0; cnt < MaxPet; cnt++)
      {
         if ( alias == pet_alias->get(cnt) )
         {
            *userStream << addUser( user )
                       << "You already have an alias for your pet with that name.\n" << pod_send;
            return 0;
         }
      }

      for (int cnt = 0; cnt < MaxPet; cnt++)
      {
         if( pet_alias->get(cnt).empty() )
         {
            *userStream << addUser( user )
                       << "When you use .pet " << alias << " it will execute:\n"
                       << name << " whispers '" << alias << "' in " << get_gender(user,"his") << " pet " << pet[pet_type->get() - 1] << "'s ear...\n"
                       << ( pet_name->get().empty() ? pet[pet_type->get() - 1] : pet_name->get() ) << " " << trigger << "\n"
                       << pod_send;
                       
            pet_alias->set(cnt, alias );
            pet_trigger->set(cnt, trigger );
            return 0;
         }
      }
      *userStream << addUser( user )
                 << "No empty slot left for the alias.\n" << pod_send;      
   }
   else if ( !strxcmp(words_ptr->word[1],"-untrain") )
   {
      if ( words_ptr->word_count < 3 )
      {
         *userStream << addUser( user )
                    << "You have to specify which alias you want to untrain your pet for.\n" << pod_send;
         return 0;
      }
      for (int cnt = 0; cnt < MaxPet; cnt++)
      {
         if ( pet_alias->get(cnt) == words_ptr->word[2] )
         {
            pet_alias->set(cnt,"");
            pet_trigger->set(cnt,"");
            *userStream << addUser( user )
                       << "Your pet has been untrained for alias '" << words_ptr->word[2] << "'.\n" << pod_send;
            return 0;
         }
      }
      *userStream << addUser( user )
                 << "I can't untrain something your pet hasn't been trained for.\n" << pod_send;
   }
   else
   {
      for (int cnt = 0; cnt < MaxPet; cnt++)
      {
         if ( pet_alias->get(cnt) == words_ptr->word[1] )
         {
            inpstr = remove_first(inpstr);
            
            *userStream << addUser( user )
                       << "You tell your pet '" << pet_alias->get(cnt) << "'.\n" << pod_send;

            *roomStream << setRoom( user->room ) << addExcept( user )
                       << name
                       << " whispers '" << pet_alias->get(cnt)
                       << "' in " << get_gender(user,"his")
                       << " pet " << pet[pet_type->get() - 1]
                       << " ear...\n" << pod_send;

            *roomStream << setRoom( user->room )
                       << ( pet_name->get().empty() ? pet[pet_type->get() - 1] : pet_name->get() )
                       << " " << StringConvert( pet_trigger->get(cnt), inpstr )
                       << "\n" << pod_send;
            return 0;
         }
      }
      *userStream << addUser( user )
                 << "You specified an invalid alias.\n" << pod_send;
   }
   return 0;
}

int give_pet(UR_OBJECT user,char *inpstr)
{
   struct words_struct* words_ptr = &words;
   WriteUserStream *userStream = WriteUserStream::getInstance();
   WriteRoomStream *roomStream = WriteRoomStream::getInstance();
   StringLibrary * stringLibrary = StringLibrary::getInstance();

   if( words_ptr->word_count < 1 )
   {
      *userStream << addUser( user )
                 << "Give who a pet?\n" << pod_send;
   }
   else
   {
      UR_OBJECT u;

      if ( !( u = get_user(words_ptr->word[1])) )
      {
         *userStream << addUser( user )
                    << stringLibrary->makeString("notloggedon") << "\n" << pod_send;
      }
      else if (u==user)
      {
         *userStream << addUser( user )
                    << "Cant give yourself a pet!\n" << pod_send;
      }
      else if (!(user->room==u->room))
      {
         *userStream << addUser( user )
                    << "Cant give someone not in the same room a pet!\n" << pod_send;
      }
      else
      {
         IntGlobalVar  *pet_type    = (IntGlobalVar*)u->getVar("pet_type");

         if(pet_type->get() != 0)
         {
            *userStream << addUser( user )
                       << u->name << " already has a pet!\n" << pod_send;
         }
         else
         {
            *userStream << addUser( user )
                       << "You give " << u->name << " a pet!\n" << pod_send;

            *userStream << addUser( u )
                       << "You get a pet from " << user->name << "!\n" << pod_send;

            *roomStream << setRoom( user->room ) << addExcept( user ) << addExcept( u )
                       << user->name << " gives a pet to " << u->name << "!\n" << pod_send;

            pet_type->set( 1 );
         }
      }
   }
   return 0;
}
