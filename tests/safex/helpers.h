#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>	/* for PATH_MAX */
#include <fcntl.h>
#include <libgen.h>

#define TRUE 1
#define FALSE 0

int protected_file(char *pathname);
char* safex(char* pathname);