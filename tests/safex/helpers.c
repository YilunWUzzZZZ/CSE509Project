#include "helpers.h"

#define LOG_FILE "safex_debug.log"



int file_is_in(char *my_str, char *string_list[], size_t num_strings)
{
    for ( int i = 0; i < num_strings; i++ )
        if (strcmp(my_str, string_list[i]) == 0 )
            return TRUE;

    return FALSE;
}


int protected_file(char *pathname) {

	// printf("Checking if file is in important list\n");
	char *filename = basename(pathname);
	char *no_read_files[] = {"important.txt", "important2.txt", "important3.txt", "important4.txt"};
	if (file_is_in(filename, no_read_files, 4)) {
		// printf("File found in important files, make tmp copy");
		return TRUE;
	}
	return FALSE;
}


int copy_contents_to_temp_file( char* fname_from, int fd_to ) {

	FILE* log_fd = fopen(LOG_FILE, "ab+");
	fprintf(log_fd, " %s: Opening file %s\n", __FUNCTION__, fname_from);
	int fd_from = open(fname_from, O_RDONLY);
	if (fd_from == -1) {
		fprintf(log_fd, "Cannot open file\n");
		return FALSE;
	}

	char buf[BUFSIZ];

	int n = read(fd_from, buf, sizeof(buf));		// Read data from file into buffer
	
	write(fd_to, buf, n);							// Write the data to temp file from buffer

	fprintf(log_fd, "%s: Contents copied from file %s \n", __FUNCTION__, fname_from);

	close(fd_from);
	fclose(log_fd);
	
	return TRUE;
}


char* safex(char* pathname)
{

	FILE* log_fd = fopen(LOG_FILE, "ab+");


	char* fname_from;
	fname_from = malloc(strlen(pathname));
	strcpy(fname_from, pathname);
	fprintf(log_fd, "%s: Filename passed %s\n", __FUNCTION__, pathname);

	static char template[] = "/tmp/tmpfXXXXXX";			// mkstemp filename template
	char* fname_to;
	fname_to = malloc(strlen(template));

	int fd;
	char buf[BUFSIZ];
	int n;

	strcpy(fname_to, template);		
	fd = mkstemp(fname_to);			/* Create and open temp file */
	//fprintf(stderr, "Filename is %s\n", fname_to);	

	if (copy_contents_to_temp_file(fname_from, fd) == FALSE) {
		fprintf(log_fd, "%s: Creation of temp file failed \n", __FUNCTION__);
		exit(0);
	}

	close(fd);					/* Close file */
	// unlink(fname);				/* Remove it */

	fclose(log_fd);
	return fname_to;
}