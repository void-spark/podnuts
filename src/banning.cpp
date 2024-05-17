#include <string.h>
#include <fstream>
#include <unistd.h>
#include <ctype.h>
#include "general_headers.h"
#include "string_misc.h"
#include "loadsave_user.h"
#include "admin.h"
#include "more.h"
#include "StringLibrary.h"
#include "banning.h"

extern char *swear_words[];

bool isSiteNameMatch( pod_string site, pod_string pattern )
{
   // if line is a part of site, for example line aol.com is part of site hell.aol.com
   if( site.find( pattern ) != pod_string::npos )
   {   
      return true;  
   }
   return false;
}

bool isSiteIpMatch( pod_string site, pod_string pattern )
{
   pod_string::size_type patternPos = 0;
   pod_string::size_type patternSize = pattern.size();
   
   pod_string::size_type sitePos = 0;
   pod_string::size_type siteSize = site.size();
   
   while( true )
   {
      // ignore number in the site if the pattern has a * in that spot
      if (pattern[patternPos] == '*')
      {
         while( sitePos < siteSize && site[sitePos] != '.' )
         {
            sitePos++;
         }
         patternPos++;
      }
      
      // we reached the end of the pattern we're matching against and till now it
      // matched the site
      if( patternPos >= patternSize )
      {
         return true;
      }

      // we reached the end of the site string before reaching the end of the pattern
      if( sitePos >= siteSize )
      {
         return false;
      }
            
      // difference between site and pattern
      if( pattern[patternPos] != site[sitePos] )
      {  
         return false;
      }      
      
      patternPos++;
      sitePos++;
   }
}

bool isSiteMatch( pod_string site, pod_string pattern )
{
   if( isSiteNameMatch(site,pattern) )
   {
      return true;
   }
   
   if( isSiteIpMatch(site,pattern) )
   {
      return true;
   }
   
   return false;   
}

/*** See if users site is banned ***/
bool site_banned( pod_string site )
{
   char filename[80];

   sprintf(filename,"%s/%s",DATAFILES,SITEBAN);
   
   std::ifstream input;
   
   input.open( filename );
   pod_string pattern;
   
   while(!input.eof())
   {
      input >> pattern;
      
      if( isSiteMatch(site,pattern) )
      {
         input.close();
         return true;
      }
   }
   input.close();
   
   return false;
}

/*** See if user is banned ***/
int user_banned(char *name)
{
   FILE *fp;
   char line[82],filename[80];

   sprintf(filename,"%s/%s",DATAFILES,USERBAN);
   if (!(fp=fopen(filename,"r"))) return 0;

   fscanf(fp,"%s",line);
   while(!feof(fp)) 
   {
      if (!strcmp(line,name)) 
      {  
         fclose(fp);  
         return 1;  
      }
      fscanf(fp,"%s",line);
   }
   fclose(fp);

   return 0;
}

/*** List banned sites or users ***/
int listbans(UR_OBJECT user)
{
   int i;
   char filename[80];

   if (!strcmp(words.word[1],"sites")) 
   {
      write_user(user,"\n~BB*** Banned sites/domains ***\n\n"); 
      sprintf(filename,"%s/%s",DATAFILES,SITEBAN);
      switch(more(user,user->socket,filename)) 
      {
         case 0:
            write_user(user,"There are no banned sites/domains.\n\n");
            return 0;

         case 1: user->misc_op=MISC_MORE;
      }
      return 0;
   }
   if (!strcmp(words.word[1],"users")) 
   {
      write_user(user,"\n~BB*** Banned users ***\n\n");
      sprintf(filename,"%s/%s",DATAFILES,USERBAN);
      switch(more(user,user->socket,filename)) 
      {
         case 0:
            write_user(user,"There are no banned users.\n\n");
            return 0;

         case 1: user->misc_op=MISC_MORE;
      }
      return 0;
   }
   if (!strcmp(words.word[1],"swears")) 
   {
      write_user(user,"\n~BB*** Banned swear words ***\n\n");
      i=0;
      while(swear_words[i][0]!='*') 
      {
         write_user(user,swear_words[i]);
         write_user(user,"\n");
         ++i;
      }
      if (!i) write_user(user,"There are no banned swear words.\n");
      if (ban_swearing.get()) write_user(user,"\n");
      else write_user(user,"\n(Swearing ban is currently off)\n\n");
      return 0;
   }
   write_user(user,"Usage: listbans sites/users/swears\n"); 
   return 0;
}

/***************************************/
/*** linedit file functions            */
/***************************************/

#define RFF_SUCCES 0
#define RFF_NO_SUCH_NAME 1
#define RFF_FAILURE 2

int remove_from_file(UR_OBJECT user,char *filename,char *buffer,char *str)
{
   StringLibrary * stringLibrary = StringLibrary::getInstance();
   FILE *infp,*outfp;
   int cnt=0,found=0;

   if (!(infp=fopen(filename,"r"))) 
   {
      return RFF_NO_SUCH_NAME;
   }
   if (!(outfp=fopen("tempfile","w"))) 
   {
      write_userf(user,"%s: Couldn't open tempfile.\n",stringLibrary->makeString("syserror").c_str());
      write_syslog("ERROR: Couldn't open tempfile to write in remove_from_file().\n",0);
      fclose(infp);
      return RFF_FAILURE;
   }
   fscanf(infp,"%s",buffer);
   while(!feof(infp)) 
   {
      if (strcmp(str,buffer)) 
      {  
         fprintf(outfp,"%s\n",buffer);  
         cnt++;  
      }
      else found=1;
      fscanf(infp,"%s",buffer);
   }
   fclose(infp);
   fclose(outfp);
   if (!found) 
   {
      unlink_checked("tempfile","remove_from_file-1");
      return RFF_NO_SUCH_NAME;
   }
   if (!cnt) 
   {
      unlink_checked(filename,"remove_from_file-2");
      unlink_checked("tempfile","remove_from_file-3");
   }
   else rename("tempfile",filename);
   return RFF_SUCCES;
}

bool is_in_file( pod_string filename, pod_string str)
{
   std::ifstream input;
   
   input.open( filename.c_str() );
   pod_string buffer;
   
   while(!input.eof())
   {
      input >> buffer;
      if(buffer == str)
      {
         input.close();
         return true;
      }
   }
   input.close();
   
   return false;
}

/***************************************/
/*** ban a site (or domain) or user  ***/
/***************************************/

int ban(UR_OBJECT user,char* inpstr)
{
   char *usage="Usage: ban site/user <site/user name> [reason]\n";

   if (words.word_count<3) 
   {
      write_user(user,usage);  
      return 0;
   }
   if (!strcmp(words.word[1],"site")) {  ban_site(user);  return 0;  }
   if (!strcmp(words.word[1],"user")) 
   {  
      inpstr=remove_first(inpstr);
      if (words.word_count<4) ban_user(user,"none given");
      else  ban_user(user,remove_first(inpstr));  
      return 0;  
   }
   write_user(user,usage);
   return 0;
}

int ban_site(UR_OBJECT user)
{
   StringLibrary * stringLibrary = StringLibrary::getInstance();
   FILE *fp;
   char filename[80],host[MAX_SITE_LEN+1];

   sprintf(filename,"%s/%s",DATAFILES,SITEBAN);
   gethostname(host,MAX_SITE_LEN);
   if(strlen(words.word[2]) > MAX_SITE_LEN) write_user(user,"Site to long!\n");
   else if (!strcmp(words.word[2],host)) write_user(user,"You cannot ban the machine that this program is running on.\n");
   else if(is_in_file(filename,words.word[2])) write_user(user,"That site/domain is already banned.\n"); /* See if ban already set for given site */
   else if (!(fp=fopen(filename,"a"))) /* Write new ban to file */
   {
      write_userf(user,"%s: Can't open file to append.\n",stringLibrary->makeString("syserror").c_str());
      write_syslog("ERROR: Couldn't open file to append in ban_site().\n",0);
   }
   else
   {
      fprintf(fp,"%s\n",words.word[2]);
      fclose(fp);
      write_user(user,"Site/domain banned.\n");
      write_syslogf("%s BANNED site/domain %s.\n",TRUE,user->name,words.word[2]);
   }
   return 0;
}

int ban_user(UR_OBJECT user, char *reason)
{
   StringLibrary * stringLibrary = StringLibrary::getInstance();
   UR_OBJECT u=NULL;
   FILE *fp;
   char filename[80];

   pod_string nameString = words.word[2];
   capitalize( nameString );

   sprintf(filename,"%s/%s",DATAFILES,USERBAN);

   if (nameString == user->name) 
   {
      write_user(user,"Trying to ban yourself is the seventh sign of madness.\n");
   }
   else if(is_in_file(filename,nameString )) 
   {
      write_user(user,"That user is already banned.\n"); 
   }
   else if ((u=get_user_exactmatch((char*)nameString.c_str()))!=NULL) 
   {
      if (u->level>=user->level) write_user(user,"You cannot ban a user of equal or higher level than yourself.\n");
      /* Write new ban to file */
      else if (!(fp=fopen(filename,"a"))) 
      {
         write_userf(user,"%s: Can't open file to append.\n",stringLibrary->makeString("syserror").c_str());
         write_syslog("ERROR: Couldn't open file to append in ban_user().\n",0);
      }
      else
      {
         fprintf(fp,"%s\n",nameString.c_str());
         fclose(fp);
         write_user(user,"User banned.\n");
         write_syslogf("%s BANNED user %s.\n",TRUE,user->name,nameString.c_str());
         pod_stringstream outputStream;
         outputStream << "user has been banned, reason: " << reason << "\n";
         log_user(u,user->name, (char*)outputStream.str().c_str() );
         write_user(u,"\n\07~FR~OL~LIYou have been banned from here!\n\n");
         quit(u);
      }
   }   
   else if (!(u=create_user()))
   {
      write_userf(user,"%s: unable to create temporary user object.\n",stringLibrary->makeString("syserror").c_str());
      write_syslog("ERROR: Unable to create temporary user object in change_pass().\n",0);
   }
   else
   {
      strcpy(u->name,nameString.c_str());
      if (!load_user(u)) write_user_crt(user,stringLibrary->makeString("nosuchuser").c_str());
      else if (u->level>=user->level) write_user(user,"You cannot ban a user of equal or higher level than yourself, even if offline.\n");
      else
      {
         if (!(fp=fopen(filename,"a"))) 
         {
            write_userf(user,"%s: Can't open file to append.\n",stringLibrary->makeString("syserror").c_str());
            write_syslog("ERROR: Couldn't open file to append in ban_user().\n",0);
            return 0;
         }
         fprintf(fp,"%s\n",nameString.c_str());
         fclose(fp);
         write_user(user,"User banned.\n");
         write_syslogf("%s BANNED user %s.\n",TRUE,user->name,nameString.c_str());
         pod_stringstream outputStream;
         outputStream << "user has been banned, reason: " << reason << "\n";
         log_user(u,user->name, (char*)outputStream.str().c_str() );
      }
      destruct_user(u);
   }
   return 0;
}

/***************************************/
/*** uban a site (or domain) or user ***/
/***************************************/

int unban(UR_OBJECT user, char *inpstr)
{
   char *usage="Usage: unban site/user <site/user name> [reason]\n";

   if (words.word_count < 3) write_user(user,usage);
   else if (!strcmp(words.word[1],"site")) unban_site(user);  
   else if (!strcmp(words.word[1],"user")) 
   {  
      if (words.word_count<4) unban_user(user,"none given");
      else  
      {
         inpstr=remove_first(inpstr);
         unban_user(user,remove_first(inpstr));
      }
   }
   else write_user(user,usage);
   return 0;
}

int unban_site(UR_OBJECT user)
{
   char filename[80], site[MAX_SITE_LEN+1];
   int retval;

   sprintf(filename,"%s/%s",DATAFILES,SITEBAN);
   if((retval=remove_from_file(user,filename,site,words.word[2])))
   {
      if(retval==RFF_NO_SUCH_NAME) write_user(user,"That site/domain is not currently banned.\n");
      return 0;
   }
   write_userf(user,"Site ban removed for site %s.\n",words.word[2]);
   write_syslogf("%s UNBANNED site %s.\n",TRUE,user->name,words.word[2]);

   return 0;
}

int unban_user(UR_OBJECT user, char *reason)
{
   StringLibrary * stringLibrary = StringLibrary::getInstance();
   UR_OBJECT u;
   char filename[80],name[USER_NAME_LEN+1];
   int retval;
   pod_string nameString = words.word[2];
   capitalize(nameString);

   sprintf(filename,"%s/%s",DATAFILES,USERBAN);
   if((retval=remove_from_file(user,filename,name,(char*)nameString.c_str())))
   {
      if(retval==RFF_NO_SUCH_NAME) write_user(user,"That user is not currently banned.\n");
      return 0;
   }
   write_user(user,"User ban removed.\n");
   write_syslogf("%s UNBANNED user %s.\n",TRUE,user->name,nameString.c_str());
   /* all this for a ulog entry, jeesh ;) */
   if ((u=create_user())==NULL) 
   {
      write_userf(user,"%s: unable to create temporary user object.\n",stringLibrary->makeString("syserror").c_str());
      write_syslog("ERROR: Unable to create temporary user object in unban_user().\n",0);
      return 0;
   }
   strcpy(u->name,nameString.c_str());
   if (load_user(u))
   {
      pod_stringstream outputStream;
      outputStream << "user has been unbanned, reason: " << reason << "\n";
      log_user(u,user->name, (char*)outputStream.str().c_str() );
   }
   destruct_user(u);
   /* phew, done ;) */
   return 0;
}

