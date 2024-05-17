#ifndef LOGIN_H
#define LOGIN_H

enum { NO_LOGIN = 0,
       LOGIN_NEWBIE_8,   
       LOGIN_NEWBIE_7,   
       LOGIN_NEWBIE_6,   
       LOGIN_NEWBIE_5,   
       LOGIN_NEWBIE_4,   
       LOGIN_NEWBIE_3_2, 
       LOGIN_NEWBIE_3,   
       LOGIN_NEWBIE_2,
       LOGIN_NEWBIE_1,
       LOGIN_CHECK_NAME,
       LOGIN_CHECK_PW    };

int login(UR_OBJECT user, char *inpstr);

#endif /* !LOGIN_H */
