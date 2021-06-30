#ifndef AST_H
#define AST_H

#include "lexer.h"
#include <stdlib.h>

typedef enum ast_type_t {
	AST_VARIABLE,
	AST_COMPOUND,
	AST_ASSIGNMENT,
	AST_FUNCTION
} ast_type_t;

typedef struct ast_t {
	char* name;
	void* value;
	vec_t list;
	token_type_t type;
} ast_t;

ast_t* init_ast(ast_type_t) {
	ast_t* ast = malloc(sizeof(struct ast_t));
	vec_init(&ast->list, 4);
	return ast;
}

#endif
