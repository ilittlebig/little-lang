#include "parser.h"
#include "file.h"
#include "asm.h"

token_t* peek_token(parser_t* parser) {
	if (vec_get(&parser->tokens, parser->tokens_parsed) != NULL) {
		parser->head = vec_get(&parser->tokens, parser->tokens_parsed);
		return parser->head;
	}
}

void advance_token(parser_t* parser) {
	token_t* token = vec_get(&parser->tokens, parser->tokens_parsed);
	if (token) {
		parser->tokens_parsed++;
	}
}

void advance_token_type(parser_t* parser, token_type_t token_type) {
	token_t* token = vec_get(&parser->tokens, parser->tokens_parsed);
	if (token && token->type == token_type) {
		parser->tokens_parsed++;
	} else {
		printf("[Parser]: Unexpected token '%s' was expecting '%s'\n", token_to_str(token->type), token_to_str(token_type));
	}
}

ast_t* parse_func(parser_t* parser) {
	advance_token_type(parser, FN);

	ast_t* ast = init_ast(AST_FUNCTION);
	ast->name = parse_expr(parser)->name;
	ast->list = parse_args(parser)->list;

	if (peek_token(parser)->type == COLON) {
		advance_token_type(parser, COLON);
		ast->data_type = peek_token(parser)->type;

		advance_token(parser);
		ast->value = parse_block(parser);
	}

	return ast;
}

ast_t* parse_keyword(parser_t* parser) {
	ast_t* ast = init_ast(AST_ASSIGNMENT);
	ast->data_type = peek_token(parser)->type;

	advance_token(parser);

	token_t* token = peek_token(parser);
	ast_t* expr = parse_expr(parser);
	ast->value = expr->value;
	ast->name = expr->name;

	return ast;
}

ast_t* parse_id(parser_t* parser) {
	token_t* token = peek_token(parser);
	char* value = token->value;

	advance_token_type(parser, IDENTIFIER);

	switch(peek_token(parser)->type) {
		case INT_NUMBER:
			ast_t* ast = init_ast(AST_ASSIGNMENT);
			ast->name = value;

			ast_t* expr_value = parse_expr(parser);
			ast->data_type = expr_value->data_type;
			ast->value = expr_value->value;

			return ast;
		default: break;
	}

	ast_t* ast = init_ast(AST_VARIABLE);
	ast->name = value;
	ast->data_type = IDENTIFIER;

	if (peek_token(parser)->type == COLON) {
		advance_token_type(parser, COLON);
		ast->data_type = peek_token(parser)->type;
		advance_token(parser);
	}

	return ast;
}

ast_t* parse_args(parser_t* parser) {
	ast_t* ast = init_ast(AST_COMPOUND);

	advance_token_type(parser, LEFT_PAREN);
	if (peek_token(parser)->type != RIGHT_PAREN) {
		vec_push_back(&ast->list, parse_expr(parser));
	}

	while (peek_token(parser)->type == COMMA) {
		advance_token_type(parser, COMMA);
		vec_push_back(&ast->list, parse_expr(parser));
	}

	advance_token_type(parser, RIGHT_PAREN);

	return ast;
}

ast_t* parse_list(parser_t* parser) {
	ast_t* ast = init_ast(AST_COMPOUND);

	advance_token_type(parser, LEFT_PAREN);
	vec_push_back(&ast->list, parse_expr(parser));

	while (peek_token(parser)->type != RIGHT_PAREN) {
		vec_push_back(&ast->list, parse_expr(parser));
	}

	advance_token_type(parser, RIGHT_PAREN);

	return ast;
}

ast_t* parse_block(parser_t* parser) {
	ast_t* ast = init_ast(AST_BLOCK);
	advance_token_type(parser, LEFT_CURLY);

	while (peek_token(parser)->type != END_OF_FILE && peek_token(parser)->type != RIGHT_CURLY) {
		vec_push_back(&ast->list, parse_expr(parser));
	}

	advance_token_type(parser, RIGHT_CURLY);

	return ast;
}

// TODO: A compound is e.g (return 0) or (int a 50)
ast_t* parse_compound(parser_t* parser) {
	ast_t* ast = init_ast(AST_COMPOUND);
	vec_push_back(&ast->list, parse_expr(parser));
	return ast;
}

ast_t* parse_return(parser_t* parser) {
	advance_token_type(parser, RETURN);

	ast_t* ast = init_ast(AST_RETURN);
	ast->data_type = RETURN;
	ast->value = parse_expr(parser)->value;

	return ast;
}

ast_t* parse_int(parser_t* parser) {
	ast_t* ast = init_ast(AST_INT);
	ast->value = peek_token(parser)->value;
	ast->data_type = INT_NUMBER;

	advance_token_type(parser, INT_NUMBER);

	return ast;
}

ast_t* parse_expr(parser_t* parser) {
	token_t* token = peek_token(parser);
	switch (token->type) {
		case FN: return parse_func(parser);
		case RETURN: return parse_return(parser);
		case INT: return parse_keyword(parser);

		case INT_NUMBER: return parse_int(parser);
		case IDENTIFIER: return parse_id(parser);
		case LEFT_PAREN: return parse_list(parser);
		case END_OF_FILE: { break; }

		default: {
			printf("[Parser]: Unexpected token '%s'\n", token_to_str(token->type));
			exit(1);
		}
	}
}

int main() {
	parser_t* parser = malloc(sizeof(struct parser_t));

	parser->tokens = tokenize(read_file("examples/main.lil"));
	parser->head = vec_get(&parser->tokens, 0);
	parser->tokens_parsed = 0;

	vec_t vec;
	vec_init(&vec, 4);

	while (peek_token(parser)->type != END_OF_FILE) {
		vec_push_back(&vec, parse_expr(parser));
	}

	asm_init(vec);

	free(parser);
	return 0;
}
