#include "helpers.h"

#define LOG_FILE "debug.log"

int is_url(char* pathname) {

    
    char filename[BUFSIZ];
    strcpy(filename, basename(pathname));
    if (strncmp(filename, "http://", 7) == 0 || strncmp(filename, "https://", 8) == 0 || strncmp(filename, "www", 3) == 0) {
        
        #ifdef LOGGING
        FILE* log_fd = fopen(LOG_FILE, "ab+");
        fprintf(log_fd, "%s: url detected %s\n", __FUNCTION__, pathname);
        fclose(log_fd);
        #endif

        return TRUE;
    }

    return FALSE;
}

char* url2file(char* pathname) {

    #ifdef LOGGING
    FILE* log_fd = fopen(LOG_FILE, "ab+");
    fprintf(log_fd, "%s called with arg %s\n", __FUNCTION__, pathname);
    #endif

    char url[BUFSIZ];
    strcpy(url, basename(pathname));

    if (strncmp(url, "http://", 7) == 0 || strncmp(url, "https://", 8) == 0 || strncmp(url, "www", 3) == 0) {
        if (file_created == 0) {
            // wget the file and return the filename
            system("rm -rf /tmp/url2file; mkdir /tmp/url2file");
            char wget_str[] = "wget -q -O /tmp/url2file/file.tmp ";
            strcat(wget_str, url);
            system(wget_str);
            file_created = TRUE;
            system("chmod 666 /tmp/url2file/file.tmp");
            strcpy(pathname, "/tmp/url2file/file.tmp");
            
            #ifdef LOGGING
            fprintf(log_fd, "%s: downloaded file for first time.\n", __FUNCTION__);
            fclose(log_fd);
            #endif
            
            return pathname;

        }

        strcpy(pathname, "/tmp/url2file/file.tmp");
        
        #ifdef LOGGING
        fprintf(log_fd, "%s: File already downloaded, return pathname = %s\n", __FUNCTION__, pathname);
        fclose(log_fd);
        #endif

        return pathname;
    }

    #ifdef LOGGING
    fclose(log_fd);
    #endif

    return NULL;
}