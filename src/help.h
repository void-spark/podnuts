#ifndef HELP_H
#define HELP_H

#define CREDITSFILE   "credits" 
#define MAINHELPFILE  "main_help_file"
#define MAINFAQFILE   "mainfaq"
#define MAINRANKSFILE "mainranks"

#define HELP_INDEX     0
#define HELP_LIST      1
#define HELP_COMMANDS  2

enum comgrpvals { MSG, 
                  ROOM, 
                  GEN, 
                  SPCH, 
                  SYS, 
                  INF, 
                  ADM, 
                  FUN, 
                  BOTY, 
                  CLO,

                  CAR, /* last three are games */
                  FIG,
                  BOA };

int ranks_faq(UR_OBJECT user, int faq);
int help(UR_OBJECT user);
int help_credits(UR_OBJECT user);
#endif /* !HELP_H */
