#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <limits.h>
#include <fcntl.h>
#include <libgen.h>

int find_in(char *my_str, char *string_list[], size_t num_strings);
int safetoread_path(char *pathname);


