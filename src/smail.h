#ifndef SMAIL_H
#define SMAIL_H

int mail_count(UR_OBJECT user);
int rmail(UR_OBJECT user);
int has_unread_mail(UR_OBJECT user);
int smail(UR_OBJECT user, char *inpstr, int done_editing);
int send_mail(UR_OBJECT user, pod_string to, char *ptr, int iscopy);
int send_mail_ex(UR_OBJECT user, char *from, pod_string to, pod_string body, int iscopy);
int dmail(UR_OBJECT user);
int mail_from(UR_OBJECT user);

#endif /* !SMAIL_H */
