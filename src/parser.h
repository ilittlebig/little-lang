#ifndef PARSER_H
#define PARSER_H

#include "ast.h"

typedef struct parser_t {
	vec_t tokens;
	token_t* head;
	int tokens_parsed;
} parser_t;

token_t* peek_token(parser_t*);
void advance_token(parser_t* parser);
void advance_token_type(parser_t* parser, token_type_t token_type);

ast_t* parse_func(parser_t*);
ast_t* parse_keyword(parser_t*);
ast_t* parse_id(parser_t*);
ast_t* parse_arguments(parser_t*);
ast_t* parse_args(parser_t*);
ast_t* parse_list(parser_t*);
ast_t* parse_block(parser_t*);
ast_t* parse_compound(parser_t*);
ast_t* parse_return(parser_t*);
ast_t* parse_int(parser_t*);
ast_t* parse_expr(parser_t*);
char* read_file(const char*);

#endif
