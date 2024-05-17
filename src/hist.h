#ifndef HIST_H
#define HIST_H

extern IntGlobalVar hist_last_minute;

extern IntGlobalVar usrcnt_hist_curr;
extern FltArrayGlobalVar usrcnt_hist;
extern FltArrayGlobalVar histw_1day;

extern IntGlobalVar hist_last_daypiece;
extern FltArrayGlobalVar hist_7days;
extern FltArrayGlobalVar histw_7day;



void hist_init();
int hist_update();
int hist_info(UR_OBJECT user);

#endif /* !HIST_H */

