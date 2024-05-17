#ifndef GLOBALVARS_H
#define GLOBALVARS_H
enum {  NC = 0 };

#include <string>
#include "pod_string.h"

// #include "general_headers.h"
#include "file_io.h"

#include "simple_types/vars.h"
#include "simple_types/IntGlobalVar.h"
#include "simple_types/SelectGlobalVar.h"
#include "simple_types/CrashActionGlobalVar.h"
#include "simple_types/DayTimeGlobalVar.h"
#include "simple_types/FltArrayGlobalVar.h"
#include "simple_types/IntArrayGlobalVar.h"
#include "simple_types/LevelGlobalVar.h"
#include "simple_types/LevelOrNoneGlobalVar.h"
#include "simple_types/LimitedIntGlobalVar.h"
#include "simple_types/NewOldGlobalVar.h"
#include "simple_types/OnOffGlobalVar.h"
#include "simple_types/strArrGlobVar.h"
#include "simple_types/StrGlobalVar.h"
#include "simple_types/TimeTGlobalVar.h"
#include "simple_types/YesNoGlobalVar.h"

typedef struct user_struct* UR_OBJECT;

#include "simple_types/UserGlobalVar.h"
#endif /* !GLOBALVARS_H */
