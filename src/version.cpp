#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "general_headers.h"
#include "more.h"
#include "compiler.h"
#include "version.h"

void show_version(UR_OBJECT user)
{
   char filename[80];
   sprintf(filename,"%s/%s",DATAFILES,VERSIONFILE);
   switch(more(user,user->socket,filename)) 
   {
      case 0: write_user(user,"Unable to find the version file.\n");
      case 1: user->misc_op=MISC_MORE;
   }
}

void show_compiler_info(UR_OBJECT user)
{
   write_userf(user,"~FTCompile Mach. : ~FG%s\n", UNAME);
   write_userf(user,"~FTCompile date  : ~FG%s\n", COMPILE_TIME);
}



