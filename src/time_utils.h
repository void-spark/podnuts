#ifndef TIME_UTILS_H
#define TIME_UTILS_H

struct     tm *get_current_localtime();
struct     tm *get_current_utctime();
long timeval_diff_usec(struct timeval *a,struct timeval *b);

#endif /* !TIME_UTILS_H */
