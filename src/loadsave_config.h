#ifndef LOADSAVE_CONFIG_H
#define LOADSAVE_CONFIG_H

typedef std::vector<pod_string,  pod_alloc< pod_string >::Type > StringsVector;
extern StringsVector pluginStringsVector;

int load_and_parse_config(char *confile);
void checkConfig();
void addConfigVar( BasicVar *newVar );
int proc_show_configvars(UR_OBJECT user);

#endif /* !LOADSAVE_CONFIG_H */

