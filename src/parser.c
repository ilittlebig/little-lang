#include "parser.h"

token_t* peek_token(parser_t* parser) {
	if (vec_get(&parser->tokens, parser->tokens_parsed) != NULL) {
		parser->head = vec_get(&parser->tokens, parser->tokens_parsed);
		return parser->head;
	}
}

void advance_token(parser_t* parser) {
	token_t* token = vec_get(&parser->tokens, parser->tokens_parsed);
	if (token != NULL) {
		parser->tokens_parsed++;
	}
}

void advance_token_type(parser_t* parser, token_type_t token_type) {
	token_t* token = vec_get(&parser->tokens, parser->tokens_parsed);
	if (token != NULL && token->type == token_type) {
		parser->tokens_parsed++;
	} else {
		printf("[Parser]: Unexpected token '%s' was expecting '%s'\n", token_to_str(token->type), token_to_str(token_type));
	}
}

ast_t* parse_fn(parser_t* parser) {
	advance_token_type(parser, FN);
	token_t* token = peek_token(parser);

	if (token->type == IDENTIFIER) {
		ast_t* ast = init_ast(AST_FUNCTION);
		ast->name = token->value;
		return ast;
	}
}

ast_t* parse_id(parser_t* parser) {
	token_t* token = peek_token(parser);
	char* value = token->value;

	advance_token_type(parser, IDENTIFIER);
	token = peek_token(parser);

	if (token->type == ASSIGN) {
		ast_t* ast = init_ast(AST_VARIABLE);
		ast->name = value;
		ast->value = parse_expr(parser);
		return ast;
	}

	ast_t* ast = init_ast(AST_VARIABLE);
	ast->name = value;

	if (token->type == COLON) {
		advance_token_type(parser, COLON);
		token = peek_token(parser);
		ast->type = token->type;

		advance_token(parser);
	}

	return ast;
}

ast_t* parse_arguments(parser_t* parser) {
	ast_t* compound = init_ast(AST_COMPOUND);

	advance_token_type(parser, LEFT_PAREN);
	vec_push_back(&compound->list, parse_expr(parser));

	while (peek_token(parser)->type == COMMA) {
		advance_token_type(parser, COMMA);
		vec_push_back(&compound->list, parse_expr(parser));
	}

	advance_token_type(parser, RIGHT_PAREN);

	return compound;
}

ast_t* parse_expr(parser_t* parser) {
	token_t* token = peek_token(parser);
	switch (token->type) {
		case FN: return parse_fn(parser);
		case IDENTIFIER: return parse_id(parser);
		case LEFT_PAREN: return parse_arguments(parser);
		default: {
			printf("[Parser]: Unexpected token '%s'\n", token_to_str(token->type));
			exit(1);
		}
	}
}

int main() {
	parser_t* parser = malloc(sizeof(struct parser_t));

	parser->tokens = tokenize("fn main(argc: char, argv: char, hello: char) : int { }");
	parser->head = vec_get(&parser->tokens, 0);
	parser->tokens_parsed = 0;

	while (parser->head->type != END_OF_FILE) {
		parse_expr(parser);
	}

	free(parser);
	return 0;
}
