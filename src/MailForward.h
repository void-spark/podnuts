#ifndef MAILFORWARD_H
#define MAILFORWARD_H

int verify_email(UR_OBJECT user);
int set_autofwd(UR_OBJECT user, char *inpstr);
int set_fmail(UR_OBJECT user, char *inpstr);
int forward_email(char *name, MailMessage *message );

#endif /* !MAILFORWARD_H */

