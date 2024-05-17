#include "general_headers.h"
#include "string_misc.h"
#include "smail.h"
#include "MailCopyToVar.h"

BasicVar* CopyToObjectCreator::getObjectInstance(pod_string name)
{
   return new CopyToVar(name);
};

CopyToVar::CopyToVar(pod_string name) : BasicVar(name,0)
{
   int i;

   for (i=0;i<MAX_COPIES;++i) lines[i][0]='\0';
}

int CopyToVar::init()
{
   int i;

   for (i=0;i<MAX_COPIES;++i) lines[i][0]='\0';
   return 0;
}

void CopyToVar::toXML( xmlTextWriterPtr ptr  )
{
   xmlTextWriterStartElement( ptr, reinterpret_cast<const xmlChar *>("array") );
   for( int i = 0; i < MAX_COPIES; i++)
   {
      if(lines[i][0] != '\0')
      {
         xmlTextWriterStartElement( ptr, reinterpret_cast<const xmlChar *>("val") );
         xmlTextWriterWriteString(ptr, reinterpret_cast<const xmlChar *>( lines[i] ));
         xmlTextWriterEndElement( ptr );
      }
   }
   xmlTextWriterEndElement( ptr );
}

void CopyToVar::fromXML( XmlTextReader * reader )
{
   bool       hasRead       = true;
   bool       inItemElement = false;
   int        ourItemDepth  = -1;
   int        type          = 0;
   int        depth         = 0;
   int        subDepth      = 0;
   pod_string name;
   pod_string itemValue;

   enum
   {
      UNSET,
      LINES_ARRAY,
      LINE_VAL,
      DONE
   };
   int state = UNSET;
   int arrIndex = 0;

   while( hasRead )
   {
      type  = reader->NodeType();
      depth = reader->Depth();
      subDepth = depth - ourItemDepth;
      
      if( type == XML_READER_TYPE_ELEMENT || type == XML_READER_TYPE_END_ELEMENT )
      {
         name = reader->Name();

         if( type == XML_READER_TYPE_ELEMENT )
         {
            if( inItemElement == false )
            {
               if( name == "item" )
               {
                  inItemElement = true;
                  ourItemDepth = depth;
               }
            }
            else if( subDepth == 1 )
            {
               if( name == "array" && state == UNSET )
               {
                  state = LINES_ARRAY;
               }
            }
            else if( subDepth == 2 )
            {
               if( name == "val" && state == LINES_ARRAY )
               {
                  state = LINE_VAL;
               }
            }
         }
         else if( type == XML_READER_TYPE_END_ELEMENT && inItemElement )
         {
            if( depth == ourItemDepth && name == "item" )
            {
               inItemElement = false;
               ourItemDepth = -1;
               return;
            }
            else if( subDepth == 1 )
            {
               if( name == "array" && state == LINES_ARRAY )
               {
                  state = DONE;
               }
            }
            else if( subDepth == 2 )
            {
               if( name == "val" && state == LINE_VAL )
               {
                  state = LINES_ARRAY;
               }
            }
         }
      }
      else if( type == XML_READER_TYPE_TEXT && inItemElement )
      {
         itemValue = reader->Value();

         if( state == LINE_VAL )
         {

            if( arrIndex == MAX_COPIES )
            {
               write_syslogf("*** BOOT error: Copyto var can contain only %i values.\n",FALSE,MAX_COPIES);
            }
            else if( itemValue.length() > USER_NAME_LEN )
            {
               write_syslogf("*** BOOT error: Copyto var strings can only be %i characters.\n",FALSE,USER_NAME_LEN);
            }
            else
            {
               strcpy( lines[arrIndex] ,itemValue.c_str() );
               arrIndex++;
            }
         }
      }
   
      hasRead = reader->Read();
   }
}

pod_string CopyToVar::toText()
{
   int i;
   pod_ostringstream outputStream;

   outputStream.width(22);
   outputStream.setf(std::ios::left);
   outputStream << name.c_str() << " :" << std::endl;
   for(i=0;i < MAX_COPIES;i++)
   {
      if(lines[i][0] != '\0')
      {
         outputStream.width(22);
         outputStream.setf(std::ios::left);
         outputStream << i <<" : \"" << lines[i] << "\"" << std::endl;
      }
   }
   return outputStream.str();
}

int CopyToVar::send(UR_OBJECT user,char* message)
{
   int cnt;

   if (lines[0][0]) write_user(user,"Attempting to send copies of smail...\n");
   for (cnt=0; cnt<MAX_COPIES && lines[cnt][0]; cnt++)
   {
      send_mail(user,lines[cnt],message,1);
      lines[cnt][0]='\0';
   }
   return 0;
}

int CopyToVar::to(UR_OBJECT user,char *inpstr)
{
   FILE *fp;
   int i,cnt;
   char filename[80];

   if (words.word_count>MAX_COPIES+1) write_userf(user,"You can only copy to a maximum of %d people.\n",MAX_COPIES);
   else if (words.word_count > 1)
   {
      write_user(user,"\n");
      for (i=0; i<MAX_COPIES; i++) lines[i][0]='\0';
      cnt=0;
      for (i=1; i<words.word_count; i++)
      {
         if (get_user(words.word[i])==user && user->level<LEV_FOU) write_user(user,"You cannot send yourself a copy.\n");
         else
         {
            words.word[i][0]=toupper(words.word[i][0]);
            sprintf(filename,"%s/%s.D.xml",USERFILES,words.word[i]);
            if (!(fp=fopen(filename,"r"))) 
            {
               write_userf(user,"There is no such user with the name '%s' to copy to.\n",words.word[i]);
            }
            else
            {
               fclose(fp);
               strcpy(lines[cnt],words.word[i]);
               cnt++;
            }
         }
      }
   }

   if(!lines[0][0])
   {
      write_user(user,"You are not sending a copy to anyone.\n");
      return 0;
   }
   write_user(user,"Sending copies of your next smail to...\n");
   for (i=0; i<MAX_COPIES && lines[i][0]; i++)
   {
      write_userf(user,"   %s",lines[i]);
   }
   write_user(user,"\n\n");
   return 0;
}

/* get users which to send copies of smail to */
int nocopies(UR_OBJECT user,char *inpstr)
{
   user_get_copyto(user)->init();
   write_user(user,"Sending no copies of your next smail.\n");

   return 0;
}

int copies_to(UR_OBJECT user,char *inpstr)
{
   user_get_copyto(user)->to(user,inpstr);
   return 0;
}
