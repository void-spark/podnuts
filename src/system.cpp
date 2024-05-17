#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#ifdef __linux__
#include <linux/version.h>
#include <malloc.h>
#endif // __linux__
#include "general_headers.h"
#include "globals.h"
#include "loadsave_config.h"
#include "string_misc.h"
#include "softboot.h"
#include "hist.h"
#include "cmd_main.h"
#include "system.h"
#include "xalloc.h"

#ifdef __linux__

#ifndef KERNEL_VERSION
#define KERNEL_VERSION(a,b,c) (((a) << 16) | ((b) << 8) | (c))
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0)
#warning compiling proc code against 2.4 kernel
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,0)
#warning compiling proc code against 2.2 kernel
#else
#error pre 2.2 kernels not supported at this time.
#endif

struct proc_stat
{
               int  pid;
               char comm[80];
               char state;
               int  ppid;
               int  pgrp;
               int  session;
               int  tty;
               int  tpgid;
 long unsigned int  flags;
 long unsigned int  minflt;
 long unsigned int  cminflt;
 long unsigned int  majflt;
 long unsigned int  cmajflt;
 long unsigned int  utime;
 long unsigned int  stime;
 long          int  cutime;
 long          int  cstime;
 long          int  priority;
 long          int  nice;
 long          int  not_used_1;
 long          int  itrealvalue;
 long unsigned int  starttime;
 long unsigned int  vsize;
 long          int  rss; /* minus three for adm. purposes */
 long unsigned int  rlim;
 long unsigned int  startcode;
 long unsigned int  endcode;
 long unsigned int  startstack;
 long unsigned int  kstkesp;
 long unsigned int  kstkeip;
 long unsigned int  obsolete_signal;   /* kernel says so, obsolete :) */           
 long unsigned int  obsolete_blocked;
 long unsigned int  obsolete_sigignore;
 long unsigned int  obsolete_sigcatch;
 long unsigned int  wchan;
 long unsigned int  nswap;
 long unsigned int  cnswap;
               int  exit_signal;
               int  processor;          
};

struct proc_statm
{
   int size;
   int resident;
   int share;
   int trs;
   int lrs;
   int drs;
   int dt;      
};

struct proc_longstatus
{
   char cmd[80];
   char status[80];
   int  pid;
   int  ppid;

   int  tpid; // new in 2.4 kernel!

   int  uid;
   int  euid;
   int  suid;
   int  fsuid;

   int  gid;
   int  egid;
   int  sgid;
   int  fsgid;

   int tgid; //new in 2.4 kernel ?

   int  fdsize; // new in 2.4 kernel

   long unsigned int vm_size;
   long unsigned int vm_lock; 
   long unsigned int vm_rss;
   long unsigned int vm_data;
   long unsigned int vm_stack;
   long unsigned int vm_exe;
   long unsigned int vm_lib;
   
   long long int sigpnd;   
   long long int sigblk;   
   long long int sigign;   
   long long int sigcgt;   
   
   unsigned int cap_inheritable;
   unsigned int cap_permitted;
   unsigned int cap_effective;

};


int proc_load_stat(UR_OBJECT user,struct proc_stat *ps,int pid)
{
   char filename[80];
   FILE *fp;
   sprintf(filename,"/proc/%i/stat",pid);
   if(!(fp=fopen(filename,"r")))
   {
      write_syslogf("Error in proc_load_stat() while opening file '%s' : '%s'.\n",TRUE,filename,strerror(errno));
      write_user(user,"Sorry, couldn't open proc entry.\n");
      return 0;
   }

   if(fscanf(fp,"%d %s %c %d %d %d %d %d %lu %lu %lu %lu %lu %lu %lu \
                 %ld %ld %ld %ld %ld %ld %lu %lu %ld %lu %lu %lu %lu %lu \
                 %lu %lu %lu %lu %lu %lu %lu %lu %d %d",
               &ps->pid,
                ps->comm,
               &ps->state,
               &ps->ppid,
               &ps->pgrp,
               &ps->session,
               &ps->tty,
               &ps->tpgid,
               &ps->flags,
               &ps->minflt,
               &ps->cminflt,
               &ps->majflt,
               &ps->cmajflt,
               &ps->utime,
               &ps->stime,
               &ps->cutime,
               &ps->cstime,
               &ps->priority,
               &ps->nice,
               &ps->not_used_1,
               &ps->itrealvalue,
               &ps->starttime,
               &ps->vsize,
               &ps->rss, /* you might want to shift this left 3 */
               &ps->rlim,
               &ps->startcode,
               &ps->endcode,
               &ps->startstack,
               &ps->kstkesp,
               &ps->kstkeip,
               &ps->obsolete_signal,
               &ps->obsolete_blocked,
               &ps->obsolete_sigignore,
               &ps->obsolete_sigcatch,
               &ps->wchan,
               &ps->nswap,
               &ps->cnswap,
               &ps->exit_signal,
               &ps->processor            ) == EOF) 
   { 
      fclose(fp); 
      return 0; 
   }
   fclose(fp);
   return 1;
}

int proc_show_stat(UR_OBJECT user, int show_kb)
{
   struct proc_stat ps;
   size_t our_page_size = getpagesize();
   if(!proc_load_stat(user,&ps,getpid())) return 0;

   write_seperator_line(user,"stat");
   write_userf(user,"Process id              : %d\n",ps.pid);
   write_userf(user,"exe. filename           : %s\n",ps.comm);
   write_userf(user,"state                   : %c\n",ps.state);
   write_userf(user,"ppid                    : %d\n",ps.ppid);
   write_userf(user,"pgrp                    : %d\n",ps.pgrp);
   write_userf(user,"session                 : %d\n",ps.session);
   write_userf(user,"tty                     : %d\n",ps.tty);
   write_userf(user,"tpgid                   : %d\n",ps.tpgid);
   write_userf(user,"flags                   : %lu\n",ps.flags);
   write_userf(user,"minor faults proc.      : %lu\n",ps.minflt);
   write_userf(user,"minor faults chld.      : %lu\n",ps.cminflt);
   write_userf(user,"major faults proc.      : %lu\n",ps.majflt);
   write_userf(user,"major faults chld.      : %lu\n",ps.cmajflt);
   write_userf(user,"time p. in user mode    : %.2lu:%.2lu:%.2lu.%.2lu\n",ps.utime/360000,(ps.utime%360000)/6000,((ps.utime%360000)%6000)/100,((ps.utime%360000)%6000)%100);
   write_userf(user,"time p. in kern. mode   : %.2lu:%.2lu:%.2lu.%.2lu\n",ps.stime/360000,(ps.stime%360000)/6000,((ps.stime%360000)%6000)/100,((ps.stime%360000)%6000)%100);
   write_userf(user,"time p.+c. in usr mode  : %.2ld:%.2ld:%.2ld.%.2ld\n",ps.cutime/360000,(ps.cutime%360000)/6000,((ps.cutime%360000)%6000)/100,((ps.cutime%360000)%6000)%100);
   write_userf(user,"time p.+c. in krn. mode : %.2ld:%.2ld:%.2ld.%.2ld\n",ps.cstime/360000,(ps.cstime%360000)/6000,((ps.cstime%360000)%6000)/100,((ps.cstime%360000)%6000)%100);
   write_userf(user,"priority                : %ld\n",ps.priority);
   write_userf(user,"nice value              : %ld\n",ps.nice);
   write_userf(user,"SIGALRM timer           : %.2ld:%.2ld:%.2ld.%.2ld\n",ps.itrealvalue/360000,(ps.itrealvalue%360000)/6000,((ps.itrealvalue%360000)%6000)/100,((ps.itrealvalue%360000)%6000)%100);
   write_userf(user,"time sysboot -> procstrt: %.2lu:%.2lu:%.2lu.%.2lu\n",ps.starttime/360000,(ps.starttime%360000)/6000,((ps.starttime%360000)%6000)/100,((ps.starttime%360000)%6000)%100);
   if(show_kb)
   {
      write_userf(user,"Virtual memory size     : %lu Kb.\n",ps.vsize>>10);
      write_userf(user,"Resident Set Size       : %ld Kb.\n",((ps.rss * our_page_size) >> 10));
      write_userf(user,"RSS limit               : %#lx Kb.\n",ps.rlim >> 10);
   }
   else
   {
      write_userf(user,"Virtual memory size     : %lu bytes.\n",ps.vsize);
      write_userf(user,"Resident Set Size       : %ld bytes.\n",(ps.rss * our_page_size));
      write_userf(user,"RSS limit               : %#lx bytes.\n",ps.rlim);
   }
   write_userf(user,"begin of runnable 'text': %#.8lx.\n",ps.startcode);
   write_userf(user,"end of runnable 'text'  : %#.8lx.\n",ps.endcode);
   write_userf(user,"begin of stack          : %#.8lx.\n",ps.startstack);
   write_userf(user,"32-bit stack pointer    : %#.8lx.\n",ps.kstkesp);
   write_userf(user,"32-bit instruction ptr. : %#.8lx.\n",ps.kstkeip);
   write_userf(user,"addr. of wchan (syscall): %#.8lx.\n",ps.wchan);
   write_userf(user,"number of swaps         : %lu\n",ps.nswap);
   write_userf(user,"number of chld swaps    : %lu\n",ps.cnswap);
   write_userf(user,"exit_signal             : %d\n",ps.exit_signal);
   write_userf(user,"Processor               : %d\n",ps.processor);
   write_seperator_line(user,NULL);
   write_user(user,"\n");
   return 0;
}

int proc_load_statm(UR_OBJECT user, struct proc_statm *ps_m,int pid)
{
   char filename[80];
   FILE *fp;

   sprintf(filename,"/proc/%i/statm",pid);
   if(!(fp=fopen(filename,"r")))
   {
      write_syslogf("Error in proc_load_statm() while opening file '%s' : '%s'.\n",TRUE,filename,strerror(errno));
      write_user(user,"Sorry, couldn't open proc entry.\n");
      return 0;
   }
   if(fscanf(fp,"%d %d %d %d %d %d %d",
               &ps_m->size,
               &ps_m->resident,
               &ps_m->share,
               &ps_m->trs,
               &ps_m->lrs,
               &ps_m->drs,
               &ps_m->dt          ) == EOF) 
   { 
      fclose(fp); 
      return 0; 
   }
   fclose(fp);
   return 1;
}

int proc_show_statm(UR_OBJECT user, int show_kb)
{
   struct proc_statm ps_m;
   size_t our_page_size = getpagesize();

   if(!proc_load_statm(user,&ps_m,getpid())) return 0;
   
   write_seperator_line(user,"statm");
   if(show_kb)
   {
      write_userf(user,"Total program size (contains all below): %d Kb.\n",(ps_m.size     * our_page_size) >> 10);
      write_userf(user,"in memory portions (how much in mem)   : %d Kb.\n",(ps_m.resident * our_page_size) >> 10);
      write_userf(user,"shared pages       (how much shared)   : %d Kb.\n",(ps_m.share    * our_page_size) >> 10);
      write_userf(user,"code pages         (part code)         : %d Kb.\n",(ps_m.trs      * our_page_size) >> 10);
      write_userf(user,"library pages      (part library)      : %d Kb.\n",(ps_m.lrs      * our_page_size) >> 10);
      write_userf(user,"data/stack pages   (part data)         : %d Kb.\n",(ps_m.drs      * our_page_size) >> 10);
      write_userf(user,"dirty pages        (how much 'dirty')  : %d Kb.\n",(ps_m.dt       * our_page_size) >> 10);
   }
   else
   {
      write_userf(user,"Total program size (contains all below): %d bytes.\n",ps_m.size     * our_page_size );
      write_userf(user,"in memory portions (how much in mem)   : %d bytes.\n",ps_m.resident * our_page_size );
      write_userf(user,"shared pages       (how much shared)   : %d bytes.\n",ps_m.share    * our_page_size );
      write_userf(user,"code pages         (part code)         : %d bytes.\n",ps_m.trs      * our_page_size );
      write_userf(user,"library pages      (part library)      : %d bytes.\n",ps_m.lrs      * our_page_size );
      write_userf(user,"data/stack pages   (part data)         : %d bytes.\n",ps_m.drs      * our_page_size );
      write_userf(user,"dirty pages        (how much 'dirty')  : %d bytes.\n",ps_m.dt       * our_page_size );
   }
   write_seperator_line(user,NULL);
   write_user(user,"\n");
   return 0;
}

void StripWhite(pod_string & word)
{
    int first = 0;
    int len = word.length();

    while (first < len && isspace(word[first]))
    {
        first++;
    }
    // assert: first indexes either '\0' or non-punctuation

    // now find last non-nonpunctuation character

    int last = len - 1;  // last char in s

    while(last >= 0 && isspace(word[last]))
    {
        last--;
    }
    word = word.substr(first,last-first+1);
}


int proc_load_status(UR_OBJECT user, struct proc_longstatus *ps_x,int pid)
{
   char filename[80];

   sprintf(filename,"/proc/%i/status",pid);

   memset( ps_x, 0, sizeof(proc_longstatus) );
   PodFile statFile(filename);
   statFile.open_read();
   pod_string::size_type pos;
   pod_string  nextLine = "";
   pod_string  name = "";
   pod_string  value = "";
   while( !statFile.eof() )
   {
      pod_string  nextLine = statFile.readLine();

      if( (pos=nextLine.find(':')) == pod_string::npos || pos == (nextLine.size()-1) )
      {
         write_syslogf("In function proc_load_status, unexpected line: <%s>\n", TRUE, nextLine.c_str());
         continue;
      }

      name.assign(nextLine,0,pos);
      value.assign(nextLine,pos+1,nextLine.size()-(pos+1));

      StripWhite(name);
      StripWhite(value);

      if( name == "Name" )
      {
         strcpy(ps_x->cmd, value.c_str());
      }
      else if( name == "State" )
      {
         strcpy(ps_x->status, value.c_str());
      }
      else if( name == "Tgid" )
      {
         sscanf( value.c_str(), "%d", &ps_x->tgid );
      }
      else if( name == "Pid" )
      {
         sscanf( value.c_str(), "%d", &ps_x->pid );
      }
      else if( name == "PPid" )
      {
         sscanf( value.c_str(), "%d", &ps_x->ppid );
      }
      else if( name == "TracerPid" )
      {
         sscanf( value.c_str(), "%d", &ps_x->tpid );
      }
      else if( name == "Uid" )
      {
         sscanf( value.c_str(), "%d\t%d\t%d\t%d", &ps_x->uid, &ps_x->euid, &ps_x->suid, &ps_x->fsuid );
      }
      else if( name == "Gid" )
      {
         sscanf( value.c_str(), "%d\t%d\t%d\t%d", &ps_x->gid, &ps_x->egid, &ps_x->sgid, &ps_x->fsgid );
      }
      else if( name == "FDSize" )
      {
         sscanf( value.c_str(), "%d", &ps_x->fdsize );
      }
      else if( name == "Groups" )
      {
      }
      else if( name == "VmSize" )
      {
         sscanf( value.c_str(), "%lu kB", &ps_x->vm_size );
      }
      else if( name == "VmLck" )
      {
         sscanf( value.c_str(), "%lu kB", &ps_x->vm_lock );
      }
      else if( name == "VmRSS" )
      {
         sscanf( value.c_str(), "%lu kB", &ps_x->vm_rss );
      }
      else if( name == "VmData" )
      {
         sscanf( value.c_str(), "%lu kB", &ps_x->vm_data );
      }
      else if( name == "VmStk" )
      {
         sscanf( value.c_str(), "%lu kB", &ps_x->vm_stack );
      }
      else if( name == "VmExe" )
      {
         sscanf( value.c_str(), "%lu kB", &ps_x->vm_exe );
      }
      else if( name == "VmLib" )
      {
         sscanf( value.c_str(), "%lu kB", &ps_x->vm_lib );
      }
      else if( name == "SigPnd" )
      {
         sscanf( value.c_str(), "%Lx", &ps_x->sigpnd );
      }
      else if( name == "SigBlk" )
      {
         sscanf( value.c_str(), "%Lx", &ps_x->sigblk );
      }
      else if( name == "SigIgn" )
      {
         sscanf( value.c_str(), "%Lx", &ps_x->sigign );
      }
      else if( name == "SigCgt" )
      {
         sscanf( value.c_str(), "%Lx", &ps_x->sigcgt );
      }
      else if( name == "CapInh" )
      {
         sscanf( value.c_str(), "%016x", &ps_x->cap_inheritable );
      }
      else if( name == "CapPrm" )
      {
         sscanf( value.c_str(), "%016x", &ps_x->cap_permitted );
      }
      else if( name == "CapEff" )
      {
         sscanf( value.c_str(), "%016x", &ps_x->cap_effective );
      }
      else
      {
         write_syslogf("In function proc_load_status, unexpected line: '%s'\n", TRUE, nextLine.c_str());	
      }
   }
   statFile.close();
   return 1;
}

int proc_show_status(UR_OBJECT user, int show_kb)
{

   struct proc_longstatus ps_x;

   if(!proc_load_status(user,&ps_x,getpid())) return 0;
   
   write_seperator_line(user,"status");
   write_userf(user,"~FTName~RS                  : %s\n",ps_x.cmd);
   write_userf(user,"~FTStatus~RS                : %s\n",ps_x.status);
   write_userf(user,"~FTPID~RS                   : %d\n",ps_x.pid);
   write_userf(user,"~FTPPID~RS                  : %d\n",ps_x.ppid);
   write_userf(user,"~FTUID~RS                   : %d\n",ps_x.uid);
   write_userf(user,"~FTEUID~RS                  : %d\n",ps_x.euid);
   write_userf(user,"~FTSUID~RS                  : %d\n",ps_x.suid);
   write_userf(user,"~FTFSUID~RS                 : %d\n",ps_x.fsuid);
   write_userf(user,"~FTGID~RS                   : %d\n",ps_x.gid);
   write_userf(user,"~FTEGID~RS                  : %d\n",ps_x.egid);
   write_userf(user,"~FTSGID~RS                  : %d\n",ps_x.sgid);
   write_userf(user,"~FTFSGID~RS                 : %d\n",ps_x.fsgid);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0)
   write_userf(user,"~FTTGID~RS                  : %d\n",ps_x.tgid);
   write_userf(user,"~FTTRACERPID~RS             : %d\n",ps_x.tpid);
   write_userf(user,"~FTFDSIZE~RS                : %d\n",ps_x.fdsize);
#endif

   write_userf(user,"~FTVM_SIZE~RS               : %lu Kb\n",ps_x.vm_size);
   write_userf(user,"~FTVM_LOCK~RS               : %lu Kb\n",ps_x.vm_lock);
   write_userf(user,"~FTVM_RSS~RS                : %lu Kb\n",ps_x.vm_rss);
   write_userf(user,"~FTVM_DATA~RS               : %lu Kb\n",ps_x.vm_data);
   write_userf(user,"~FTVM_STACK~RS              : %lu Kb\n",ps_x.vm_stack);
   write_userf(user,"~FTVM_EXE~RS                : %lu Kb\n",ps_x.vm_exe);
   write_userf(user,"~FTVM_LIB~RS                : %lu Kb\n",ps_x.vm_lib);

   write_userf(user,"~FTSIGPND~RS                : %.16Lx\n",ps_x.sigpnd);
   write_userf(user,"~FTSIGBLK~RS                : %.16Lx\n",ps_x.sigblk);
   write_userf(user,"~FTSIGIGN~RS                : %.16Lx\n",ps_x.sigign);
   write_userf(user,"~FTSIGCGT~RS                : %.16Lx\n",ps_x.sigcgt);

   write_userf(user,"~FTCapInh~RS                : %016x\n",ps_x.cap_inheritable);
   write_userf(user,"~FTCapPrm~RS                : %016x\n",ps_x.cap_permitted);
   write_userf(user,"~FTCapEff~RS                : %016x\n",ps_x.cap_effective);

   write_seperator_line(user,NULL);
   write_user(user,"\n");
   return 0;
}

int proc_show_compilation(UR_OBJECT user, int show_kb)
{
   struct proc_stat ps;
   struct proc_statm ps_m;
   struct proc_longstatus ps_x;
   struct mallinfo malloc_inf;
   size_t our_page_size = getpagesize();
   

   if(!proc_load_stat(user,&ps,getpid())) return 0;
   if(!proc_load_statm(user,&ps_m,getpid())) return 0;
   if(!proc_load_status(user,&ps_x,getpid())) return 0;
   malloc_inf=mallinfo();
   
   write_seperator_line(user," Process info ");
   write_userf(user,"~FTProcess name~RS : %s   ",ps_x.cmd);
   write_userf(user,"~FTProcess id~RS  : %-8d   ",ps.pid);
   write_userf(user,"~FTUID~RS : %d\n",ps_x.uid);
   write_userf(user,"~FT                     Parent PID~RS  : %-8d   ",ps.ppid);
   write_userf(user,"~FTGID~RS : %d\n",ps_x.gid);
   write_seperator_line(user," mem and timing ");
   if(show_kb)
   {
      write_userf(user,"~FTTotal program size~RS : %8d Kb.     ",(ps_m.size     * our_page_size) >> 10);
         write_userf(user,"~FTProc. time in user  mode~RS : %.2ld:%.2ld:%.2ld.%.2ld\n",ps.utime/360000,(ps.utime%360000)/6000,((ps.utime%360000)%6000)/100,((ps.utime%360000)%6000)%100);
      write_userf(user,"~FTin memory portions~RS : %8d Kb.     ",(ps_m.resident * our_page_size) >> 10);
         write_userf(user,"~FTProc. time in kern. mode~RS : %.2ld:%.2ld:%.2ld.%.2ld\n",ps.stime/360000,(ps.stime%360000)/6000,((ps.stime%360000)%6000)/100,((ps.stime%360000)%6000)%100);
      write_userf(user,"~FTshared pages~RS       : %8d Kb.     ",(ps_m.share    * our_page_size) >> 10);
         write_userf(user,"~FTChld. time in user  mode~RS : %.2ld:%.2ld:%.2ld.%.2ld\n",ps.cutime/360000,(ps.cutime%360000)/6000,((ps.cutime%360000)%6000)/100,((ps.cutime%360000)%6000)%100);
      write_userf(user,"~FTcode pages~RS         : %8d Kb.     ",(ps_m.trs      * our_page_size) >> 10);
         write_userf(user,"~FTChld. time in kern. mode~RS : %.2ld:%.2ld:%.2ld.%.2ld\n",ps.cstime/360000,(ps.cstime%360000)/6000,((ps.cstime%360000)%6000)/100,((ps.cstime%360000)%6000)%100);
      write_userf(user,"~FTdata/stack pages~RS   : %8d Kb.\n",   (ps_m.drs      * our_page_size) >> 10);
      write_userf(user,"~FTdirty pages~RS        : %8d Kb.     ",(ps_m.dt       * our_page_size) >> 10);
         write_userf(user,"~FTSIGALRM timer~RS            : %.2ld:%.2ld:%.2ld.%.2ld\n",ps.itrealvalue/360000,(ps.itrealvalue%360000)/6000,((ps.itrealvalue%360000)%6000)/100,((ps.itrealvalue%360000)%6000)%100);
   }
   else
   {
      write_userf(user,"~FTTotal program size~RS : %8d bytes.  ",ps_m.size     * our_page_size);
         write_userf(user,"~FTProc. time in user  mode~RS : %.2ld:%.2ld:%.2ld.%.2ld\n",ps.utime/360000,(ps.utime%360000)/6000,((ps.utime%360000)%6000)/100,((ps.utime%360000)%6000)%100);
      write_userf(user,"~FTin memory portions~RS : %8d bytes.  ",ps_m.resident * our_page_size);
         write_userf(user,"~FTProc. time in kern. mode~RS : %.2ld:%.2ld:%.2ld.%.2ld\n",ps.stime/360000,(ps.stime%360000)/6000,((ps.stime%360000)%6000)/100,((ps.stime%360000)%6000)%100);
      write_userf(user,"~FTshared pages~RS       : %8d bytes.  ",ps_m.share    * our_page_size);
         write_userf(user,"~FTChld. time in user  mode~RS : %.2ld:%.2ld:%.2ld.%.2ld\n",ps.cutime/360000,(ps.cutime%360000)/6000,((ps.cutime%360000)%6000)/100,((ps.cutime%360000)%6000)%100);
      write_userf(user,"~FTcode pages~RS         : %8d bytes.  ",ps_m.trs      * our_page_size);
         write_userf(user,"~FTChld. time in kern. mode~RS : %.2ld:%.2ld:%.2ld.%.2ld\n",ps.cstime/360000,(ps.cstime%360000)/6000,((ps.cstime%360000)%6000)/100,((ps.cstime%360000)%6000)%100);
      write_userf(user,"~FTdata/stack pages~RS   : %8d bytes.\n",ps_m.drs      * our_page_size);
      write_userf(user,"~FTdirty pages~RS        : %8d bytes.  ",ps_m.dt       * our_page_size);
         write_userf(user,"~FTSIGALRM timer~RS            : %.2ld:%.2ld:%.2ld.%.2ld\n",ps.itrealvalue/360000,(ps.itrealvalue%360000)/6000,((ps.itrealvalue%360000)%6000)/100,((ps.itrealvalue%360000)%6000)%100);
   }
   write_seperator_line(user," virtual mem ");
   write_userf(user,"~FTVM_SIZE~RS  : %6lu Kb      ",ps_x.vm_size);
      write_userf(user,"~FTVM_DATA~RS  : %6lu Kb      ",ps_x.vm_data);
            write_userf(user,"~FTVM_LIB~RS   : %6lu Kb\n",ps_x.vm_lib);
   write_userf(user,"~FTVM_LOCK~RS  : %6lu Kb      ",ps_x.vm_lock);
         write_userf(user,"~FTVM_STACK~RS : %6lu Kb\n",ps_x.vm_stack);
   write_userf(user,"~FTVM_RSS~RS   : %6lu Kb      ",ps_x.vm_rss);
         write_userf(user,"~FTVM_EXE~RS   : %6lu Kb\n",ps_x.vm_exe);
   write_seperator_line(user," malloc ");
   if(show_kb)  write_userf(user, "~FTTotal usage~RS : %10d Kb.     /  %10d Kb.   \n",malloc_inf.uordblks >> 10 ,malloc_inf.arena >> 10);
   else         write_userf(user, "~FTTotal usage~RS : %10d bytes.  /  %10d bytes.\n",malloc_inf.uordblks , malloc_inf.arena);
   write_seperator_line(user,NULL);
   
   write_user(user,"\n");
   return 0;
}

int proc_show_malloc(UR_OBJECT user, int show_kb)
{
   struct mallinfo malloc_inf;
   
   malloc_inf=mallinfo();
   write_seperator_line(user,"malloc");
   if(show_kb)
   {
      write_userf(user,"~FTTotal mem allocated with 'sbrk'~RS                   : %8d Kb.\n",malloc_inf.arena >> 10);
      write_userf(user,"~FTUnused chunks~RS                                     : %8d.\n",malloc_inf.ordblks);
/*      write_userf(user,"~FT-this field unused-~RS                               : %8d Kb.\n",malloc_inf.smblks >> 10);*/
      write_userf(user,"~FTTotal 'mmap' chunks~RS                               : %8d.\n",malloc_inf.hblks);
      write_userf(user,"~FTTotal 'mmap' size~RS                                 : %8d Kb.\n",malloc_inf.hblkhd >> 10);
/*      write_userf(user,"~FT-this field unused-~RS                               : %8d Kb.\n",malloc_inf.usmblks >> 10);*/
/*      write_userf(user,"~FT-this field unused-~RS                               : %8d Kb.\n",malloc_inf.fsmblks >> 10);*/
      write_userf(user,"~FTTotal size of mem used by chunks~RS                  : %8d Kb.\n",malloc_inf.uordblks >> 10);
      write_userf(user,"~FTTotal size of mem used by unused chunks~RS           : %8d Kb.\n",malloc_inf.fordblks >> 10);
      write_userf(user,"~FTSize of topmost, releasable chunk~RS                 : %8d Kb.\n",malloc_inf.keepcost >> 10);
   }
   else
   {
      write_userf(user,"~FTTotal mem allocated with 'sbrk'~RS                   : %8d bytes.\n",malloc_inf.arena);
      write_userf(user,"~FTUnused chunks~RS                                     : %8d.\n",malloc_inf.ordblks);
/*      write_userf(user,"~FT-this field unused-~RS                               : %8d bytes.\n",malloc_inf.smblks);*/
      write_userf(user,"~FTTotal 'mmap' chunks~RS                               : %8d.\n",malloc_inf.hblks);
      write_userf(user,"~FTTotal 'mmap' size~RS                                 : %8d bytes.\n",malloc_inf.hblkhd);
/*      write_userf(user,"~FT-this field unused-~RS                               : %8d bytes.\n",malloc_inf.usmblks);*/
/*      write_userf(user,"~FT-this field unused-~RS                               : %8d bytes.\n",malloc_inf.fsmblks);*/
      write_userf(user,"~FTTotal size of mem used by chunks~RS                  : %8d bytes.\n",malloc_inf.uordblks);
      write_userf(user,"~FTTotal size of mem used by unused chunks~RS           : %8d bytes.\n",malloc_inf.fordblks);
      write_userf(user,"~FTSize of topmost, releasable chunk~RS                 : %8d bytes.\n",malloc_inf.keepcost);
   }
   write_seperator_line(user,NULL);
   
   write_user(user,"\n");
   
   return 0;
}

#endif // __linux__

/*
 *  This is an almost 1:1 spoon of cbserver (Cuban Bar) talker code by, erm.. ,
 *  name is on the other OS ^.^
 **/
int proc_show_fds(UR_OBJECT user)
{
   unsigned int    fd;
   struct  stat    buf;
   char            *type;
   char            atime[30],mtime[30],ctime[30];
   struct  tm      *tmptr;
   struct rlimit maximumFiledescriptors;

   getrlimit (RLIMIT_NOFILE, &maximumFiledescriptors);

   write_seperator_line(user," open filedescriptors ");
   write_userf(user,"%2.2s   %-2.2s %-8.8s %-2.2s %-8.8s %-12.12s %-12.12s %-12.12s\n"
                     ,"fd"
                     ,"T"
                     ,"ino"
                     ,"nl"
                     ,"sz"
                     ,"atime"
                     ,"mtime"
                     ,"ctime");
   write_seperator_line(user,NULL);

   for (fd = 0; fd < maximumFiledescriptors.rlim_cur ; ++fd)
   {
      if (fstat(fd,&buf) < 0)
      {
         continue;
      }

      /**
       * Determine the type of descriptor
       */
      if      (S_ISREG(buf.st_mode))  type = "re";
      else if (S_ISDIR(buf.st_mode))  type = "di";
      else if (S_ISCHR(buf.st_mode))  type = "ch";
      else if (S_ISBLK(buf.st_mode))  type = "bl";
      else if (S_ISFIFO(buf.st_mode)) type = "fi";
#ifdef  S_ISLNK
      else if (S_ISLNK(buf.st_mode))  type = "sy";
#endif
#ifdef  S_ISSOCK
      else if (S_ISSOCK(buf.st_mode)) type = "so";
#endif
      else                            type = "**";

      tmptr = localtime(&buf.st_atime);
      strftime((char *)atime,30,"%b %d %H:%M",tmptr);

      tmptr = localtime(&buf.st_mtime);
      strftime((char *)mtime,30,"%b %d %H:%M",tmptr);

      tmptr = localtime(&buf.st_ctime);
      strftime((char *)ctime,30,"%b %d %H:%M",tmptr);

      write_userf(user,"%2.2d | %-2.2s|%-8ld|%-2d|%-8ld|%s|%s|%s\r\n"
                        ,fd
                        ,type
                        ,buf.st_ino
                        ,buf.st_nlink
                        ,buf.st_size
                        ,atime
                        ,mtime
                        ,ctime);
   }
   write_seperator_line(user,NULL);
   write_user(user,"\n");
   return 0;
}

int room_vars_proc(UR_OBJECT user)
{
   RoomsVector::iterator roomNode;

   write_seperator_line(user,"rooms info");
   for(roomNode=roomsList.begin();roomNode != roomsList.end();roomNode++)
   {
      write_userf(user,"room name       : %s\n",(*roomNode)->name);
      write_userf(user,"label           : %s\n",(*roomNode)->label);
      write_userf(user,"desc len        : %d\n",(*roomNode)->desc.length() );
      write_userf(user,"topic           : %s\n",(*roomNode)->topic);
      write_seperator_line(user,NULL);
   }
   write_user(user,"\n");

   return 0;
}

void c_count_output(UR_OBJECT user);

int proc_status(UR_OBJECT user)
{
   int show_kb=1;

   if(!strcmp(words.word[2],"-b")) show_kb=0;
      
   if(!strcmp(words.word[1],"sockets"))
   {
      WriteUserStream *userStream = WriteUserStream::getInstance();

      *userStream << addUser( user );
      socketInterface.procShow( *userStream );
      *userStream << pod_send;
   }
#ifdef __linux__
   else if(!strcmp(words.word[1],"malloc"))      proc_show_malloc(user, show_kb);
   else if(!strcmp(words.word[1],"mem"))    proc_show_statm(user, show_kb);
   else if(!strcmp(words.word[1],"stat"))   proc_show_stat(user, show_kb);
   else if(!strcmp(words.word[1],"status")) proc_show_status(user, show_kb);
#endif // __linux__
#if defined(MALLOC_STATS) || defined(DEBUG_MALLOC)
   else if(!strcmp(words.word[1],"malloc_int")) dump_meminfo(user);
#endif
#ifdef DEBUG_MALLOC
   else if(!strcmp(words.word[1],"malloc_dump"))dump_malloc_table(user);
#endif /* DEBUG_MALLOC */
   else if(!strcmp(words.word[1],"hist")) hist_info(user);
   else if(!strcmp(words.word[1],"c_count")) c_count_output(user);
   else if(!strcmp(words.word[1],"glob")) proc_show_globvars(user);
   else if(!strcmp(words.word[1],"config")) proc_show_configvars(user);
   else if(!strcmp(words.word[1],"u_var")) user_vars_proc(user);
   else if(!strcmp(words.word[1],"cmds")) cmd_list_proc(user);
   else if(!strcmp(words.word[1],"fds")) proc_show_fds(user);
   else if(!strcmp(words.word[1],"shorts")) cmd_shortcutslist_proc(user);
   else if(!strcmp(words.word[1],"rooms")) room_vars_proc(user);
   else
   {
#ifdef __linux__
      if(!strcmp(words.word[1],"-b")) show_kb=0;
      proc_show_compilation(user, show_kb);
#else
   write_userf(user,"Not on linux, choose an option (cool, it works tho! :)\n");
#endif // __linux__
   }

   return 0;
}
