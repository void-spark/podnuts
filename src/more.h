#ifndef MORE_H
#define MORE_H

int parse_stuff(std::ostream & outputStream, UR_OBJECT user,std::istream  & inputStream,int is_more);
void misc_more(UR_OBJECT user, const char *inpstr);
int more(UR_OBJECT user, PlainSocket *socket,  const pod_string filename);
int more_ext(UR_OBJECT user, PlainSocket *socket, const pod_string filename, int prompt);

#endif /* !MORE_H */
