#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "general_headers.h"
#include "color.h"
#include "string_misc.h"
#include "GenericMessage.h"
#include "MailMessage.h"
#include "MailForward.h"
#include "help.h"
#include "set.h"

extern char *noyes2[];

int set_species(UR_OBJECT user, char *inpstr)
{
   StrGlobalVar *species = (StrGlobalVar*)user->getVar("species");

   if (words.word_count<2) 
   {
      if( species->get().length() == 0 ) write_user(user,"Species currently unset.\n");
      else write_userf(user,"Species Set to: %s\n",species->get().c_str() );
      return 0;
   }
   inpstr=remove_first(inpstr);
   if (strlen(inpstr) > SPECIES_LEN) 
   {
      write_user(user,"Species too long.\n");
      return 0;
   }
   species->set( inpstr );
   write_user(user,"Species Set.\n");
   return 0;
}

/*** Set user description ***/
int set_desc(UR_OBJECT user, char *inpstr)
{
   StrGlobalVar *desc  = (StrGlobalVar*)user->getVar("desc");

   if (words.word_count<2)                      write_userf(user,"Your current description is: %s\n", desc->get().c_str() );
   else if (strstr(words.word[1],"(CLONE)"))    write_user(user,"You cannot have that description.\n");  
   else if (strlen(inpstr)>USER_DESC_LEN) write_user(user,"Description too long.\n");  
   else
   {
      desc->set(inpstr);
      write_user(user,"Description set.\n");
   }
   return 0;
}

int set_noise(UR_OBJECT user, char *inpstr)
{
   StrGlobalVar *noise  = (StrGlobalVar*)user->getVar("Noise");

   if ( words.word_count < 2 || !strcmp(words.word[1],"") )
   {
      if( noise->get().length() == 0 ) write_userf(user,"\n~OLYour noise is not set, using default.\n\n");
      else write_userf(user,"\n~OLNoise set to: ~RS%s\n\n",noise->get().c_str() );
      return 0;
   }
   else 
   {
      inpstr = remove_first(inpstr);
      if ( strlen(inpstr) > NOISE_LEN )
      {
         write_userf(user,"\n~OLNoise too long.\n\n");
         return 0;
      }
      else 
      {
         if (!strcmp(words.word[1],"none")) 
         {
            noise->set( "" );
            write_userf(user,"\n~OLYour noise is now unset, using default.\n\n");
            return 0;
         }
         else 
         {
            noise->set( inpstr );
            write_userf(user,"\n~OLNoise set to: ~RS%s\n\n",noise->get().c_str() );
            return 0;
         }
      }
   }
}

int set_inphr(UR_OBJECT user, char *inpstr)
{
   StrGlobalVar *in_phrase  = (StrGlobalVar*)user->getVar("in_phrase");

   if ((words.word_count<2) || (!strcmp(words.word[1],""))) 
   {
      if(in_phrase->get().length() == 0) write_user(user,"Your in phrase is currently unset.\n");
      else write_userf(user,"Your current in phrase is: %s\n",in_phrase->get().c_str() );
      return 0;
   }
   inpstr=remove_first(inpstr);
   if (strlen(inpstr)>PHRASE_LEN) 
   {
      write_user(user,"Phrase too long.\n");  
      return 0;
   }
   in_phrase->set(inpstr);
   write_user(user,"In phrase set.\n");
   return 0;
}

int set_outphr(UR_OBJECT user, char *inpstr)
{
   StrGlobalVar *out_phrase  = (StrGlobalVar*)user->getVar("out_phrase");

   if ((words.word_count<2) || (!strcmp(words.word[1],""))) 
   {
      if(out_phrase->get().length() == 0) write_user(user,"Your out phrase is currently unset.\n");
      else write_userf(user,"Your current out phrase is: %s\n",out_phrase->get().c_str() );
      return 0;
   }
   inpstr=remove_first(inpstr);
   if (strlen(inpstr)>PHRASE_LEN) 
   {
      write_user(user,"Phrase too long.\n");  
      return 0;
   }
   out_phrase->set(inpstr);
   write_user(user,"Out phrase set.\n");
   return 0;
}

int set_comephr(UR_OBJECT user, char *inpstr)
{
   StrGlobalVar *enter_phrase  = (StrGlobalVar*)user->getVar("enter_phrase");

   if ((words.word_count<2) || (!strcmp(words.word[1],""))) 
   {
      if(enter_phrase->get().length() == 0) write_user(user,"Your enter phrase is currently unset.\n");
      else write_userf(user,"Your current enter phrase is: %s\n",enter_phrase->get().c_str() );
      return 0;
   }
   inpstr=remove_first(inpstr);
   if (strlen(inpstr)>ENT_LVE_LEN) 
   {
      write_user(user,"Phrase too long.\n");  
      return 0;
   }
   enter_phrase->set(inpstr);
   write_user(user,"Enter phrase set.\n");
   return 0;
}

int set_gonephr(UR_OBJECT user, char *inpstr)
{
   StrGlobalVar *leave_phrase  = (StrGlobalVar*)user->getVar("leave_phrase");

   if ((words.word_count<2) || (!strcmp(words.word[1],""))) 
   {
      if(leave_phrase->get().length() == 0) write_user(user,"Your leave phrase is currently unset.\n");
      else write_userf(user,"Your current leave phrase is: %s\n",leave_phrase->get().c_str());
      return 0;
   }
   inpstr=remove_first(inpstr);
   if (strlen(inpstr)>ENT_LVE_LEN) /* how's everything else ? */
   {
      write_user(user,"Phrase too long.\n"); 
      return 0;
   }
   leave_phrase->set(inpstr);
   write_user(user,"Leave phrase set.\n");
   return 0;
}

int set_timezone(UR_OBJECT user, char *inpstr)
{
   IntGlobalVar *timezone  = (IntGlobalVar*)user->getVar("timezone");

   int val=0;
   char* first_invalid = (char*)(10); /*not NULL , can be anything else)*/

   if ( words.word_count != 2 )
   {
      if( timezone->get() >= 0 )
      {
         write_userf( user, "Your current timezone is: UTC +%i\n", timezone->get() );
      }
      else
      {
         write_userf( user, "Your current timezone is: UTC %i\n", timezone->get() );
      }
      return 0;
   }

   val = strtol(words.word[1],&first_invalid,10);

   if(words.word[1] == first_invalid)
   {
      write_userf(user,"Please enter a nummeric timezone offset in hours.\n");
      return 0;
   }
   if( *first_invalid )
   {
      write_userf(user,"Eek!, no strange char's after the number :). ( %s )\n",words.word[1]);
      return 0;
   }
   if( val > 12 || val < -12 )
   {
      write_userf(user,"Please enter an offset between -12 and +12 hours.\n");
      return 0;
   }

   timezone->set(val);
   write_user(user,"Timezone set.\n");
   return 0;
}

int set_autoread(UR_OBJECT user, char *inpstr)
{
   strtolower(inpstr);
   if ((words.word_count<2) || (!strcmp(words.word[1],""))) 
   {
      write_userf(user,"\n~OLAutoread set to: ~RS%s\n\n",noyes2[user->autoread]);
      return 0;
   }
   if (!strcmp(words.word[1],"yes")) 
   {
      if (user->autoread==0) 
      {
         user->autoread=1;
         write_user(user,"You will now autoread mail on login.\n\n");
         return 0;
      }
      else 
      {
         write_user(user,"You already have autoread enabled.\n\n");
         return 0;
      }
   }
   if (!strcmp(words.word[1],"no")) 
   {
      if (user->autoread==1) 
      {
         user->autoread=0;
         write_user(user,"You will no longer autoread mail on login.\n\n");
         return 0;
      }
      else 
      {
         write_user(user,"You do not have autoread enabled.\n\n");
         return 0;
      }
   }
   write_user(user,"Usage: set autoread <on/off>\n\n");
   return 0;
}

int set_hide(UR_OBJECT user, char *inpstr)
{
   strtolower(inpstr);
   if ((words.word_count<2) || (!strcmp(words.word[1],""))) 
   {
      write_user(user,"You are currently hidding: \n");
      write_userf(user,"~OL     Email: ~RS%s\n",noyes2[user->hide_email]);
      write_userf(user,"~OL     URL  : ~RS%s\n\n",noyes2[user->hide_url]);
      return 0;
   }
   if (!strcmp(words.word[1],"email")) 
   {
      if (user->hide_email==0) 
      { 
         user->hide_email=1;
         write_user(user,"You have hidden your email.\n\n");
         return 0;
      }
      else 
      { 
         user->hide_email=0;
         write_user(user,"Your email is now visible.\n\n");
         return 0;
      }
   }
   if (!strcmp(words.word[1],"url")) 
   {
      if (user->hide_url==0) 
      { 
         user->hide_url=1;
         write_user(user,"You have hidden your url.\n\n");
         return 0;
      }
      else 
      {
         user->hide_url=0;
         write_user(user,"Your url is now visible.\n\n");
         return 0;
      }
   }
   write_user(user,"Usage: set hide <email/url>\n\n");
   return 0;
}

int set_help(UR_OBJECT user, char *inpstr)
{
   strtolower(inpstr);
   if ((words.word_count<2) || (!strcmp(words.word[1],""))) 
   {
      write_user(user,"Your .help is currently set to default to: \n");
      if(user->help_mode==HELP_INDEX) write_user(user,"Index\n");
      else if(user->help_mode==HELP_COMMANDS)write_user(user,"Commands\n");
      else if(user->help_mode==HELP_LIST)write_user(user,"List\n");
      return 0;
   }
   if (!strcmp(words.word[1],"index")) 
   {
      user->help_mode=HELP_INDEX;
      write_user(user,"Your .help is now set to default to 'index'.\n\n");
      return 0;
   }
   if (!strcmp(words.word[1],"commands")) 
   {
      user->help_mode=HELP_COMMANDS;
      write_user(user,"Your .help is now set to default to 'commands'.\n\n");
      return 0;
   }
   if (!strcmp(words.word[1],"list")) 
   {
         user->help_mode=HELP_LIST;
         write_user(user,"Your .help is now set to default to 'list'.\n\n");
         return 0;
   }
   
   write_user(user,"Usage: set help <index/commands/list>\n\n");
   return 0;
}

int set_login(UR_OBJECT user, char *inpstr)
{
   RoomsVector::iterator roomNode;
   StrGlobalVar *prefroom      = (StrGlobalVar*)user->getVar("PrefRoom");

   strtolower(inpstr);
   if ((words.word_count<2) || (!strcmp(words.word[1],""))) 
   {
      write_userf(user,"Your current login room is the ~OL%s.\n",prefroom->get().c_str());
      return 0;
   }
   if (!strcmp(words.word[1],globalSpecialRoomNames.getJailRoomName()->get().c_str()))
   {
      write_user(user,"You cannot log into that room.\n");
      return 0;
   }
   for(roomNode=roomsList.begin();roomNode != roomsList.end();roomNode++)
   {
      if (!strncmp((*roomNode)->name,words.word[1],strlen(words.word[1])))
      {
         if (!has_room_access(user,(*roomNode)))
         {
            write_user(user,"You do not have access to that room.\n");
            return 0;
         }
         else 
         {
            prefroom->set(words.word[1]);
            write_user(user,"Login Room Set.\n");
            return 0;
         }
      }
   }
   write_user(user,"That room does not exist.\n");
   return 0;
}

int set_gend(UR_OBJECT user, char *inpstr)
{
   StrGlobalVar *desc  = (StrGlobalVar*)user->getVar("desc");
   StrGlobalVar *gend_choice   = (StrGlobalVar*)user->getVar("GendChoice");

   strtolower(inpstr);
   if ((words.word_count<2) || (!strcmp(words.word[1],""))) 
   {
      write_userf(user,"Gender set to: %s\n",gend_choice->get().c_str() );
      return 0;
   }
   if ( !strcmp( desc->get().c_str() ,"hasn't used .desc yet" ) )
   {
      write_user(user,"Set your description first.\n");
      return 0;
   }
   strtolower(words.word[1]);
   if (strlen(words.word[1]) > GEND_CHOICE_LEN) 
   {
      write_user(user,"Gender too long.\n");
      write_user(user,"Gender must be ~OL~FTMale~RS or ~OL~FMFemale~RS!\n");
      return 0;
   }
   if((strcmp(words.word[1],"male")) && (strcmp(words.word[1],"female")))
   {
      write_user(user,"Gender must be ~OL~FTMale~RS or ~OL~FMFemale~RS!\n");
      return 0;
   }
   gend_choice->set( words.word[1] );
   write_user(user,"Gender set.\n");
   return 0;
}

int set_gend_desc(UR_OBJECT user, char *inpstr)
{
   StrGlobalVar *gend_desc     = (StrGlobalVar*)user->getVar("Gender");

   strtolower(inpstr);
   if ((words.word_count<2) || (!strcmp(words.word[1],""))) 
   {
      write_userf(user,"Gender set to: %s\n",gend_desc->get().c_str() );
      return 0;
   }
   if (strlen(words.word[1]) > GEND_DESC_LEN) 
   {
      write_user(user,"Gender description too long.\n");
      return 0;
   }
   gend_desc->set( words.word[1] );
   write_user(user,"Gender set.\n");
   return 0;
}


int set_mood(UR_OBJECT user, char *inpstr)
{
   StrGlobalVar *mood  = (StrGlobalVar*)user->getVar("mood");

   int i=0,val=0;
   int col1_start,col2_start,col3_start;
   int last_column;
   int mood_node;
   char* first_invalid = (char*)(10); /*not NULL , can be anything else)*/

   if ((words.word_count<2) || (!strcmp(words.word[1],""))) 
   {
      write_userf(user,"Mood Set to: %s\n\n", mood->get().c_str() );
      write_userf(user,"Usage : .set mood <mood number>\n");
      write_userf(user,"~FGPossible moods are (type .faq mood to see what they mean):\n\n");

      for(i=0; mood_list[i][0] != '*'; i++ );

      val=(int)(i/4);

      last_column = i%4;
      
      switch(last_column)
      {
         case 0:
            col1_start = val;
            col2_start = 2*val;
            col3_start = 3*val;
            break;
         case 1:
            col1_start = val+1;
            col2_start = 2*val+1;
            col3_start = 3*val+1;
            break;
         case 2:
            col1_start = val+1;
            col2_start = 2*val+2;
            col3_start = 3*val+2;
            break;
         case 3:
            col1_start = val+1;
            col2_start = 2*val+2;
            col3_start = 3*val+3;
            break;
         default:
            break;
            /* no waaaaaaaaaay */
      }
                   
      
      for( i=0; mood_list[i][0] != '*'; i++ )
      {
              if (i%4 == 0) mood_node = (i/4);
         else if (i%4 == 1) mood_node = (i/4) + col1_start;
         else if (i%4 == 2) mood_node = (i/4) + col2_start;
         else               mood_node = (i/4) + col3_start;
         
         if (i%4 != 3)
         {
            write_userf(user,"~OL~FB %-2d %*s          ",   mood_node,
            MOOD_LEN + ( color_com_count( mood_list[mood_node] ) * 3 ),
            mood_list[mood_node].c_str() );
         }
         else
         {
            write_userf(user,"~OL~FB %-2d %*s\n",           mood_node,
            MOOD_LEN + ( color_com_count( mood_list[mood_node] ) * 3 ),
            mood_list[mood_node].c_str() );
         }
      }

      write_user(user,"\n");
      return 0;
   }

   val=strtol(words.word[1],&first_invalid,10);
   if(words.word[1] == first_invalid) 
   {
      write_userf(user,"Mood value should be a number (see \".set mood\").\n");
      return 0;
   }
   if((*first_invalid != '-') && (*first_invalid)) 
   {
      write_userf(user,"Eek!, no strange char's after the number :). ( %s )\n",words.word[1]);
      return 0;
   }
                
   i=0;
   while( mood_list[i][0] != '*' )
   {
      ++i;
   }
   if (val < 0) 
   {
      write_userf(user,"No negative values allowed! ( %s )\n",words.word[1]);
      return 0;
   }
   if (val >= i) 
   {
      write_userf(user,"Value to large! ( %s )\n",words.word[1]);
      return 0;
   }
   if( mood_list[val].size() > TOTAL_MOOD_LEN)
   {
      write_userf(user,"Mood number %d too long! . mood NOT set to: %s\n",val,mood_list[val].c_str() );
      logStream << setLogfile( SYSLOG ) << noTime << "Mood number " << val << " too long! . mood NOT set to: " << mood_list[val] << "\n" << pod_send;
   }
   else
   {
      mood->set(mood_list[val]);
      write_userf(user,"Mood set to: %s\n",mood_list[val].c_str() );
   }
   return 0;
}



int set_age(UR_OBJECT user, char *inpstr)
{
   StrGlobalVar *bday  = (StrGlobalVar*)user->getVar("Age");

   if ((words.word_count<2) || (!strcmp(words.word[1],"")))
   {
      write_userf(user,"Age Set to: %s\n",bday->get().c_str() );
      return 0;
   }
   if (strlen(words.word[1]) > AGE_LEN) 
   {
      write_user(user,"Age too long.\n");
      return 0;
   }
   bday->set( words.word[1] );
   write_user(user,"Age Set.\n");
   return 0;
}

int set_email(UR_OBJECT user, char *inpstr)
{
   StrGlobalVar *email         = (StrGlobalVar*)user->getVar("Email");

   if ((words.word_count<2) || (!strcmp(words.word[1],"")))
   {
      write_userf(user,"Email Set to: %s\n",email->get().c_str() );
      return 0;
   }
   if (strlen(words.word[1]) > EMAIL_LEN) 
   {
      write_user(user,"Email too long.\n");
      return 0;
   }
   email->set( words.word[1] );
   write_user(user,"Email Set.\n");
   return 0;
}

int set_birthday(UR_OBJECT user, char *inpstr)
{
   StrGlobalVar *bday  = (StrGlobalVar*)user->getVar("Squeeze_out_of_a_tight_slit_day");

   char* nextnum;
   int val;
   char* first_invalid = (char*)(10); /*not NULL , can be anything else)*/
   if ((words.word_count<2) || (!strcmp(words.word[1],""))) 
   {
      write_userf(user,"Birthday set to: %s\n",bday->get().c_str() );
      return 0;
   }
   if (strlen(words.word[1]) > BDAY_LEN) 
   {
      write_user(user,"Bday string too long.\n");
      return 0;
   }
   val=strtol(words.word[1],&first_invalid,10);
   if(words.word[1] == first_invalid) 
   {
      write_user(user,"Birthday should be a value in the form \"mm/dd\" .\n");
      return 0;
   }
   if((*first_invalid != '/')) 
   {
      write_user(user,"Birthday should be a value in the form \"mm/dd\" , found no '/' .\n");
      return 0;
   }
   if (val < 1) 
   {
      write_userf(user,"No month lower then 1 allowed! ( %s )\n",words.word[1]);
      return 0;
   }
   if (val > 12) 
   {
      write_userf(user,"No month higher then 12 allowed! ( %s )\n",words.word[1]);
      return 0;
   }
   if((nextnum=strchr(words.word[1],'/')))
   {
      val=strtol(nextnum+1,&first_invalid,10);
      if((nextnum+1) == first_invalid) 
      {
         write_user(user,"Birthday should be a value in the form \"mm/dd\" .\n");
         return 0;
      }
      if((*first_invalid)) 
      {
         write_user(user,"Birthday should be a value in the form \"mm/dd\" , found no '/' .\n");
         return 0;
      }
      if (val < 1) 
      {
         write_userf(user,"No day lower then 1 allowed! ( %s )\n",words.word[1]);
         return 0;
      }
      if (val > 31) 
      {
         write_userf(user,"No day higher then 31 allowed! ( %s )\n",words.word[1]);
         return 0;
      }
   }
   bday->set( words.word[1] );
   write_user(user,"Birthday set.\n");
   return 0;
}

int set_icq(UR_OBJECT user, char *inpstr)
{
   StrGlobalVar *icq  = (StrGlobalVar*)user->getVar("Icq");

   if ((words.word_count<2) || (!strcmp(words.word[1],"")))
   {
      write_userf(user,"Icq number set to: %s\n",icq->get().c_str());
      return 0;
   }
   if (strlen(words.word[1]) > ICQ_LEN) 
   {
      write_user(user,"Icq number too long.\n");
      return 0;
   }
   icq->set( words.word[1] );
   write_user(user,"Icq number set.\n");
   return 0;
}

int set_url(UR_OBJECT user, char *inpstr)
{
   StrGlobalVar *url         = (StrGlobalVar*)user->getVar("Url");

   if ((words.word_count<2) || (!strcmp(words.word[1],""))) 
   {
      write_userf(user,"Homepage Set to: %s\n",url->get().c_str() );
      return 0;
   }
   if (strlen(words.word[1]) > URL_LEN) 
   {
      write_user(user,"URL too long.\n");
      return 0;
   }
   url->set( words.word[1] );
   write_user(user,"Homepage Set.\n");
   return 0;
}

int set_color(UR_OBJECT user, char *inpstr)
{
   StrGlobalVar *color         = (StrGlobalVar*)user->getVar("Color");
   unsigned int i;

   strtolower(inpstr);
   if ((words.word_count<2) || (!strcmp(words.word[1],""))) { }                
   else if (!strcmp(words.word[1],"black"))     color->set("~FK");
   else if (!strcmp(words.word[1],"purple"))    color->set("~FM");
   else if (!strcmp(words.word[1],"red"))       color->set("~FR");
   else if (!strcmp(words.word[1],"yellow"))    color->set("~FY");
   else if (!strcmp(words.word[1],"green"))     color->set("~FG");
   else if (!strcmp(words.word[1],"blue"))      color->set("~FB");
   else if (!strcmp(words.word[1],"turquoise")) color->set("~FT");
   else if (!strcmp(words.word[1],"white"))     color->set("~FW");
   else 
   {
      if (strlen(words.word[1])>COLOR_LEN) 
      {
         write_user(user,"That color string is too long! Try a shorter one.\n");
         return 0; 
      }
      if (strlen(words.word[1])%3!=0) 
      {
         write_user(user,"That is not a valid color setting.\n");
         return 0; 
      }
      pod_string colorString = words.word[1];
      strToUpper( colorString );
      for( i = 0; i < colorString.size() / 3; i++ )
      {
         if(!is_color(colorString,i*3))
         {
            write_user(user,"That is not a valid color setting .\n");
            return 0; 
         }
      }
      color->set( colorString );
   }
   write_userf(user,"%sThe color of this text is your speech color\n", color->get().c_str() );
   return 0;
}

int set_says(UR_OBJECT user, char *inpstr)
{
   strtolower(inpstr);
   if (nuts_talk_style.get()) 
   {
      nuts_talk_style.set(0);
      write_user(user,"Say Style is OLD\n\n");
      write_room_except(user->room,"\n~OL~FRSYSTEM:~RS Changing to OLD say style.\n\n",user);
      return 0;
   }
   else 
   {
      nuts_talk_style.set(1);
      write_user(user,"Say Style is NEW\n\n");
      write_room_except(user->room,"\n~OL~FRSYSTEM:~RS Changing to NEW say style.\n\n",user);
      return 0;
   }
   return 0;
}

int set_atmos(UR_OBJECT user, char *inpstr)
{
   strtolower(inpstr);
   if (atmos_on.get()) 
   {
      atmos_on.set(0);
      write_user(user,"Atmospherics OFF\n\n");
      write_room_except(user->room,"\n~OL~FRSYSTEM:~RS Atmospherics turned off.\n\n",user);
      return 0;
   }
   else 
   {
      atmos_on.set(1);
      write_user(user,"Atmospherics ON\n\n");
      write_room_except(user->room,"\n~OL~FRSYSTEM:~RS Atmospherics turned on.\n\n",user);
      return 0;
   }
   return 0;
}
       
int set_desc_caller(UR_OBJECT user, char *inpstr)
{
      StrGlobalVar *desc  = (StrGlobalVar*)user->getVar("desc");

      if ((words.word_count<2) || (!strcmp(words.word[1],""))) 
      {
         write_userf(user,"\n~OLDesc set to: ~RS%s\n\n",desc->get().c_str() );
         return 0;
      }
      inpstr = remove_first(inpstr);
      set_desc(user,inpstr);
      return 0;
}

struct set_type
{
   char *name;
   char *desc;
   int   level;
   int (*function)(UR_OBJECT user, char *inpstr);
} set_list[] = {  { "desc",     "<your description>",   LEV_ZER, set_desc_caller},
                  { "gend_desc","<gender description>", LEV_ONE, set_gend_desc  },
                  { "age",      "<your age>",           LEV_ONE, set_age        },
                  { "gender",   "<your gender>",        LEV_ZER, set_gend       },
                  { "species",  "<your species>"     ,  LEV_ONE, set_species    },
                  { "email",    "<email address>",      LEV_ONE, set_email      },
                  { "url",      "<homepage address>",   LEV_ONE, set_url        },
                  { "icq",      "<icq number>",         LEV_ONE, set_icq        },
                  { "color",    "<foreground color>",   LEV_ONE, set_color      },
                  { "fmail",    "<forwarding address>", LEV_ONE, set_fmail      },
                  { "autoread", "<'no'/'yes'>",         LEV_ONE, set_autoread   },
                  { "hide",     "<'email'/'url'>",      LEV_ONE, set_hide       },
                  { "noise"  ,  "<noise/'none'>",       LEV_ONE, set_noise      },
                  { "mood",     "<mood number>",        LEV_ONE, set_mood       },
                  { "birthday", "<mm/dd>",              LEV_ONE, set_birthday   },
                  { "help",     "<index/commands/list>",LEV_ZER, set_help       },
                  { "inphr"  ,  "<enter room phrase>",  LEV_TWO, set_inphr      },
                  { "outphr" ,  "<leave room phrase>",  LEV_TWO, set_outphr     },
                  { "comephr",  "<login phrase>",       LEV_ONE, set_comephr    },
                  { "gonephr",  "<logout phrase>",      LEV_ONE, set_gonephr    },
                  { "login",    "<room name>",          LEV_TWO, set_login      },
                  { "autofwd",  "<'no'/'yes'>",         LEV_ONE, set_autofwd    },
                  { "atmos",    "<--toggle",            LEV_FOU, set_atmos      },
                  { "says",     "<--toggle",            LEV_FIV, set_says       },
                  { "timezone", "<UTC offset in hours>",LEV_ZER, set_timezone   },
                  { "*",        NULL,                   0,       NULL           } };

int opts_set(UR_OBJECT user, char *inpstr)
{
   int set_cnt;
   int step_cnt=0;

   wordfind(inpstr);
   strtolower(words.word[0]);
   
   for(set_cnt=0;strcmp("*",set_list[set_cnt].name);set_cnt++)   
      if (!strcmp(words.word[0],set_list[set_cnt].name) && user->level >= set_list[set_cnt].level)
         return (int)set_list[set_cnt].function(user,inpstr);
		
   if(strcmp(words.word[0],"")) write_userf(user,"\n~OL~FR%s ~FWis not a valid SET parameter.\n\n",words.word[0]);
   
   write_user(user,"~FTValid ~OLSET ~RS~FTParameters are:\n\n");  
   for(set_cnt=0;strcmp("*",set_list[set_cnt].name);set_cnt++)
   {
      if (user->level >= set_list[set_cnt].level)
      {      
         if(set_list[set_cnt].level >= LEV_FOU) write_userf(user,"~FT%9s~RS %-23s",set_list[set_cnt].name,set_list[set_cnt].desc);
         else                                   write_userf(user,"~OL%9s~RS %-23s",set_list[set_cnt].name,set_list[set_cnt].desc);
         if(step_cnt%2) write_user(user,"\n");
         step_cnt++;
      }
   }
   write_user(user,"\n\n");
   return 0;
}


