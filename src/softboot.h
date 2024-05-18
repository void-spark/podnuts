#ifndef SOFTBOOT_H
#define SOFTBOOT_H

// #define SOFTBOOT_DEBUG

#define SYSTMPINFO         "temp.sysinfo.reboot.xml"
#define USRTMPINFO         "temp.usersinfo.reboot.xml"
#define ROOMTMPINFO        "temp.roomsinfo.reboot.xml"
#define SOFT_BOOT_PID_FILE "softboot"

#define FUNC_NAME_LEN      40

extern TimeTGlobalVar firstBootTime;
extern IntGlobalVar   total_logins;

int  softboot_restart();
void softboot_shutdown();
void show_total_uptime(UR_OBJECT user);
void set_user_link(struct user_struct **target, pod_string name );


#endif
