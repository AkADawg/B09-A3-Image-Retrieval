#include <stdio.h>
#include <string.h>

#define LINE_LENGTH 80
int read_image(char *filename)
{
	printf("this2");
	FILE *file;
	int error;
	char line[LINE_LENGTH + 1];
	printf("this3");
	file = fopen(, "r");
	printf("hello");

	if (file == NULL) {
		fprintf(stderr, "Error opening file\n");
		return 0;
	}

	while (fgets(line, LINE_LENGTH + 1, file) != NULL) {
		printf("%s", line);
	}

	error = fclose(file);
	if (error != 0) {
		fprintf(stderr, "fclose failed\n");
		return 0;
	}
	return 1;
}

/* Reads and prints the contents of a file (top10.txt). */
int main() {
	printf("this");
	read_image("top10.txt");
	return 0;
}