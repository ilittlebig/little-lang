#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include "vec.h"

typedef enum token_type_t {
	// Reserved Keywords
	FN,
	IF,
	WHILE,
	FOR,
	ELSE,
	INT,
	FLOAT,
	STRING,
	VOID,
	RETURN,
	DEFVAR,
	LIST,
	NTH,

	NUMBER,
	IDENTIFIER,

	GREATER_OR_EQUAL,
	EQUAL,
	LESS_OR_EQUAL,
	NOT_EQUAL,

	LEFT_PAREN,
	RIGHT_PAREN,
	LEFT_BRACKET,
	RIGHT_BRACKET,
	LEFT_CURLY,
	RIGHT_CURLY,

	ADD,
	SUB,
	MUL,
	DIV,
	MOD,
	POW,

	ASSIGN,
	LESS,
	GREATER,
	NOT,

	COLON, // :
	COMMA, // ,
	SEMICOLON, // ;
	ATTR, // .

	SINGLE_LINE_COMMENT,
	MULTI_LINE_COMMENT,

	STRING_LITERAL,
	UNCLOSED_STRING_LITERAL,

	WHITESPACE,
	UNIDENTIFIED,
	END_OF_FILE
} token_type_t;

typedef struct token_t {
	token_type_t type;
	size_t line_no;
	char* value;
} token_t;

typedef struct tokenizer_t {
	char* input;
	size_t line_no;
	size_t pos;
} tokenizer_t;

char* token_to_str(token_type_t);
vec_t tokenize(const char*);

#endif
