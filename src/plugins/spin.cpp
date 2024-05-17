#include <vector>
#include "../levels.h"
#include "../socket_funcs.h"
#include "../GlobalVars.h"
#include "../Room.h"
#include "../user_objects.h"
#include "../speech_funcs.h"
#include "../help.h"
#include "../gender.h"
#include "../cmd_main.h"
#include "../StringLibrary.h"

#include "../clones.h"

int spin  (UR_OBJECT user,char *inpstr);
int flip  (UR_OBJECT user,char *inpstr);

extern "C" void plugin_init()
{
   StringLibrary::getInstance()->addFile("datafiles/spin.xml");

   cmd_add("spin",   LEV_ONE, FIG, &spin);
   cmd_add("flip",   LEV_ONE, FIG, &flip);
}

typedef std::vector< UR_OBJECT, pod_alloc< UR_OBJECT >::Type > UsersVector;

int spin(UR_OBJECT user,char *inpstr)
{
   StringLibrary * stringLibrary = StringLibrary::getInstance();
   UsersVector candidates;
   WriteUserStream *userStream = WriteUserStream::getInstance();
   WriteRoomStream *roomStream = WriteRoomStream::getInstance();

   for( UR_OBJECT u = user_first; u != 0; u = u->next )
   {
      if ( u != user && ( u->room == user->room ) && u->vis && !u->cloaked && !is_clone(u) )
      {
         candidates.push_back(u);
      }
   }

   stringLibrary->addTextTag( "name", user->name );
   stringLibrary->addTextTag( "his", get_gender( user, "his") );
   
   if ( candidates.size() == 0 )
   {
      *userStream << addUser( user )
                 << "This command only works if there are at least 2 people in the room!\n" << pod_send;
      return 0;
   }

   *roomStream  << setRoom( user->room )
                << stringLibrary->makeString( "spins" ) << '\n' << pod_send;

   int choice = (int) (1.0 * candidates.size() * rand() / (RAND_MAX + 1.0));
   
   if( choice < 0 || choice >= candidates.size() )
   {
      *userStream << addUser( user )
                 << "Internal error, choice: " << choice << " max: " << candidates.size() - 1 << " .\n" << pod_send;
      return 0;
   }
   
   stringLibrary->addTextTag( "random", candidates[choice]->name );

   *roomStream  << setRoom( user->room )
                << stringLibrary->makeString( "pointed_at" ) << '\n' << pod_send;
                
   return 0;
}

int flip(UR_OBJECT user,char *inpstr)
{
   StringLibrary * stringLibrary = StringLibrary::getInstance();
   WriteRoomStream *roomStream = WriteRoomStream::getInstance();
   int side;

   side = (int) (2.0 * rand() / (RAND_MAX + 1.0));

   stringLibrary->addTextTag( "name", user->name );
   stringLibrary->addTextTag( "his", get_gender( user, "his") );
   
   *roomStream  << setRoom( user->room )
                << stringLibrary->makeString( "flips" ) << '\n' << pod_send;

   if (side == 0)
   {
      *roomStream << setRoom( user->room ) 
                  << stringLibrary->makeString( "flip_up" ) << '\n' << pod_send;
   }
   else
   {
      *roomStream << setRoom( user->room )
                  << stringLibrary->makeString( "flip_down" ) << '\n' << pod_send;
   }
   return 0;
}
