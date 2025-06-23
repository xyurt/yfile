#include "yfile.h"

int main() {
	FILE *fp;							   // File pointer							
	const char *filename = "example.txt";  // File name
	char buffer[] = "Hello, World!"; 	   // Buffer which holds the content

	fp = file_open(filename, "r+b");	   // The file is opened with 'r+b' mode
	file_write(fp, buffer, strlen(buffer));// The content is being overwritten

	file_close(fp);						   // File is closed
	return 0;
}