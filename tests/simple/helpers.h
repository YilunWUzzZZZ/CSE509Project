#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <limits.h>
#include <fcntl.h>
#include <libgen.h>

int find_in(char *pathname);
char* safetoread_path(char *pathname);


