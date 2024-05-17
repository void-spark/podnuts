#ifndef ADMIN_H
#define ADMIN_H
int no_delold(UR_OBJECT user);
int pop_level(UR_OBJECT user, char *inpstr);
int log_user(UR_OBJECT user,char* logger,char *str);
int view_user_log(UR_OBJECT user,UR_OBJECT u);
int log_user_com(UR_OBJECT user, char *inpstr);
int force(UR_OBJECT user, char *inpstr);
int list_users(UR_OBJECT user);
int unjail_user(UR_OBJECT user, char *inpstr);
int jail_user(UR_OBJECT user, char *inpstr);
int wizon(UR_OBJECT user);
int delold_users(UR_OBJECT user);
int swban(UR_OBJECT user);
int muzzle(UR_OBJECT user);
int unmuzzle(UR_OBJECT user);
int promote(UR_OBJECT user);
int demote(UR_OBJECT user);
int untag_user(UR_OBJECT user,char* inpstr);
int tag_user(UR_OBJECT user,char* inpstr);
#endif /* !ADMIN_H */
