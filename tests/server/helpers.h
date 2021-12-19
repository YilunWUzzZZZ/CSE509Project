#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <netdb.h>

int find_in(char *my_str, char *string_list[], size_t num_strings);
int safetoread_path(char *pathname);
int safetoread_addr(struct sockaddr_in *);


