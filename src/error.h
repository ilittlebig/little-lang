#ifndef ERROR_H
#define ERROR_H

#include "lexer.h"

extern int has_error;

void error_at(token_t* token, char* message, ...);
void fatal_error(char* message, ...);
void warning_at(token_t* token, char* message, ...);

#endif /* ERROR_H */
