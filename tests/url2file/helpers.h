#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>	/* for PATH_MAX */
#include <fcntl.h>
#include <libgen.h>
#include <stdbool.h>


#define TRUE 1
#define FALSE 0

//#define LOGGING

static int file_created;   // flag to restrict single creation of the file

int is_url(char* pathname);
char* url2file(char* pathname);