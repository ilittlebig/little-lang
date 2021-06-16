#include "parser.h"

token_t* peek_token(parser_t* parser) {
	if (vec_get(&parser->tokens, parser->tokens_parsed) != NULL) {
		parser->head = vec_get(&parser->tokens, parser->tokens_parsed++);
		return parser->head;
	}
}

int main() {
	parser_t* parser = malloc(sizeof(struct parser_t));

	parser->tokens = tokenize("c = 2");
	parser->head = vec_get(&parser->tokens, 0);
	parser->tokens_parsed = 0;

	while (parser->head->type != END_OF_FILE) {
		token_t* token = peek_token(parser);
	}

	free(parser);
	return 0;
}
