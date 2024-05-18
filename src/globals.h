#ifndef GLOBALS_H
#define GLOBALS_H

enum {  BY_LOAD_CONFIG,
        FIRST_BOOT,
        EVERY_BOOT }; 

int init_glob_list();
int init_glob_firstboot_list();
void glob_add_var_cust( BasicVar *newVar );
void globVarsToXML( xmlTextWriterPtr ptr);
void globVarsFromXML( XmlTextReader * reader );
int proc_show_globvars(UR_OBJECT user);
#endif /* !GLOBALS_H */
