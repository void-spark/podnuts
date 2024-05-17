#ifndef STRINGLIBRARY_H
#define STRINGLIBRARY_H

#include <vector>
#include <map>
#include <string>

class StringTag
{
   public:
      StringTag( unsigned int pos, pod_string name );
      ~StringTag();
      
      unsigned int getPos();
      pod_string   getName();
      
   protected:
      unsigned int pos;
      pod_string   name;
};

typedef std::vector< StringTag*, pod_alloc< StringTag* >::Type > StringTagsVector;

class String
{
   public:
      String( pod_string name );
      pod_string       getName();
      pod_string       getString();
      StringTagsVector getTagsVector();
      void appendString( pod_string string );
      void appendTag( pod_string name );

   protected:
      pod_string       name;
      pod_string       string;
      StringTagsVector tagsVector;      
};

class Tag
{
   public:
      Tag( pod_string name );
      virtual ~Tag();
      virtual pod_string getString() = 0;
      pod_string   getName();

   protected:
      pod_string name;
};

class TextTag : public Tag
{
   public:
      TextTag( pod_string name, pod_string text );
      ~TextTag();
      pod_string getString();

   protected:
      pod_string text;
};

typedef std::map<pod_string, String*, std::less<pod_string>, pod_alloc< std::pair<pod_string, String* > >::Type > StringsMap;
typedef std::map<pod_string, Tag*,    std::less<pod_string>, pod_alloc< std::pair<pod_string, Tag*    > >::Type > TagsMap;
typedef std::vector<pod_string, pod_alloc< pod_string >::Type > StringsVector;

class StringLibrary
{
   private:
      static bool instanceFlag;
      static StringLibrary *single;
      StringLibrary();

   public:
      static StringLibrary* getInstance();
      ~StringLibrary();

   protected:
      StringsMap    strings;
      TagsMap       tags;
      StringsVector files;
      
   public:
      void parseFile( pod_string filename );
      void addFile( pod_string filename );
      StringsVector getFileNames( );
      void parseAllFiles();
      
      void addTag( Tag* tag );
      void addTextTag( pod_string name, pod_string text );
      void addString( String* string );
      pod_string makeString( pod_string name );
      
      StringsVector getFiles( );
};

#endif /* !STRINGLIBRARY_H */

