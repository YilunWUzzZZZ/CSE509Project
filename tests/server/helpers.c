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
  char *file_list[] = { "server", "ld.so.preload", "libc.so.6" ,"ld.so.cache" };
  char *filename = basename(pathname);
  
if (find_in(filename, file_list,4))
  {
		return 1;
  }
	
	return 0;
}

int safetoread_addr(struct sockaddr_in *addr)
{
	if ((addr->sin_family == 0x2) && (addr->sin_port == 0x1F90) && (addr->sin_addr.s_addr == htonl("127.0.0.1")))
		return 1;
	
	return 0;	
}
