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

	ast_t* expr = parse_expr(parser);
	ast->value = expr;
	ast->name = expr->name;

	return ast;
}

ast_t* parse_id(parser_t* parser) {
	token_t* token = peek_token(parser);
	char* value = token->value;

	advance_token_type(parser, IDENTIFIER);

	if (peek_token(parser)->type == ASSIGN) {
		advance_token_type(parser, ASSIGN);

		ast_t* ast = init_ast(AST_ASSIGNMENT);
		ast->name = value;

		ast_t* expr_value = parse_expr(parser);
		ast->data_type = expr_value->data_type;
		ast->value = expr_value;

		advance_token_type(parser, SEMICOLON);
		return ast;
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
	ast_t* ast = init_ast(AST_COMPOUND);
	advance_token_type(parser, LEFT_CURLY);

	while (peek_token(parser)->type != END_OF_FILE && peek_token(parser)->type != RIGHT_CURLY) {
		vec_push_back(&ast->list, parse_expr(parser));
	}

	advance_token_type(parser, RIGHT_CURLY);

	return ast;
}

ast_t* parse_compound(parser_t* parser) {
	ast_t* ast = init_ast(AST_COMPOUND);
	advance_token_type(parser, LEFT_PAREN);

	while (peek_token(parser)->type != END_OF_FILE && peek_token(parser)->type != RIGHT_PAREN) {
		vec_push_back(&ast->list, parse_expr(parser));
	}

	advance_token_type(parser, RIGHT_PAREN);
	return ast;
}

ast_t* parse_return(parser_t* parser) {
	advance_token_type(parser, RETURN);

	ast_t* ast = init_ast(AST_RETURN);
	ast->value = parse_compound(parser);

	advance_token_type(parser, SEMICOLON);

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

char* read_file(const char* filename) {
	FILE* file;
	char* line = NULL;
	size_t len = 0;
	ssize_t read;

	file = fopen(filename, "rb");
	if (file == NULL) {
		printf("Could not read file '%s'\n", filename);
		exit(1);
	}

	char* buffer = (char*)calloc(1, sizeof(char));
	buffer[0] = '\0';

	while ((read = getline(&line, &len, file)) != -1) {
		buffer = (char*)realloc(buffer, (strlen(buffer) + strlen(line) + 1) * sizeof(char));
		strcat(buffer, line);
	}

	fclose(file);
	if (line) {
		free(line);
	}

	return buffer;
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

	free(parser);
	return 0;
}
