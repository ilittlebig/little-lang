#ifndef ERROR_H
#define ERROR_H

extern int has_error;

void error_at(char* message, ...);
void fatal_error(char* message, ...);
void warning_at(char* message, ...);

#endif /* ERROR_H */
