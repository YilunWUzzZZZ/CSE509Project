#include "helpers.h"
#define LOG_FILE "debug.log"


int find_in(char *pathname)
{
  FILE* log_fd = fopen(LOG_FILE, "ab+");
  fprintf(log_fd, " %s: my_str =%s \n", __FUNCTION__, pathname);

  char *file_list[] = { "vuln", "ld.so.preload", "libc.so.6" ,"ld.so.cache", "locale-archive", "123.txt" };
  char* filename = basename(pathname);
  for ( int i = 0; i < 6; i++ )
      if (strcmp(filename, file_list[i]) == 0 ) {
          
          fprintf(log_fd, "%s: Found match for my_str = %s \n", __FUNCTION__,  filename);
          fclose(log_fd);
          return 1;
      }

  fprintf(log_fd, "%s: No match for my_str = %s \n", __FUNCTION__, filename);
  fclose(log_fd);
  return 0;

}

char* safetoread_path(char *pathname)
{
	return pathname;
}
