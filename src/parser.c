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
		ast->data_type = token->type;

		advance_token(parser);
	}

	return ast;
}

ast_t* parse_list(parser_t* parser) {
	ast_t* ast = init_ast(AST_COMPOUND);

	advance_token_type(parser, LEFT_PAREN);
	vec_push_back(&ast->list, parse_expr(parser));

	while (peek_token(parser)->type == COMMA) {
		advance_token_type(parser, COMMA);
		vec_push_back(&ast->list, parse_expr(parser));
	}

	advance_token_type(parser, RIGHT_PAREN);

	if (peek_token(parser)->type == COLON) {
		advance_token_type(parser, COLON);
		ast->type = AST_FUNCTION;
		ast->data_type = peek_token(parser)->type;

		advance_token(parser);
		ast->value = parse_block(parser);
	}

	return ast;
}

ast_t* parse_block(parser_t* parser) {
	ast_t* ast = init_ast(AST_COMPOUND);
	advance_token_type(parser, LEFT_CURLY);

	while (peek_token(parser)->type != END_OF_FILE && peek_token(parser)->type != RIGHT_CURLY) {
		vec_push_back(&ast->list, parse_expr(parser));

		if (peek_token(parser)->type == SEMICOLON) {
			advance_token_type(parser, SEMICOLON);
		}
	}

	advance_token_type(parser, RIGHT_CURLY);
}

ast_t* parse_return(parser_t* parser) {
	ast_t* ast = init_ast(AST_COMPOUND);
	advance_token_type(parser, RETURN);
	advance_token_type(parser, LEFT_PAREN);

	while (peek_token(parser)->type != END_OF_FILE && peek_token(parser)->type != RIGHT_PAREN) {
		vec_push_back(&ast->list, parse_expr(parser));
	}

	advance_token_type(parser, RIGHT_PAREN);
}

ast_t* parse_expr(parser_t* parser) {
	token_t* token = peek_token(parser);
	switch (token->type) {
		case FN: { advance_token_type(parser, FN); };
		case IDENTIFIER: return parse_id(parser);
		case RETURN: return parse_return(parser);
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

	parser->tokens = tokenize("fn main(argc: int, argv: char, hello: char) : int { return (func a b); }");
	parser->head = vec_get(&parser->tokens, 0);
	parser->tokens_parsed = 0;

	while (parser->head->type != END_OF_FILE) {
		parse_expr(parser);
	}

	free(parser);
	return 0;
}
