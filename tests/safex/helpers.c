#include "helpers.h"


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

	//printf("Opening file %s \n", fname1);
	int fd_from = open(fname_from, O_RDONLY);
	if (fd_from == -1) {
		printf("Cannot open file %s \n", fname_from);
		return FALSE;
	}

	char buf[BUFSIZ];

	int n = read(fd_from, buf, sizeof(buf));		// Read data from file into buffer
	
	write(fd_to, buf, n);							// Write the data to temp file from buffer

	//printf("\n Contents copied from file %s \n", fname1);

	close(fd_from);
	return TRUE;
}


char* safex(char* pathname)
{

	char* fname_from;
	fname_from = malloc(strlen(pathname));
	strcpy(fname_from, pathname);
	//printf("Filename passed %s\n", fname1);

	static char template[] = "./tmp/tmpfXXXXXX";			// mkstemp filename template
	char* fname_to;
	fname_to = malloc(strlen(template));

	int fd;
	char buf[BUFSIZ];
	int n;

	strcpy(fname_to, template);		
	fd = mkstemp(fname_to);			/* Create and open temp file */
	//printf("Filename is %s\n", fname);	

	if (copy_contents_to_temp_file(fname_from, fd) == FALSE) {
		printf("Creation of temp file failed \n");
		exit(0);
	}

	
	//lseek(fd, 0L, SEEK_SET);			// Rewind to front 
	//n = read(fd, buf, sizeof(buf));		// Read data back; NOT '\0' terminated! 
	//printf("Got back: %.*s", n, buf);	// Print it out for verification 
	

	close(fd);					/* Close file */
	// unlink(fname);				/* Remove it */

	return fname_to;
}
