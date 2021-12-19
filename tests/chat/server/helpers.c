#include "helpers.h"
#define LOG_FILE "safex_debug.log"
int find_in(char *my_str, char *string_list[], size_t num_strings)
{
    for ( int i = 0; i < num_strings; i++ )
        if (strcmp(my_str, string_list[i]) == 0 )
            return 1;

    return 0;
}

int safetoread_path(char *pathname)
{
  char *file_list[] = { "./server", "/etc/ld.so.preload", "/lib/x86_64-linux-gnu/libc.so.6" ,"/etc/ld.so.cache" };

  
if (find_in(pathname, file_list,4))
  {
		return 1;
  }
	
	return 0;
}

int safetoread_addr(struct sockaddr_in *addr)
{

	if ((addr->sin_family == 0x2) && (addr->sin_port == htons(0x1F90)) && (addr->sin_addr.s_addr == inet_addr("127.0.0.1")))
		return 1;
	
	return 0;	
}

int safetoreada_addr(struct sockaddr_in *addr)
{

	if (((addr->sin_family == 0x2)  && (addr->sin_addr.s_addr == inet_addr("127.0.0.1"))) || ((addr->sin_family == 0)  && (addr->sin_addr.s_addr == 0)))
		return 1;
	
	return 0;	
}
