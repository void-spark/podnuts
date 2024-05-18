#ifndef MACRO_H
#define MACRO_H

#include <map>

#define SYSMACROFILE "sysmacros"   /* name of system macros file */
#define SYSACTIONFILE "sysactions"

#define MINLEVELACTIONS LEV_TWO

class Macro
{
   public:
      pod_string getValue();
      void setValue( pod_string value );

      Macro() : is_running(false)
      {
      }
      int expand( UR_OBJECT user );

   protected: // Protected methods
      pod_string value; /* the replacement text of the macro */
      bool is_running;  /* is the macro running or not (boolean flag) */
      int star;         /* the starting word # for a $* expansion */

      /** store the starting place of a $* expansion, for speed */
      int calcStar();
};

typedef std::map<pod_string, Macro, std::less<pod_string>, pod_alloc< std::pair<pod_string, Macro> >::Type > MacroList;

class MacroListGlobalVar
{
   enum result {SUCCESS,FAIL};

   protected:
      MacroList macros;
      Macro* isMacro(const char *command);
      int addMacro(pod_string &name,pod_string &value);
      int updateMacro(pod_string &name,pod_string &value);
      int deleteMacro(pod_string &name);

   public:
      int macroExpand(UR_OBJECT user);
      void cmd(char *input,UR_OBJECT user,bool is_action);
      int loadMacros(char *filename);
      int saveMacros(char *filename);
      void list_macros(UR_OBJECT user);
};

extern MacroListGlobalVar system_macrolist;
extern MacroListGlobalVar system_actionlist;

/* function prototypes */

void list_action_macros(UR_OBJECT user);
#endif /* !MACRO_H */

