#include "helpers.h"

int find_in(char *my_str, char *string_list[], size_t num_strings)
{
  
    for ( int i = 0; i < num_strings; i++ )
        if (strcmp(my_str, string_list[i]) == 0 )
            return 1;
    
    return 0;
}

int safetoread_path(char *pathname)
{
  
  char *file_list[] = { "vuln", "ld.so.preload", "libc.so.6" ,"ld.so.cache" };
  char *filename = basename(pathname);
  
	if (find_in(filename, file_list,4))
  {
		return 1;
  }
	
	return 0;
}