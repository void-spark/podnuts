#include <iostream>
#include "pod_string.h"
#include "logging.h"
#include "libxml_glue.h"
#include "StringLibrary.h"


StringTag::StringTag( unsigned int pos, pod_string name ) : pos( pos ), name( name )
{
}

StringTag::~StringTag()
{
}
      
unsigned int StringTag::getPos()
{
   return pos;
}

pod_string   StringTag::getName()
{
   return name;
}

String::String( pod_string name ) : name( name )
{
}

pod_string       String::getName()
{
   return name;
}

pod_string       String::getString()
{
   return string;
}

StringTagsVector String::getTagsVector()
{
   return tagsVector;
}

void String::appendString( pod_string string )
{
   this->string += string;
}

void String::appendTag( pod_string name )
{
   StringTag *stringTag = new StringTag( string.size(), name );
   tagsVector.push_back( stringTag );
}

Tag::Tag( pod_string name ) : name ( name )
{
}

Tag::~Tag()
{
}

pod_string Tag::getName()
{
   return name;
}

TextTag::TextTag( pod_string name, pod_string text ) : Tag( name ), text( text )
{
}
      
TextTag::~TextTag()
{
}

pod_string TextTag::getString()
{
   return text;
}

bool StringLibrary::instanceFlag = false;

StringLibrary* StringLibrary::single = 0;

StringLibrary::StringLibrary()
{
}

StringLibrary* StringLibrary::getInstance()
{
    if( !instanceFlag )
    {
        single = new StringLibrary();
        instanceFlag = true;
        return single;
    }
    else
    {
        return single;
    }
}

StringLibrary::~StringLibrary()
{
   instanceFlag = false;
}

void StringLibrary::addFile( pod_string filename )
{
   files.push_back( filename );
}

void StringLibrary::addTag( Tag* tag )
{   
   TagsMap::iterator iter = tags.find( tag->getName() );
   
   if( iter != tags.end() )
   {
      Tag* oldTag = (*iter).second;
      tags.erase( iter );
      delete oldTag;
   }

   tags.insert( TagsMap::value_type( tag->getName(), tag ) );
}

void StringLibrary::addTextTag( pod_string name, pod_string text )
{
   addTag( new TextTag( name, text ) );
}

void StringLibrary::addString( String* string )
{
   StringsMap::iterator iter = strings.find( string->getName() );
   
   if( iter != strings.end() )
   {
      String* oldString = (*iter).second;
      strings.erase( iter );
      delete oldString;
   }

   strings.insert( StringsMap::value_type( string->getName(), string ) );
}

pod_string StringLibrary::makeString( pod_string name )
{
   pod_string result;
   StringsMap::iterator iter = strings.find( name );

   if( iter == strings.end() )
   {
      std::cerr << "Error: could not find string with name: " << name << std::endl;
      return result;
   }
   
   String* stringObj = (*iter).second;

   result = stringObj->getString();
   
   StringTagsVector tagsVector = stringObj->getTagsVector();
   
   StringTagsVector::iterator stringTagsIter;
      
   unsigned int grown = 0;
   StringTag *stringTag = 0;
   
   for( stringTagsIter  = tagsVector.begin(); 
        stringTagsIter != tagsVector.end(); 
        stringTagsIter++ )
   {
      stringTag = (*stringTagsIter);
      
      pod_string   tagName = stringTag->getName();
      unsigned int tagPos  = stringTag->getPos();

      TagsMap::iterator tagsIter = tags.find( tagName );

      if( tagsIter == tags.end() )
      {
         std::cerr << "Error: could not find tag with name: " << tagName << std::endl;
         return result;
      }
      
      pod_string tagString = tagsIter->second->getString();  
             
      result.insert( tagPos + grown, tagString);
      
      grown += tagString.size();
   }
   
   return result;
}

void StringLibrary::parseFile( pod_string filename )
{
   bool inGlobalsElement = false;
   bool inItemElement = false;

   bool hasRead = false;
   
   pod_string itemName;
   pod_string itemValue;

   String *string = 0;
      
   XmlTextReader *reader = 0;
   
   try
   {
      reader = new XmlTextReader( filename );   
   }
   catch (XmlError &e)
   {
      std::cerr << "Failed to construct XmlTextReader for file: " << filename << std::endl;
      abort();
   }
   
   hasRead = reader->Read();
   while( hasRead )
   {
      int type = reader->NodeType();
      if( type == XML_READER_TYPE_ELEMENT || type == XML_READER_TYPE_END_ELEMENT )
      {
         pod_string name = reader->Name();

         if( type == XML_READER_TYPE_ELEMENT )
         {
            if( name == "globalstrings" )
            {
               inGlobalsElement = true;
            }
            else if( name == "item" &&  inGlobalsElement == true )
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
               
               string = new String( itemName );
               
               inItemElement = true;
            }
            else if( inItemElement == true )
            {
               string->appendTag( name );
            }
         }
         else if( type == XML_READER_TYPE_END_ELEMENT )
         {
            if( name == "globalstrings" && inGlobalsElement == true )
            {
               inGlobalsElement = false;
            }
            if( name == "item" && inItemElement == true )
            {
               addString( string );
               string = 0;
               
               inItemElement = false;
            }
         }

      }
      else if( type == XML_READER_TYPE_TEXT ||
               type == XML_READER_TYPE_SIGNIFICANT_WHITESPACE )
      {
         if( inItemElement == true )
         {
            string->appendString( reader->Value() );            
         }
      }
      
      hasRead = reader->Read();
   }
   
   delete reader;
}

void StringLibrary::parseAllFiles()
{
   StringsVector::iterator iter;

   for( iter = files.begin(); iter != files.end(); iter++ )
   {
      parseFile( iter->c_str() );
   }
}

StringsVector StringLibrary::getFileNames( )
{
   return files;
}


