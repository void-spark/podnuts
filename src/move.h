#ifndef MOVE_H
#define MOVE_H

#define GO_MODE_NORMAL_GO      0  /* move to adjecent room on own account */
#define GO_MODE_MAGICAL_VORTEX 1  /* move to a non adjecent room on own account */
#define GO_MODE_GIANT_FLIPPER  2  /* get forcefully moved to another room */
#define GO_MODE_THROWN         3  /* get forcefully thrown to another room */
#define GO_MODE_SILENT         4  /* move without msg or check */

int move_user(UR_OBJECT user, RM_OBJECT rm, int teleport);
int go(UR_OBJECT user);
int move(UR_OBJECT user);
int throw_user(UR_OBJECT user);
int bring(UR_OBJECT user);
int join(UR_OBJECT u);
int back(UR_OBJECT user);
int invite(UR_OBJECT user);
int letmein(UR_OBJECT user);

#endif /* !MOVE_H */
