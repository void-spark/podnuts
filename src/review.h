#ifndef REVIEW_H
#define REVIEW_H

#ifndef PODNUTS_H
typedef struct user_struct* UR_OBJECT;
typedef struct Room* RM_OBJECT;
#endif

void revbuffs_init();

int record_tell(UR_OBJECT user,char *str);
int record(RM_OBJECT rm, char *str);
int wrecord(char *str);
int record_shout(char *str);

int clear_revbuff(RM_OBJECT rm);

#endif /* !REVIEW_H */
