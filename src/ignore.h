#ifndef IGNORE_H
#define IGNORE_H

#include "GlobalVars.h"

#define GET_IGNORE_PIC(user)       \
        ((IntGlobalVar*)user->getVar("ignpic"))
#define GET_IGNORE_ATMOS(user)     \
        ((IntGlobalVar*)user->getVar("ignatmos"))
#define GET_IGNORE_WIZ(user)       \
        ((IntGlobalVar*)user->getVar("ignwiz"))
#define GET_IGNORE_SYS(user)       \
        ((IntGlobalVar*)user->getVar("ignsys"))
#define GET_IGNORE_SHOUT(user)     \
        ((IntGlobalVar*)user->getVar("ignshout"))
#define GET_IGNORE_TELL(user)      \
        ((IntGlobalVar*)user->getVar("igntell"))

void ignore_init();
int toggle_ignore(UR_OBJECT user);
char* ignore_stat(UR_OBJECT user);

extern CommandSet picCmdsSet;

#endif /* !IGNORE_H */
