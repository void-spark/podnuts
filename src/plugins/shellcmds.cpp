#include "../general_headers.h"
#include "../help.h"
#include "../more.h"

int regnif(UR_OBJECT user, char *inpstr);
int pukoolsn(UR_OBJECT user, char *inpstr);
int siohw(UR_OBJECT user, char *inpstr);

extern "C" void plugin_init()
{
   cmd_add("finger",   LEV_FIV, SYS, &regnif);
   cmd_add("whois",    LEV_FIV, SYS, &siohw);
   cmd_add("nslookup", LEV_FIV, SYS, &pukoolsn);
}

int write_file(UR_OBJECT user)
{
   char filename[80];

   sprintf(filename,"logfiles/system_temp.%s",user->name);
   switch(more(user,user->socket,filename))
   {
      case 0:
         write_user(user,"There is no info.\n");
         break;
      case 1:
         user->misc_op=MISC_MORE;
   }
   return 0;
}

/*** Use system's finger command to get info on troublesome user ***/
int regnif(UR_OBJECT user, char *inpstr)
{
   char filename[80];
   if (!strcmp(inpstr,"-r")) {
	write_file(user);
	return 0;
	}
   if (strpbrk(inpstr,";$/+*[]\\") ) {
        write_user(user,"Illegal character in address\n");
        return 0;
        }
   if ((!strlen(inpstr)) || (strlen(inpstr) < 8)) {
       write_user(user,"You need to specify address in form: user@host\n");
       return 0;
       }
   if (strlen(inpstr) > 41) {
       write_user(user,"Address specified too long. No greater than 41 chars.\n");
       return 0;
       }
   sprintf(filename,"logfiles/system_temp.%s",user->name);
   pod_stringstream outputStream;
   outputStream << "finger " << inpstr << " > " << filename << " 2> /dev/null";
   system((char*)outputStream.str().c_str());
   write_user(user,"Done.\n");
   return 0;
}

/*** Use system's nslookup command to resolve an ip address ***/
int pukoolsn(UR_OBJECT user, char *inpstr)
{
   char filename[80];
   if (!strcmp(inpstr,"-r")) {
        write_file(user);
        return 0;
        }
   if (strpbrk(inpstr,";$/+*[]\\") ) {
        write_user(user,"Illegal character in ip address\n");
        return 0;
        }
   if ((!strlen(inpstr)) || (strlen(inpstr) < 7)) {
       write_user(user,"You need to specify a valid ip address\n");
       return 0;
       }
   if (strlen(inpstr) > 25) {
       write_user(user,"Address specified too long. No greater than 25 chars.\n");
       return 0;
       }
   sprintf(filename,"logfiles/system_temp.%s",user->name);

   pod_stringstream outputStream;
   outputStream << "nslookup " << inpstr << " > " << filename << " 2> /dev/null";
   system((char*)outputStream.str().c_str());

   write_user(user,"Done.\n");
   return 0;
}

/*** Use system's whois command to find what a domain name is ***/
int siohw(UR_OBJECT user, char *inpstr)
{
   char filename[80];
   if (!strcmp(inpstr,"-r")) {
        write_file(user);
        return 0;
        }
   if (strpbrk(inpstr,";:$/+*[]\\") ) {
        write_user(user,"Illegal character in search string\n");
        return 0;
        }
   if ((!strlen(inpstr)) || (strlen(inpstr) < 3)) {
       write_user(user,"You need to specify a valid search string\n");
       return 0;
       }
   if (strlen(inpstr) > 45) {
       write_user(user,"String specified too long. No greater than 45 chars.\n");
       return 0;
       }

   sprintf(filename,"logfiles/system_temp.%s",user->name);

   pod_stringstream outputStream;
   outputStream << "fwhois " << inpstr << " > " << filename << " 2> /dev/null";
   system((char*)outputStream.str().c_str());

   write_user(user,"Done.\n");
return 0;
}
