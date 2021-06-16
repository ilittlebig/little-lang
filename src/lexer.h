#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

typedef enum token_type_t {
	// Reserved Keywords
	FN,
	INT,
	FLOAT,
	CHAR,
	VOID,

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
	MINUS,
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

	SINGLELINE_COMMENT,
	MULTILINE_COMMENT,

	STRING_LITERAL,
	UNCLOSED_STRING_LITERAL,

	WHITESPACE,
	UNIDENTIFIED,
	END_OF_FILE
} token_type_t;

typedef struct token_t {
	token_type_t type;
} token_t;

typedef struct tokenizer_t {
	char* input;
	size_t pos;
} tokenizer_t;

#endif
