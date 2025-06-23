#include "../yfile.h"

int main() {
	FILE *fp;							   // File pointer							
	const char *filename = "example.txt";  // File name
	char buffer[1024];					   // Buffer to read the contents

	fp = file_open(filename, "r");		   // The file is opened with 'r' read-only mode
	file_read(fp, buffer, sizeof(buffer)); // File is being read into buffer

	file_close(fp);						   // File is closed
	return 0;
}