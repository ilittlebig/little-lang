/*
#include <stdarg.h>
#include "error.h"

void go_error_at(location_t* location, char* message, ...) {
	printf("\033[1;37m");
	printf("%s:%i: ", location->path, location->line);
	printf("\033[0m");

	va_list args;
	va_start(args, message);
	printf("\033[1;31m");
	printf("error: ");
	printf("\033[0m");
	vprintf(message, args);
	va_end(args);
	printf("\n");
}

void fatal_error(char* message, ...) {
	printf("\033[1;37m");
	printf("little: ");
	printf("\033[0m");

	va_list args;
	va_start(args, message);
	printf("\033[1;31m");
	printf("fatal error: ");
	printf("\033[0m");
	vprintf(message, args);
	va_end(args);
	printf("\n");
}

void go_warning_at(location_t* location, char* message, ...) {
	printf("\033[1;37m");
	printf("%s:%i: ", location->src, location->line);
	printf("\033[0m");

	va_list args;
	va_start(args, message);
	printf("\033[1;34m");
	printf("error: ");
	printf("\033[0m");
	vprintf(message, args);
	va_end(args);
	printf("\n");
}
*/
