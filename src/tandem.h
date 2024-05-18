#ifndef TANDEM_H
#define TANDEM_H

#define MAX_TANDEM_FOLLOWERS   3

#define FOLLOW_MODE_NONE       0
#define FOLLOW_MODE_ASKING     1
#define FOLLOW_MODE_FOLLOWING  2 

struct tandem_list
{
   UR_OBJECT tandem_partner[MAX_TANDEM_FOLLOWERS];
   int follow_count;
};

int follow_break(UR_OBJECT user);
int follow_kill(UR_OBJECT user,int silent);
int follow_accept(UR_OBJECT user);
int follow_ask(UR_OBJECT user, char *inpstr);
int tandem_get_followers(UR_OBJECT user ,struct tandem_list *followers);
int multi_move_user_msg(UR_OBJECT user, RM_OBJECT rm,int mode,struct tandem_list *followers);
int multi_move_user_move(UR_OBJECT user, struct tandem_list *followers);
#endif /* !TANDEM_H */
