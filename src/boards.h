#ifndef BOARDS_H
#define BOARDS_H

int news_count(pod_string board_name);
int write_board(UR_OBJECT user, char *inpstr, int done_editing);
int read_board(UR_OBJECT user);
int wipe_board(UR_OBJECT user,char *inpstr);
int search_boards(UR_OBJECT user);
int check_messages(UR_OBJECT user, int force);

#endif /* !BOARDS_H */
