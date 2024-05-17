#ifndef GENDER_H
#define GENDER_H

enum { SINGULAR_MALE = 0, 
       SINGULAR_FEMALE, 
       PLURAL }; // target
       
enum { SUBJECT_PRONOUN = 0, 
       OBJECT_PRONOUN, 
       POSSESSIVE_ADJECTIVE,
       POSSESSIVE_PRONOUN }; // type

char *getPronoun( int target, int type );
char *get_gender(UR_OBJECT user, pod_string noun_type);

#endif /* !GENDER_H */
