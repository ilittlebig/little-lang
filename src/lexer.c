#include "lexer.h"

bool starts_with(const char* str1, const char* str2, const int pos) {
	for (int i = 0; i < strlen(str2); i++) {
		if (str1[pos + i] != str2[i]) {
			return false;
		}
	}
	return true;
}

bool has_at_least(const tokenizer_t* tokenizer, int n) {
	return tokenizer->pos + n < strlen(tokenizer->input);
}

bool is_eof(const tokenizer_t* tokenizer) {
	return !has_at_least(tokenizer, 0);
}

bool is_valid_ident_start(const tokenizer_t* tokenizer) {
	char c = tokenizer->input[tokenizer->pos];
	if (isalpha(c) || c == '_') {
		return true;
	}
	return false;
}

bool is_valid_ident(const tokenizer_t* tokenizer) {
	char c = tokenizer->input[tokenizer->pos];
	if (isalpha(c) || isdigit(c) || c == '_') {
		return true;
	}
	return false;
}

token_type_t str_to_token(char* word) {
	token_type_t token_type;
	if (strcmp(word, "fn") == 0) {
		token_type = FN;
	} else if (strcmp(word, "int") == 0) {
		token_type = INT;
	} else if (strcmp(word, "float") == 0) {
		token_type = FLOAT;
	} else if (strcmp(word, "string") == 0) {
		token_type = STRING;
	} else if (strcmp(word, "void") == 0) {
		token_type = VOID;
	} else if (strcmp(word, "return") == 0) {
		token_type = RETURN;
	} else if (strcmp(word, "defvar") == 0) {
		token_type = DEFVAR;
	} else if (strcmp(word, "funcall") == 0) {
		token_type = FUNCALL;
	} else {
		token_type = UNIDENTIFIED;
	}
	return token_type;
}

char* token_to_str(token_type_t token_type) {
	switch (token_type) {
		case FN: return "FN";
		case RETURN: return "RETURN";
		case INT: return "INT";
		case FLOAT: return "FLOAT";
		case STRING: return "STRING";
		case VOID: return "VOID";
		case INT_NUMBER: return "INT_NUMBER";
		case IDENTIFIER: return "IDENTIFIER";
		case GREATER_OR_EQUAL: return "GREATER_OR_EQUAL";
		case EQUAL: return "EQUAL";
		case LESS_OR_EQUAL: return "LESS_OR_EQUAL";
		case NOT_EQUAL: return "NOT_EQUAL";
		case LEFT_PAREN: return "LEFT_PAREN";
		case RIGHT_PAREN: return "RIGHT_PAREN";
		case LEFT_BRACKET: return "LEFT_BRACKET";
		case RIGHT_BRACKET: return "RIGHT_BRACKET";
		case LEFT_CURLY: return "LEFT_CURLY";
		case RIGHT_CURLY: return "RIGHT_CURLY";
		case ADD: return "ADD";
		case MINUS: return "MINUS";
		case MUL: return "MUL";
		case DIV: return "DIV";
		case MOD: return "MOD";
		case POW: return "POW";
		case ASSIGN: return "ASSIGN";
		case LESS: return "LESS";
		case GREATER: return "GREATER";
		case NOT: return "NOT";
		case COLON: return "COLON";
		case COMMA: return "COMMA";
		case SEMICOLON: return "SEMICOLON";
		case ATTR: return "ATTR";
		case SINGLE_LINE_COMMENT: return "SINGLE_LINE_COMMENT";
		case STRING_LITERAL: return "STRING_LITERAL";
		case UNCLOSED_STRING_LITERAL: return "UNCLOSED_STRING_LITERAL";
		case WHITESPACE: return "WHITESPACE";
		case END_OF_FILE: return "END_OF_FILE";
		default: return "UNIDENTIFIED";
	}
}

token_t* read_digit(tokenizer_t* tokenizer) {
	token_t* token = malloc(sizeof(token_t));
	char* input = tokenizer->input;

	char* digit = calloc(0, sizeof(char));
	int size = 0;

	while (true) {
		const char c = input[tokenizer->pos];
		if (c == '.') {
			size++;
			digit = realloc(digit, size);
			digit[size-1] = input[tokenizer->pos++];
		} else if (isdigit(c)) {
			size++;
			digit = realloc(digit, size);
			digit[size-1] = input[tokenizer->pos++];
		} else {
			break;
		}
	}

	token->type = INT_NUMBER;
	token->value = digit;
	return token;
}

token_t* read_string(tokenizer_t* tokenizer) {
	token_t* token = malloc(sizeof(token_t));

	char* input = tokenizer->input;
	const char start_char = input[tokenizer->pos++];

	char* string = calloc(1, sizeof(char));
	int size = 0;

	bool is_closed = false;

	while (true) {
		const char c = input[tokenizer->pos];
		if (c != start_char && has_at_least(tokenizer, 0)) {
			size++;
			string = realloc(string, size);
			string[size-1] = input[tokenizer->pos++];
		} else if (c == start_char) {
			is_closed = true;
			tokenizer->pos++;
			break;
		} else {
			break;
		}
	}
	string[size] = '\0';

	if (is_closed) {
		token->type = STRING_LITERAL;
	} else {
		token->type = UNCLOSED_STRING_LITERAL;
	}

	token->value = string;
	return token;
}

token_t* read_single_line_comment(tokenizer_t* tokenizer) {
	token_t* token = malloc(sizeof(token_t));
	token->type = SINGLE_LINE_COMMENT;

	char* input = tokenizer->input;
	const char start_char = input[tokenizer->pos];

	char* string = calloc(0, sizeof(char));
	int size = 0;

	while (true) {
		const char c = input[tokenizer->pos];
		if (c != '\n' && has_at_least(tokenizer, 0)) {
			string = realloc(string, ++size);
			string[size-1] = input[tokenizer->pos++];
		} else {
			break;
		}
	}

	free(string);
	return token;
}

token_t* read_multi_line_comment(tokenizer_t* tokenizer) {
	token_t* token = malloc(sizeof(token_t));
	token->type = MULTI_LINE_COMMENT;

	char* input = tokenizer->input;
	const char start_char = input[tokenizer->pos];

	char* string = calloc(0, sizeof(char));
	int size = 0;

	while (true) {
		if (!starts_with(input, "*/", tokenizer->pos) && has_at_least(tokenizer, 0)) {
			string = realloc(string, ++size);
			string[size-1] = input[tokenizer->pos++];
		} else {
			string = realloc(string, ++size);
			string[size-1] = input[tokenizer->pos++];
			string = realloc(string, ++size);
			string[size-1] = input[tokenizer->pos++];
			break;
		}
	}

	free(string);
	return token;
}

token_t* read_other_tokens(tokenizer_t* tokenizer) {
	token_t* token = malloc(sizeof(token_t));
	char* input = tokenizer->input;
	const char c = input[tokenizer->pos];

	switch(c) {
		case '!':
			if (starts_with(input, "!=", tokenizer->pos)) {
				tokenizer->pos += 2;
				token->type = NOT_EQUAL;
			} else {
				tokenizer->pos++;
				token->type = NOT;
			}
			break;
		case '>':
			if (starts_with(input, ">=", tokenizer->pos)) {
				tokenizer->pos += 2;
				token->type = GREATER_OR_EQUAL;
			} else {
				tokenizer->pos++;
				token->type = GREATER;
			}
			break;
		case '=':
			if (starts_with(input, "==", tokenizer->pos)) {
				tokenizer->pos += 2;
				token->type = EQUAL;
			} else {
				tokenizer->pos++;
				token->type = ASSIGN;
			}
			break;
		case '<':
			if (starts_with(input, "<=", tokenizer->pos)) {
				tokenizer->pos += 2;
				token->type = LESS_OR_EQUAL;
			} else {
				tokenizer->pos++;
				token->type = LESS;
			}
			break;
		case '/':
			if (starts_with(input, "//", tokenizer->pos)) {
				free(token);
				return read_single_line_comment(tokenizer);
			} else if (starts_with(input, "/*", tokenizer->pos)) {
				free(token);
				return read_multi_line_comment(tokenizer);
			} else {
				tokenizer->pos++;
				token->type = DIV;
			}
			break;
		default:
			token->type = UNIDENTIFIED;
			break;
	}

	if (is_valid_ident_start(tokenizer)) {
		char* word = malloc(sizeof(char));
		int size = 0;

		while (is_valid_ident(tokenizer)) {
			size++;
			word = realloc(word, size + 1);
			word[size-1] = input[tokenizer->pos++];
		}
		word[size] = '\0';

		token_type_t token_type = str_to_token(word);
		if (token_type != UNIDENTIFIED) {
			token->type = token_type;
			free(word);
		} else {
			token->type = IDENTIFIER;
			token->value = word;
		}
	}

	return token;
}

token_t* next_token(tokenizer_t* tokenizer) {
	token_t* token = malloc(sizeof(token_t));
	const char c = tokenizer->input[tokenizer->pos];

	switch(c) {
		case '\'':
			free(token);
			return read_string(tokenizer);
		case '\"':
			free(token);
			return read_string(tokenizer);

		case '+':
			token->type = ADD;
			break;
		case '-':
			token->type = MINUS;
			break;
		case '*':
			token->type = MUL;
			break;
		case '%':
			token->type = MOD;
			break;
		case '^':
			token->type = POW;
			break;

		case '(':
			token->type = LEFT_PAREN;
			break;
		case ')':
			token->type = RIGHT_PAREN;
			break;
		case '[':
			token->type = LEFT_BRACKET;
			break;
		case ']':
			token->type = RIGHT_BRACKET;
			break;
		case '{':
			token->type = LEFT_CURLY;
			break;
		case '}':
			token->type = RIGHT_CURLY;
			break;
		case ':':
			token->type = COLON;
			break;
		case ',':
			token->type = COMMA;
			break;
		case '.':
			token->type = ATTR;
			break;
		default:
			token->type = UNIDENTIFIED;
			break;
	}

	if (isdigit(c)) {
		free(token);
		return read_digit(tokenizer);
	} else if (isspace(c) != 0) {
		token->type = WHITESPACE;
	}

	// Check if there are any tokens with two or more characters
	if (token->type == UNIDENTIFIED) {
		free(token);
		return read_other_tokens(tokenizer);
	} else {
		tokenizer->pos++;
	}

	return token;
}

vec_t tokenize(const char* input) {
	tokenizer_t* tokenizer = malloc(sizeof(struct tokenizer_t));
	tokenizer->input = strdup(input);
	tokenizer->pos = 0;

	vec_t tokens;
	vec_init(&tokens, 4);

	while (!is_eof(tokenizer)) {
		token_t* token = next_token(tokenizer);
		if (token->type != WHITESPACE) {
			vec_push_back(&tokens, token);
		} else {
			free(token);
		}
	}

	token_t* token = malloc(sizeof(token_t));
	token->type = END_OF_FILE;
	vec_push_back(&tokens, token);

	free(tokenizer->input);
	free(tokenizer);

	return tokens;
}
