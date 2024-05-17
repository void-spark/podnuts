#include "pod_string.h"
#include "levels.h"
#include "socket_funcs.h"
#include "GlobalVars.h"
#include "Room.h"
#include "user_objects.h"
#include "gender.h"

char *gender_list[3][4] = {
// Subject Pronouns  Object Pronouns  Possessive Adjective  Possessive Pronoun
   { "he",           "him",           "his",                "his"    },
   { "she",          "her",           "her",                "hers"   },
   { "they",         "them",          "their",              "theirs" } };

char *getPronoun( int target, int type )
{
   return gender_list[target][type];
}

/* Get gender pronoun based on user's gender */
char *get_gender(UR_OBJECT user, pod_string noun_type)
{
   int target = PLURAL;
   int type   = 0;
   
   if( user != 0 )
   {
      bool invisible  = user->vis == 0 || user->cloaked == 1;
      bool inked      = user->room != 0 && user->room->inked;
      
      if( !invisible && !inked )
      {
         StrGlobalVar *gend_choice   = (StrGlobalVar*)user->getVar("GendChoice");
         pod_string gend = gend_choice->get();

         if( !gend.compare("male") )        
         {
            target = SINGULAR_MALE;
         }
         else if( !gend.compare("female") ) 
         {
            target = SINGULAR_FEMALE;
         }
      }
   }

   if( !noun_type.compare("he") || 
       !noun_type.compare("e") ||
       !noun_type.compare("they") ||
       !noun_type.compare("y") )
   {
      type=SUBJECT_PRONOUN;
   }
   else if( !noun_type.compare("him") || 
            !noun_type.compare("m") ||
            !noun_type.compare("them") )
   {
      type=OBJECT_PRONOUN;
   }
   else 
   {
      type=POSSESSIVE_ADJECTIVE;
   }
   
   return getPronoun( target, type );
}
