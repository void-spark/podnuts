#include <vector>
#include <string>

#include "general_headers.h"
#include "xalloc.h"
#include "string_misc.h"
#include "file_io.h"
#include "GlobalVars.h"
#include "globals.h"

typedef std::vector<BasicVar*, pod_alloc< BasicVar* >::Type > globVarsVector;

globVarsVector globVars;

void glob_add_var_cust( BasicVar *newVar )
{
   globVars.push_back(newVar);
}

int gen_init_glob_list(int which)
{
   globVarsVector::iterator globVarNode;
   
   for(globVarNode=globVars.begin();globVarNode!=globVars.end();globVarNode++)
   {
      if( (*globVarNode)->getFlags() == which)
      {
         (*globVarNode)->init();
      }
   }   
   
  return 0;
}

int init_glob_list()
{
   return gen_init_glob_list(EVERY_BOOT);
}

int init_glob_firstboot_list()
{
   return gen_init_glob_list(FIRST_BOOT);
}

void globVarsToXML( xmlTextWriterPtr ptr)
{
   globVarsVector::iterator globVarNode;

   for(globVarNode=globVars.begin();globVarNode!=globVars.end();globVarNode++)
   {
      pod_string name = (*globVarNode)->getName();

      xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("   "));

      xmlTextWriterStartElement( ptr, reinterpret_cast<const xmlChar *>("item") );
      xmlTextWriterWriteAttribute( ptr, reinterpret_cast<const xmlChar*>("name"), reinterpret_cast<const xmlChar *>( name.c_str() ) );
      (*globVarNode)->toXML( ptr );
      xmlTextWriterEndElement( ptr );
      xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("\n"));
   }

}

void globVarsFromXML( XmlTextReader * reader )
{
   bool hasRead          = true;
   bool inGlobalsElement = false;
   globVarsVector::iterator globVarNode;
   pod_string name;
   pod_string itemName;

   while( hasRead )
   {
      int type = reader->NodeType();
      if( type == XML_READER_TYPE_ELEMENT || type == XML_READER_TYPE_END_ELEMENT )
      {
         name = reader->Name();

         if( type == XML_READER_TYPE_ELEMENT )
         {
            if( name == "globals" )
            {
               inGlobalsElement = true;
            }
            else if( inGlobalsElement == true && name == "item" )
            {               
               try
               {
                  itemName = reader->GetAttribute( "name" );
               }
               catch (NoSuchAttribute &e)
               {
                  std::cerr << "item with no name!" << std::endl;
                  abort();
               }

               for(globVarNode=globVars.begin();globVarNode!=globVars.end();globVarNode++)
               {
                  if( !strcmp(itemName.c_str(),(*globVarNode)->getName() )) break;
               }
               if(globVarNode==globVars.end())
               {
                  std::cerr << "No global var for itemName '" << itemName << "'!" << std::endl;
               }

               (*globVarNode)->fromXML( reader );
            }
         }
         else if( type == XML_READER_TYPE_END_ELEMENT )
         {
            if( name == "globals" && inGlobalsElement == true )
            {
               inGlobalsElement = false;
               return;
            }
            else if( name == "item" && inGlobalsElement == true)
            {
            }
         }
      }

      hasRead = reader->Read();
   }

}

int proc_show_globvars(UR_OBJECT user)
{
   globVarsVector::iterator globVarNode;

   write_seperator_line(user,"globvars list");

   for(globVarNode=globVars.begin();globVarNode!=globVars.end();globVarNode++)
   {
      const pod_string &test =(*globVarNode)->toText();
      write_user(user,test.c_str());
   }

   write_seperator_line(user,NULL);
   write_user(user,"\n");

   return 0;
}
