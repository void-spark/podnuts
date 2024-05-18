#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
   system("mv -f bin_new/pod bin/");

   execv("bin/pod",argv);

   printf("error in softboot_shutdown(), couldn't restart pod, we're dead, error:\n%s\n",strerror(errno));
   exit(13);
}
