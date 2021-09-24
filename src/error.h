#ifndef ERROR_H
#define ERROR_H

#include "parser.h"

void go_error_at(location_t* location, char* message, ...);
void go_warning_at(location_t* location, char* message, ...);

#endif /* ERROR_H */
