#include "parser.h"

token_t* peek_token(parser_t* parser) {
	if (vec_get(&parser->tokens, parser->tokens_parsed) != NULL) {
		parser->head = vec_get(&parser->tokens, parser->tokens_parsed);
		return parser->head;
	}
}

void advance_token(parser_t* parser) {
	if (vec_get(&parser->tokens, parser->tokens_parsed) != NULL) {
		parser->tokens_parsed++;
	}
}

int main() {
	parser_t* parser = malloc(sizeof(struct parser_t));

	parser->tokens = tokenize("int c = 54");
	parser->head = vec_get(&parser->tokens, 0);
	parser->tokens_parsed = 0;

	while (parser->head->type != END_OF_FILE) {
		token_t* token = peek_token(parser);
		advance_token(parser);

		if (token->type == INT) {
			printf("int keyword\n");
		} else if (token->type == IDENTIFIER) {
			printf("identifier, value: %s\n", token->value);
		} else if (token->type == STRING_LITERAL) {
			printf("string, value: %s\n", token->value);
		} else if (token->type == ASSIGN) {
			printf("assign\n");
		} else if (token->type == INT_NUMBER) {
			printf("int number, value: %i\n", token->value);
		}
	}

	free(parser);
	return 0;
}
