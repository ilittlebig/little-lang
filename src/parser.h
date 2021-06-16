#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

typedef struct parser_t {
	vec_t tokens;
	token_t* head;
	int tokens_parsed;
} parser_t;

token_t* peek_token(parser_t* parser);

#endif
