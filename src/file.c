#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "file.h"

void write_file(const char* buff, const char* filename) {
	FILE* file;

	file = fopen(filename, "w");
	if (!file) {
		printf("Could not read file '%s'\n", filename);
		exit(1);
	}

	fprintf(file, "%s", buff);
}

char* read_file(const char* filename) {
	FILE* file;
	char* line = NULL;
	size_t len = 0;

	file = fopen(filename, "rb");
	if (!file) {
		printf("Could not read file '%s'\n", filename);
		exit(1);
	}

	char* buffer = (char*)calloc(1, sizeof(char));
	buffer[0] = '\0';

	while (getline(&line, &len, file) != -1) {
		buffer = (char*)realloc(buffer, (strlen(buffer) + strlen(line) + 1) * sizeof(char));
		strcat(buffer, line);
	}

	fclose(file);
	if (line) {
		free(line);
	}

	return buffer;
}
