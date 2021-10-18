#include <stdarg.h>
#include <stdio.h>

#include "error.h"

int has_error = 0;
// TODO: Show error with filename and location.
void error_at(token_t* token, char* message, ...) {
	printf("\033[1;37m");
	printf("examples/main.lil:%d: ", token->line_no);
	printf("\033[0m");

	va_list args;
	va_start(args, message);
	printf("\033[1;31m");
	printf("error: ");
	printf("\033[0m");
	vprintf(message, args);
	va_end(args);

	printf("\n");

	has_error = 1;
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

	has_error = 1;
}

// TODO: Show error with filename and location.
void warning_at(token_t* token, char* message, ...) {
	printf("\033[1;37m");
	printf("examples/main.lil:%d: ", token->line_no);
	printf("\033[0m");

	va_list args;
	va_start(args, message);
	printf("\033[1;34m");
	printf("warning: ");
	printf("\033[0m");
	vprintf(message, args);
	va_end(args);

	printf("\n");
}
