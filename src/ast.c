#include "ast.h"

char* ast_type_to_str(ast_type_t ast_type) {
	switch (ast_type) {
		case AST_IDENTIFIER: return "AST_IDENTIFIER";
		case AST_INT: return "AST_INT";
		case AST_STRING: return "AST_STRING";
		case AST_CALL: return "AST_CALL";
		case AST_COMPOUND: return "AST_COMPOUND";
		case AST_KEYWORD: return "AST_KEYWORD";
		case AST_ASSIGNMENT: return "AST_ASSIGNMENT";
		case AST_FUNCTION: return "AST_FUNCTION";
		case AST_BLOCK: return "AST_BLOCK";
	}
}

ast_t* init_ast(ast_type_t ast_type) {
	ast_t* ast = malloc(sizeof(struct ast_t));
	ast->type = ast_type;
	ast->value = 0;
	ast->label = NULL;

	return ast;
}
