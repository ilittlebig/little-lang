#ifndef TYPER_H
#define TYPER_H

#include "parser.h"

static int match_types(token_type_t type1, token_type_t type2);
static void match_function_arguments(obj_t* fn, node_t* var);
void match_function_return_buffer(node_t* return_buffer, obj_t* fn);
static void match_arithmetic_operators(node_t* op);
static void match_variable_type(node_t* var);
void check_body(node_t* node);
void type_check_program(obj_t* globals);

#endif /* TYPER_H */
