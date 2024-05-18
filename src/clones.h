#ifndef CLONES_H
#define CLONES_H

int is_clone(UR_OBJECT user);
int reset_clone_socks(UR_OBJECT old_user,UR_OBJECT new_user);
int clones_count();
int destroy_user_clones(UR_OBJECT user,int silent);
int create_clone(UR_OBJECT user);
int destroy_clone(UR_OBJECT user);
int myclones(UR_OBJECT user);
int allclones(UR_OBJECT user);
int clone_switch(UR_OBJECT user);
int clone_say(UR_OBJECT user, char *inpstr);
int clone_emote(UR_OBJECT user, char *inpstr);
int clone_hear(UR_OBJECT user);


#endif /* !CLONES_H */
