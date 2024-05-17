#include "general_headers.h"
#include "xalloc.h"
#include "dyn_str.h"

class dyn_arr_inf
{
   public:
      dyn_arr_inf(pod_string & fileName, StringsArray *p) : filename(fileName) , ptr(p)
      {}
      pod_string filename;
      StringsArray *ptr;
};

typedef std::vector< dyn_arr_inf, pod_alloc< dyn_arr_inf >::Type > DynArrInfVector;

DynArrInfVector dyn_arr_inf_list;

int dyn_arr_file_add(pod_string & filename, StringsArray *ptr)
{
   dyn_arr_inf_list.push_back( dyn_arr_inf( filename, ptr ) );
   return 0;
}

void load_str_array( pod_string & filename, StringsArray * ptr)
{
   PodFile inputFile(filename);
   pod_string newLine;

   if( inputFile.open_read() != 0 )
   {
      write_syslogf("ERROR: Fatal opening file(%s).\n", FALSE, filename.c_str() );
      abort();
   }

   newLine = inputFile.fgets();
   while( !inputFile.eof() && newLine.size() )
   {
      newLine.resize( newLine.size() - 1 );
      ptr->push_back(newLine);
      newLine = inputFile.fgets();
   }
   inputFile.close();
}

int load_dyn_arrs()
{
   unsigned int arrays = dyn_arr_inf_list.size();
   unsigned int cnt;

   for( cnt = 0; cnt < arrays; cnt++ )
   {
      load_str_array( dyn_arr_inf_list[cnt].filename , dyn_arr_inf_list[cnt].ptr );
   }
   return 0;
}

int dyn_arrs_reload(UR_OBJECT user)
{
   unsigned int arrays = dyn_arr_inf_list.size();
   unsigned int cnt;

   for( cnt = 0; cnt < arrays; cnt++ )
   {
      dyn_arr_inf_list[cnt].ptr->clear();
      load_str_array( dyn_arr_inf_list[cnt].filename , dyn_arr_inf_list[cnt].ptr );
   }

   write_user(user,"Sucessfully reloaded dynamic arrays\n");

   return 0;
}

