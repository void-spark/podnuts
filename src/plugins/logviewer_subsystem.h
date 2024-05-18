#ifndef LOGVIEWER_SUBSYSTEM_H
#define LOGVIEWER_SUBSYSTEM_H

class log_type
{
   public:
      char* show_name_lwr;
      char* show_name;
      char* filename;

      log_type() : show_name_lwr(0), show_name(0), filename(0) { }
      log_type(const log_type& copyFrom) : show_name_lwr(copyFrom.show_name_lwr), show_name(copyFrom.show_name), filename(copyFrom.filename) { }
      log_type(char *showNameLwr, char *showName, char *fileName) : show_name_lwr(showNameLwr), show_name(showName), filename(fileName) { }
};

int showLogfileLines(UR_OBJECT user, log_type & logInfo, unsigned int lines);
int showLogfile(UR_OBJECT user, log_type & logInfo);

#endif /* !LOGVIEWER_SUBSYSTEM_H */
