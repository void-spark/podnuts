#include "general_headers.h"
#include "./simple_types/vars.h"
#include "file_io.h"
#include "string_misc.h"
#include "dynamicVarsController.h"

BasicVar *DynamicVarsController::getptr( BasicVar** & data_ptr, VarTempl *node )
{
   return data_ptr[node->index];
}

void DynamicVarsController::setptr( BasicVar** & data_ptr, VarTempl *node, BasicVar *newptr )
{
   data_ptr[node->index] = newptr;
}

int DynamicVarsController::addVarTempl( pod_string name, ObjectCreator *var, int save_type )
{
   varsTemplMap[name].save_type   = save_type;
   varsTemplMap[name].creator     = var;
   varsTemplMap[name].index       = varsTemplMap.size() - 1;

   return 0;
}

int DynamicVarsController::createObj(BasicVar** & data_ptr)
{
   VarsTemplMap::iterator iterator;

   data_ptr = new (BasicVar*)[varsTemplMap.size()];
   if ( !data_ptr )
   {
      write_syslog("ERROR: Memory allocation failure in  DynamicVarsController::createObj().\n",0);
      return 0;
   }
   memset( data_ptr, 0, varsTemplMap.size() * sizeof(BasicVar*) );

   for ( iterator = varsTemplMap.begin(); iterator != varsTemplMap.end(); iterator++ )
   {
      const pod_string & name =   (*iterator).first;
      VarTempl   * node = &((*iterator).second);

      BasicVar *newVar = node->creator->getObjectInstance( name );
      setptr( data_ptr, node, newVar );
   }

   return 0;
}

int DynamicVarsController::initObj(BasicVar** & data_ptr)
{
   VarsTemplMap::iterator iterator;

   for( iterator = varsTemplMap.begin(); iterator != varsTemplMap.end(); iterator++ )
   {
      VarTempl *node = &((*iterator).second);

      BasicVar *curVar = getptr( data_ptr, node );
      curVar->init();
   }

   return 0;
}

int DynamicVarsController::deleteObj(BasicVar** & data_ptr)
{
   VarsTemplMap::iterator iterator;

   for ( iterator = varsTemplMap.begin(); iterator != varsTemplMap.end(); iterator++ )
   {
      VarTempl *node = &((*iterator).second);

      delete getptr( data_ptr, node );
      setptr( data_ptr, node, 0 );
   }

   delete [] data_ptr;

   return 0;
}

VarTempl *DynamicVarsController::getObj( BasicVar** & data_ptr, pod_string name )
{
   VarTempl *node = 0;

   if ( varsTemplMap.count(name) )
   {
      node = &(varsTemplMap[name]);
   }

   return node;
}

void DynamicVarsController::toXML( BasicVar** & data_ptr, int save_type, xmlTextWriterPtr ptr )
{
   VarsTemplMap::iterator iterator;

   for ( iterator = varsTemplMap.begin(); iterator != varsTemplMap.end(); iterator++ )
   {
      VarTempl *node = &((*iterator).second);
      pod_string name = (*iterator).first;

      if(node->save_type != save_type) 
      {
         continue;
      }
      xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("   "));

      xmlTextWriterStartElement( ptr, reinterpret_cast<const xmlChar *>("item") );
      xmlTextWriterWriteAttribute( ptr, reinterpret_cast<const xmlChar*>("name"), reinterpret_cast<const xmlChar *>( name.c_str() ) );
      getptr( data_ptr, node )->toXML( ptr );
      xmlTextWriterEndElement( ptr );
      xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>("\n"));
   }
}
 
void DynamicVarsController::fromXML( XmlTextReader * reader, BasicVar** & data_ptr  )
{
   int        itemDepth = reader->Depth();
   bool       hasRead   = true;
   VarTempl * node      = 0;
   int        type      = 0;
   int        depth     = 0;
   pod_string name;
   pod_string itemName;

   while( hasRead )
   {
      type  = reader->NodeType();
      depth = reader->Depth();
      
      if( type == XML_READER_TYPE_ELEMENT || type == XML_READER_TYPE_END_ELEMENT )
      {
         name = reader->Name();

         if( type == XML_READER_TYPE_ELEMENT )
         {
            if( name == "item" && depth == itemDepth )
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
                           
               node = getObj( data_ptr, itemName );
               if(node == 0)
               {
                  std::cerr << "Node = 0 for itemName '" << itemName << "'!" << std::endl;
                  VarsTemplMap::iterator iterator;

                  std::cerr << "List of possible names:" << std::endl;
                  for ( iterator = varsTemplMap.begin(); iterator != varsTemplMap.end(); iterator++ )
                  {
                     const pod_string & name =   (*iterator).first;
                     std::cerr << " - '" << name << "'" << std::endl;
                  }
                  abort();
               }
               getptr( data_ptr, node )->fromXML( reader );
               continue;
               #warning quick fix above here
            }
         }
         #warning , de fromXML hierboven parst het sluitende item element al, we lezen het volgende element in ergens \
         hier beneden, en dat is dus niet de sluitende item element, dus we returnen niet.
         else if( type == XML_READER_TYPE_END_ELEMENT )
         {
            if( name == "item" && depth == itemDepth )
            {
               node = 0;
               return;
            }
         }
      }

      hasRead = reader->Read();
   }

}

BasicVar* DynamicVarsController::objGetVar( BasicVar** & data_ptr, pod_string name)
{
   VarsTemplMap::iterator iterator;

   iterator = varsTemplMap.find( name );
   if ( iterator != varsTemplMap.end() )
   {
      VarTempl *node = &((*iterator).second);

      return getptr( data_ptr, node );
   }

   return (BasicVar*)0;
}

int DynamicVarsController::objPrintVar( BasicVar** & data_ptr, UR_OBJECT user )
{
   VarsTemplMap::iterator iterator;

   for ( iterator = varsTemplMap.begin(); iterator != varsTemplMap.end(); iterator++ )
   {
      write_user(user,getptr(data_ptr,&((*iterator).second))->toText().c_str());
   }

   return 0;
}

