#ifndef PARSER_H
#define PARSER_H

#include "ast.h"

typedef struct location_t {
	int line;
	char* src;
	char* path;
} location_t;

typedef struct parser_t {
	vec_t tokens;
	token_t* head;
	location_t* location;
	int tokens_parsed;
} parser_t;

static ast_t* locals;
static ast_t* current_func;

token_t* peek_token(parser_t* parser);
void advance_token(parser_t* parser);
void advance_token_type(parser_t* parser, token_type_t token_type);
ast_t* find_var(const char* varname);

ast_t* parse_func(parser_t* parser);
void parse_params(parser_t* parser);
ast_t* parse_body(parser_t* parser);
void push_args(parser_t* parser, ast_t* compound);
ast_t* parse_return(parser_t* parser);
ast_t* parse_defvar(parser_t* parser);
ast_t* parse_identifier(parser_t* parser);
ast_t* parse_string(parser_t* parser);
ast_t* parse_int(parser_t* parser);
ast_t* parse_compound(parser_t* parser);
ast_t* parse_expr(parser_t* parser);
void parse_src(char* path, char* src);

#endif /* PARSER_H */
