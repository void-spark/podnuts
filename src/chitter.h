#ifndef CHITTER_H
#define CHITTER_H

int say_gen(UR_OBJECT user, const char *inpstr);
int say_to (UR_OBJECT user, const char *inpstr);
int wake   (UR_OBJECT user, const char *inpstr);
int shout  (UR_OBJECT user, const char *inpstr);
int tell   (UR_OBJECT user, const char *inpstr);
int whisper(UR_OBJECT user, const char *inpstr);
int wtell  (UR_OBJECT user, const char *inpstr);
int wemote (UR_OBJECT user, const char *inpstr);
int wsto   (UR_OBJECT user, const char *inpstr);
int emote  (UR_OBJECT user, const char *inpstr);
int semote (UR_OBJECT user, const char *inpstr);
int pemote (UR_OBJECT user, const char *inpstr);
int echo   (UR_OBJECT user, const char *inpstr);
int show   (UR_OBJECT user, const char *inpstr);

#endif /* !CHITTER_H */

