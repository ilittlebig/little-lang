#ifndef AST_H
#define AST_H

#include "lexer.h"

typedef enum ast_type_t {
	AST_VARIABLE,
	AST_INT,
	AST_CALL,
	AST_STRING,
	AST_COMPOUND,
	AST_RETURN,
	AST_ASSIGNMENT,
	AST_FUNCTION,
	AST_BLOCK
} ast_type_t;

typedef struct ast_t {
	char* name;
	void* value;
	vec_t list;
	ast_type_t type;
	token_type_t data_type;
} ast_t;

char* ast_type_to_str(ast_type_t);
ast_t* init_ast(ast_type_t);

#endif
