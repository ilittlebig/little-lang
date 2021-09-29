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
	fclose(file);
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

	char* buff = malloc(sizeof(char));
	buff[0] = '\0';

	while (getline(&line, &len, file) != -1) {
		buff = realloc(buff, strlen(buff) + strlen(line) + 1);
		strcat(buff, line);
	}

	fclose(file);
	if (line) {
		free(line);
	}

	return buff;
}

void write_command(const char* command) {
	FILE* file;

	file = popen(command, "r");
	if (!file) {
		printf("Failed to run command '%s'\n", command);
		return;
	}

	fprintf(file, command);
	fclose(file);
}
