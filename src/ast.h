#ifndef AST_H
#define AST_H

#include "lexer.h"

typedef enum ast_type_t {
	AST_IDENTIFIER,
	AST_INT,
	AST_CALL,
	AST_STRING,
	AST_COMPOUND,
	AST_RETURN,
	AST_DEFVAR,
	AST_FUNCALL,
	AST_ASSIGNMENT,
	AST_FUNCTION,
	AST_BLOCK
} ast_type_t;

typedef struct ast_t {
	char* name;
	char* label;
	int offset;
	int arg_offset; // TODO: hacky solution for passing args as variables?

	void* value;
	void* body;
	void* next;
	void* vars;
	void* params;

	vec_t list;
	ast_type_t type;
	token_type_t type_specifier;
} ast_t;

char* ast_type_to_str(ast_type_t);
ast_t* init_ast(ast_type_t);

#endif
