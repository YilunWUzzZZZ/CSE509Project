#include <stdio.h>
int main() {
   // printf() displays the string inside quotation
      FILE* log_fd = fopen("123.txt", "ab+");
	fprintf(log_fd, " Opening file \n");
	fclose(log_fd);
  printf("Hello, World!\n");

   return 0;
}
