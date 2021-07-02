#include "ast.h"

ast_t* init_ast(ast_type_t ast_type) {
	ast_t* ast = malloc(sizeof(struct ast_t));
	ast->type = ast_type;
	ast->value = 0;

	vec_init(&ast->list, 4);
	return ast;
}
