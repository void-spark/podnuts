#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "general_headers.h"
#include "string_misc.h"
#include "more.h"
#include "xalloc.h"

#ifdef DEBUG_MALLOC
#define MALLSIZE 10000

struct mallelem
{
   char *ptr;
   size_t size;
   char desc[50];
} malltable[MALLSIZE];
#endif /* DEBUG_MALLOC */


#ifdef MALLOC_STATS
static int tmalloc, trealloc;
static int nmalloc, nrealloc, nfree;
#ifdef DEBUG_MALLOC
static int tfree;
#endif /* DEBUG_MALLOC */
#endif /* MALLOC_STATS */


void malloc_init()
{
   static bool firstTime = true;
   if( !firstTime )
   {
     return;
   }
   firstTime = false;

#ifdef DEBUG_MALLOC
   int i;
   for (i = 0; i < MALLSIZE; i++) malltable[i].ptr = (char *)NULL;
#endif

#ifdef MALLOC_STATS
   tmalloc  = 0;
   trealloc = 0;
   nmalloc  = 0;
   nrealloc = 0;
   nfree    = 0;
#ifdef DEBUG_MALLOC
   tfree = 0;
#endif /* DEBUG_MALLOC */
#endif /* MALLOC_STATS */
}

#if defined(MALLOC_STATS) || defined(DEBUG_MALLOC)
void dump_meminfo(UR_OBJECT user)
{
   int usage,tusage,count;

   malloc_init();

   write_seperator_line(user,"malloc_internal");
#ifdef MALLOC_STATS
   write_userf(user, "Total malloced:    %10d        %10d bytes. = %7d Kb.\n", nmalloc,  tmalloc,tmalloc >> 10 );
   write_userf(user, "Total realloced:   %10d        %10d bytes. = %7d Kb.\n", nrealloc, trealloc, trealloc >> 10);
#ifdef DEBUG_MALLOC
   write_userf(user, "Total freed:       %10d        %10d bytes. = %7d Kb.\n", nfree,    tfree, tfree >> 10);
   write_userf(user, "Free/malloc ratio: %10.2f%%\n", tmalloc ? (float)(tfree * 100) / (float)tmalloc : 0);
#else
   write_userf(user, "Total freed:       %10d\n", nfree);
   write_userf(user, "Free/malloc ratio: %10.2f%%\n", nmalloc ? (float)(nfree * 100) / (float)nmalloc : 0);
#endif /* DEBUG_MALLOC */
#endif /* MALLOC_STATS */
#ifdef DEBUG_MALLOC
   for (tusage = usage = count = 0; usage < MALLSIZE; usage++)
   {
      if (malltable[usage].ptr == (void *)NULL)
      {
         continue;
      }
      tusage += malltable[usage].size;
      count++;
   }
   write_userf(user, "\nDebug malloc:                        %10d bytes. = %7d Kb.\n", tusage, tusage >> 10);
   write_userf(user, "\nDebug malloc:                        %10d allocs.\n", count);
   write_seperator_line(user,NULL);
   write_user(user,"\n");
#endif /* DEBUG_MALLOC */
}
#endif

#ifdef DEBUG_MALLOC
int xalloced(void *r)
{
   int i;

   for (i = 0; i < MALLSIZE; i++)
      if (malltable[i].ptr == r) return i;

   return -1;
}
#endif /* DEBUG_MALLOC */

void *xalloc(size_t size, char *desc)
{
   char *r;
#ifdef DEBUG_MALLOC
   int i;
#endif

   malloc_init();

   if (!size) 
   {
      write_syslogf("ERROR: xalloc(0,%s).\n",FALSE,desc);
      return NULL;
   }
   r = (char*)malloc(size);
   if (r == (void *)NULL) 
   {
      write_syslogf("ERROR: Out of memory (for %s).\n",FALSE,desc);
      return NULL;
   }
#ifdef DEBUG_MALLOC
   if ((i = xalloced((void *)NULL)) == -1) 
   {
      write_syslog("ERROR: Malloc table overflow.\n",0);
      return NULL;
   }
   strcpy(malltable[i].desc, desc);
   malltable[i].ptr = r;
   malltable[i].size = size;

#endif /* DEBUG_MALLOC */
#ifdef MALLOC_STATS
   tmalloc += size;
   nmalloc++;
#endif
   return r;
}

void xfree(void *r)
{
#ifdef DEBUG_MALLOC
   int i;
#endif /* DEBUG_MALLOC */

   malloc_init();

   if (r == (void *)NULL)
   {
      write_syslog("ERROR: xfree on NULL pointer.\n",0);
      return;
   }
#ifdef DEBUG_MALLOC
   if ((i = xalloced(r)) == -1)
   {
      write_syslog("ERROR: Freeing unallocated memory.\n",0);
      return;
   }
   malltable[i].ptr = (char *)NULL;

#ifdef MALLOC_STATS
   tfree += malltable[i].size;
#endif /* MALLOC_STATS */
#endif /* DEBUG_MALLOC */
   free(r);
#ifdef MALLOC_STATS
   nfree++;
#endif
}

void *xrealloc(void *old, size_t size)
{
#ifdef DEBUG_MALLOC
   int i;
#endif /* DEBUG_MALLOC */
   char *r;

   malloc_init();

   if (!size) 
   {
      write_syslog("ERROR: xrealloc(*old,0).\n",0);
      return NULL;
   }

#ifdef DEBUG_MALLOC
   if ((i = xalloced(old)) == -1) 
   {
      write_syslog("ERROR: Reallocating unallocated memory.\n",0);
      return NULL;
   }
#endif

   r = (char*)realloc(old, size);
   if (r == (void *)NULL) 
   {
      write_syslog("ERROR: Out of memory.\n",0);
      return NULL;
   }

#ifdef DEBUG_MALLOC
   malltable[i].size = size;
   malltable[i].ptr = r;
#endif /* DEBUG_MALLOC */

#ifdef MALLOC_STATS
   tmalloc += size;
   trealloc += size;
   nrealloc++;
#endif
   return r;
}

#ifdef DEBUG_MALLOC
void dump_malloc_table(UR_OBJECT user)
{
   FILE *fp;
   int ret;

   malloc_init();

   int i, sum;
   char *p, *q;
   if ((fp = fopen("debug_malloc", "w")) == (FILE *)NULL)
   {
      return;
   }

   for (sum = i = 0; i < MALLSIZE; i++)
   {
      if (malltable[i].ptr == (void *)NULL)
      continue;
      fprintf(fp, "%p-%p : %s (%d) [", malltable[i].ptr,(malltable[i].ptr+malltable[i].size)-1, malltable[i].desc, malltable[i].size);
      q = malltable[i].ptr;
      for (p = q; (unsigned)(p - q) < malltable[i].size && p - q < 60; p++)
         if (isprint(*p))       
         {
            fputc(*p, fp);
         }
         else 
         {
            fputc('.', fp);
         }
      fprintf(fp, "]\n");
      sum += malltable[i].size;
   }
   fprintf(fp, "Total size %d\n", sum);
   fclose(fp);

   if (!(ret=more(user,0,"debug_malloc")))
	write_user(user,"problem reading temp file.\n");
   if (ret==1) user->misc_op=MISC_MORE;
}
#endif /* DEBUG_MALLOC */


/*
#ifdef DEBUG_MALLOC
void
dump_malloc_table(int opt)
{
   FILE *fp;
   int i, sum;
   char *p, *q;

   if ((fp = fopen("log/debug_malloc", "w")) == (FILE *)NULL)
   {
      log_perror("dump_malloc_table");
      return;
   }
   for (sum = i = 0; i < MALLSIZE; i++)
   {
      if (malltable[i].ptr == (void *)NULL)
      continue;
      if (!opt && malltable[i].desc[0] == '*')
      continue;
      fprintf(fp, "%p : %s (%d) [", malltable[i].ptr, malltable[i].desc, malltable[i].size);
      q = malltable[i].ptr;
      for (p = q; (unsigned)(p - q) < malltable[i].size && p - q < 10; p++)
         if (isprint(*p)) fputc(*p, fp);
         else fputc('.', fp);
      fprintf(fp, "]\n");
      sum += malltable[i].size;
   }
   fprintf(fp, "Total size %d\n", sum);
   fclose(fp);
}

void
dump_malloc_table_sig(int sig)
{
	signal(sig, dump_malloc_table_sig);
	dump_malloc_table(1);
} 
#endif*/ /* DEBUG_MALLOC */
 
