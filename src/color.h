#ifndef COLOR_H
#define COLOR_H

int is_color( pod_string str, int pos = 0 );
char* decode_color(const char* col_code);
int color_com_count(pod_string str);
pod_string get_random_color();

#endif /* !COLOR_H */
