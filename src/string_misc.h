#ifndef STRING_MISC_H
#define STRING_MISC_H

bool isNonPrinting( char c );
bool isNonPrintingOrWS( char c );
bool isBackOrDel( char c );

pod_string gen_seperator_line(pod_string string);
void strToUpper(pod_string &str);
void strToLower(pod_string &str);
void capitalize(pod_string &str);
int strtolower(char *str);
int isNumber(char *str);
pod_string intToString(int i);
int stringToInt( pod_string str );
pod_string floatToString(float f);
float stringToFloat( pod_string str);
pod_string time2string(int show_seconds,time_t my_time);
const char *const_remove_first(const char *inpstr);
char *remove_first(char *inpstr);
pod_string remove_first( pod_string input );
int terminate(pod_string &str);
pod_string color_com_strip( pod_string str );
int is_alpha_str(char *str);
pod_string longDateFromTime( time_t timeValue );
pod_string wordWrap( const pod_string input, int width);
pod_string long_date(int which);
int contains_swearing(pod_string str);
int strxcmp(const char *s1, const char *s2);
pod_string str_ansi( pod_string str );

#endif /* !STRING_MISC */

