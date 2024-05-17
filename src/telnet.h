#ifndef TELNET_H
#define TELNET_H

int sendEchoOn(UR_OBJECT user);
int echo_off(UR_OBJECT user);
int telnet_eor_send(UR_OBJECT user);
int cls(UR_OBJECT user);

#endif /* !TELNET_H */
