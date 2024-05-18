#ifndef BANNING_H
#define BANNING_H

#define SITEBAN      "siteban"
#define USERBAN      "userban"

bool   site_banned(pod_string site);
int   user_banned(char *name);
int listbans(UR_OBJECT user);
int unban(UR_OBJECT user, char *inpstr);
int unban_user(UR_OBJECT user, char *reason);
int unban_site(UR_OBJECT user);
int ban(UR_OBJECT user,char* inpstr);
int ban_user(UR_OBJECT user, char *reason);
int ban_site(UR_OBJECT user);

#endif /* !BANNING_H */

