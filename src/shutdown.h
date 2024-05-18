#ifndef SHUTDOWN_H
#define SHUTDOWN_H

#include "GlobalVars.h"

extern jmp_buf jmpvar;
extern CrashActionGlobalVar crash_action;

extern StrGlobalVar rsUserName;
int talker_exit(int shutdown);
void talker_shutdown(const char *who_did_it_str, int reboot);
void sig_handler(int sig);
void misc_shutdown(UR_OBJECT user, const char *inpstr);
void misc_reboot(UR_OBJECT user, const char *inpstr);
int shutdown_com(UR_OBJECT user);
int reboot_com(UR_OBJECT user);
int check_reboot_shutdown();
char *crash_action_txt();

#endif /* !SHUTDOWN_H */

